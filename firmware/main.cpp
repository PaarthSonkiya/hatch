// ============================================================================
//  HATCH — main.cpp
// ============================================================================
//  Edge node firmware for multi-modal Aedes breeding-site detection.
//
//  State machine:
//
//    POR/RESET ──► BOOT ──► ENV_SAMPLE ──► ENV_GATE
//                                              │
//                              ┌───────────────┴───────────────┐
//                              ▼                               ▼
//                          GATE_CLOSED                     GATE_OPEN
//                              │                               │
//                              │                          ACOUSTIC_CAPTURE
//                              │                               │
//                              │                          ACOUSTIC_INFER
//                              │                               │
//                              │                          TEMPORAL_VOTE
//                              │                               │
//                              │                       ┌───────┴────┐
//                              │                       ▼            ▼
//                              │                   NO_ALERT      ALERT_TX
//                              │                       │            │
//                              └───────────────────────┴────────────┘
//                                                  │
//                                                  ▼
//                                              DEEP_SLEEP ──► (wake on RTC)
//
//  Power profile:
//    Deep sleep:                ~50 μA
//    Env. sample (1.5s):        ~18 mA
//    Acoustic capture (4s):     ~25 mA
//    Acoustic inference (~200ms): ~110 mA
//    LoRa TX (~500ms):          ~130 mA
//
//  See /docs/whitepaper.md §3.2 for full power budget derivation.
// ============================================================================

#include <Arduino.h>
#include "esp_sleep.h"
#include "esp_log.h"
#include "config.h"
#include "sensors.h"
#include "acoustic.h"
#include "comms.h"
#include "power.h"
#include "state.h"

static const char* TAG = "hatch.main";

// ---------------------------------------------------------------------------
//  RTC-persistent state (survives deep sleep)
// ---------------------------------------------------------------------------
RTC_DATA_ATTR uint32_t  boot_count          = 0;
RTC_DATA_ATTR uint32_t  stagnation_seconds  = 0;   // continuous water-present time
RTC_DATA_ATTR uint32_t  last_alert_epoch    = 0;
RTC_DATA_ATTR uint32_t  last_summary_epoch  = 0;
RTC_DATA_ATTR uint8_t   recent_acoustic_votes[HATCH_TEMPORAL_VOTE_WINDOW] = {0};
RTC_DATA_ATTR uint8_t   vote_index          = 0;

// ---------------------------------------------------------------------------
//  Forward declarations
// ---------------------------------------------------------------------------
static void on_boot();
static void run_env_sample(EnvReading& out);
static bool evaluate_env_gate(const EnvReading& env);
static AcousticResult run_acoustic_stage();
static bool temporal_vote(uint8_t label_id);
static void compose_and_send_alert(const EnvReading& env, const AcousticResult& acc);
static void compose_and_send_summary(const EnvReading& env);
static void enter_deep_sleep(uint32_t seconds);

// ===========================================================================
//  setup() — runs once per wake. There is no loop(); we deep-sleep instead.
// ===========================================================================
void setup() {
    Serial.begin(115200);
    delay(50);  // let UART settle before first log line

    boot_count++;
    ESP_LOGI(TAG, "==== HATCH " HATCH_FW_VERSION " — wake #%u ====", boot_count);

    on_boot();

    // ---- 1. Environmental sample (always runs) ----
    EnvReading env;
    run_env_sample(env);

    // Update stagnation timer based on water presence
    if (env.water_present) {
        stagnation_seconds += HATCH_ENV_SAMPLE_PERIOD_S;
    } else {
        stagnation_seconds = 0;
    }
    env.stagnation_seconds = stagnation_seconds;

    ESP_LOGI(TAG, "env: water=%d, T=%.1fC, RH=%.0f%%, stagnation=%us",
             env.water_present, env.temperature_c, env.humidity_pct, stagnation_seconds);

    // ---- 2. Environmental gate ----
    bool gate_open = evaluate_env_gate(env);
    ESP_LOGI(TAG, "env_gate: %s (score=%.2f)", gate_open ? "OPEN" : "closed", env.favorability_score);

    AcousticResult acc = { .label = ACOUSTIC_LABEL_NONE, .confidence = 0.0f };

    if (gate_open) {
        // ---- 3. Acoustic capture + inference ----
        acc = run_acoustic_stage();
        ESP_LOGI(TAG, "acoustic: label=%d, conf=%.2f", acc.label, acc.confidence);

        // ---- 4. Temporal voting ----
        bool aedes_detected = (acc.label == ACOUSTIC_LABEL_AE_AEGYPTI ||
                               acc.label == ACOUSTIC_LABEL_AE_ALBOPICTUS) &&
                              acc.confidence >= HATCH_ACOUSTIC_CONF_THRESHOLD;

        if (temporal_vote(aedes_detected ? 1 : 0)) {
            // ---- 5. ALERT ----
            ESP_LOGW(TAG, ">>> ALERT: confirmed Aedes activity at favorable breeding site");
            compose_and_send_alert(env, acc);
            last_alert_epoch = comms_get_epoch();
        }
    }

    // ---- 6. Daily summary (if due) ----
    uint32_t now = comms_get_epoch();
    if (now - last_summary_epoch >= 24UL * 3600UL) {
        compose_and_send_summary(env);
        last_summary_epoch = now;
    }

    // ---- 7. Back to sleep ----
    enter_deep_sleep(HATCH_ENV_SAMPLE_PERIOD_S);
}

void loop() {
    // Never reached; deep sleep at end of setup() restarts the cycle.
}

// ===========================================================================
//  Helpers
// ===========================================================================

static void on_boot() {
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    switch (cause) {
        case ESP_SLEEP_WAKEUP_TIMER:
            ESP_LOGD(TAG, "wake: RTC timer");
            break;
        case ESP_SLEEP_WAKEUP_EXT0:
            ESP_LOGD(TAG, "wake: external GPIO");
            break;
        default:
            ESP_LOGI(TAG, "wake: cold boot / reset");
            // First boot: initialise comms (LoRa join), do a longer self-test
            comms_init();
            comms_join();
            break;
    }

    // Always re-init sensor bus on wake (BME280, I2C bus)
    sensors_init();
}

static void run_env_sample(EnvReading& out) {
    power_begin_env_phase();  // turn on sensor rail
    sensors_read_bme280(&out.temperature_c, &out.humidity_pct, &out.pressure_hpa);
    out.water_present  = sensors_read_water_capacitive();
    out.water_capacity = sensors_get_last_capacitance_raw();
    power_end_env_phase();
}

static bool evaluate_env_gate(const EnvReading& env) {
    // Hand-tuned, intentionally transparent. See whitepaper §2.3.
    float s = 0.0f;
    if (env.stagnation_seconds > 24 * 3600) s += 0.45f;
    if (env.temperature_c >= 25.0f && env.temperature_c <= 33.0f) s += 0.25f;
    if (env.humidity_pct > 70.0f) s += 0.15f;
    if (env.stagnation_seconds > 12 * 3600) s += 0.15f;  // proxy for "no recent flow"

    const_cast<EnvReading&>(env).favorability_score = s;
    return s >= 0.60f;
}

static AcousticResult run_acoustic_stage() {
    AcousticResult r = { .label = ACOUSTIC_LABEL_NONE, .confidence = 0.0f };

    power_begin_acoustic_phase();

    // 4-second capture at 16 kHz from INMP441 over I2S.
    // Audio buffer lives in PSRAM (128 KB).
    int16_t* buf = acoustic_capture(HATCH_AUDIO_BUFFER_S * HATCH_AUDIO_SAMPLE_RATE_HZ);
    if (buf == nullptr) {
        ESP_LOGE(TAG, "acoustic capture failed (out of PSRAM?)");
        power_end_acoustic_phase();
        return r;
    }

    // Pre-emphasis + mel-spectrogram + PCEN → tensor input
    acoustic_features_t feat;
    if (acoustic_preprocess(buf, &feat) != ACOUSTIC_OK) {
        ESP_LOGE(TAG, "preprocessing failed");
        power_end_acoustic_phase();
        return r;
    }

    // Run quantized 1D-CNN (Edge Impulse SDK).
    // Returns argmax label + softmax confidence.
    r = acoustic_infer(&feat);

    power_end_acoustic_phase();
    return r;
}

static bool temporal_vote(uint8_t aedes_now) {
    recent_acoustic_votes[vote_index] = aedes_now;
    vote_index = (vote_index + 1) % HATCH_TEMPORAL_VOTE_WINDOW;

    uint8_t agree = 0;
    for (uint8_t i = 0; i < HATCH_TEMPORAL_VOTE_WINDOW; ++i) {
        agree += recent_acoustic_votes[i];
    }
    ESP_LOGI(TAG, "temporal vote: %u of %u", agree, HATCH_TEMPORAL_VOTE_WINDOW);
    return agree >= HATCH_TEMPORAL_VOTE_MIN_AGREE;
}

static void compose_and_send_alert(const EnvReading& env, const AcousticResult& acc) {
    AlertPacket p;
    p.node_id          = HATCH_NODE_ID;
    p.fw_version_minor = HATCH_FW_VERSION_MINOR;
    p.epoch            = comms_get_epoch();
    p.temperature_dC   = (int16_t)(env.temperature_c * 10);
    p.humidity_pct     = (uint8_t)env.humidity_pct;
    p.stagnation_h     = (uint16_t)(env.stagnation_seconds / 3600);
    p.fav_score_pct    = (uint8_t)(env.favorability_score * 100);
    p.acoustic_label   = acc.label;
    p.acoustic_conf    = (uint8_t)(acc.confidence * 100);
    p.battery_mv       = power_read_battery_mv();
    comms_send_alert(p);
}

static void compose_and_send_summary(const EnvReading& env) {
    SummaryPacket s;
    s.node_id          = HATCH_NODE_ID;
    s.epoch            = comms_get_epoch();
    s.uptime_hours     = (uint32_t)(boot_count * HATCH_ENV_SAMPLE_PERIOD_S / 3600);
    s.battery_mv       = power_read_battery_mv();
    s.internal_humidity_pct = (uint8_t)env.humidity_pct;
    s.boot_count       = boot_count;
    s.last_alert_epoch = last_alert_epoch;
    comms_send_summary(s);
}

static void enter_deep_sleep(uint32_t seconds) {
    ESP_LOGI(TAG, "deep sleep for %u s\n", seconds);
    Serial.flush();
    esp_sleep_enable_timer_wakeup((uint64_t)seconds * 1000000ULL);
    esp_deep_sleep_start();
}
