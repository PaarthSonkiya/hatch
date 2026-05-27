// ============================================================================
//  HATCH — sensors.cpp
//  BME280 over I2C + capacitive water-presence probe.
// ============================================================================

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Preferences.h>
#include "esp_log.h"
#include "config.h"
#include "sensors.h"

static const char* TAG = "hatch.sensors";

static Adafruit_BME280 bme;
static bool            bme_ok = false;

// Adaptive capacitive baseline persisted across deep-sleep cycles.
// Stored in NVS, updated slowly during dry periods.
static Preferences     prefs;
static uint32_t        cap_baseline = 0;
static uint32_t        cap_last_raw = 0;

// Number of capacitive samples averaged per read
static constexpr uint8_t CAP_SAMPLES = 8;

// Threshold multiplier: probe is "wet" when reading exceeds baseline by 30%
static constexpr float   CAP_WET_RATIO = 1.30f;

// Baseline learning rate (slow IIR): only updated when dry
static constexpr float   CAP_BASELINE_ALPHA = 0.05f;

bool sensors_init(void) {
    Wire.begin(HATCH_PIN_I2C_SDA, HATCH_PIN_I2C_SCL);
    Wire.setClock(100000);

    // BME280 on the default 0x76 (some breakout boards use 0x77)
    bme_ok = bme.begin(0x76, &Wire);
    if (!bme_ok) {
        bme_ok = bme.begin(0x77, &Wire);
    }
    if (!bme_ok) {
        ESP_LOGE(TAG, "BME280 not found on I2C bus");
    }

    // Configure BME280 for "weather" use case: low power, 1 sample/s when polled
    if (bme_ok) {
        bme.setSampling(Adafruit_BME280::MODE_FORCED,
                        Adafruit_BME280::SAMPLING_X1,
                        Adafruit_BME280::SAMPLING_X1,
                        Adafruit_BME280::SAMPLING_X1,
                        Adafruit_BME280::FILTER_OFF);
    }

    // Load persisted capacitive baseline
    prefs.begin("hatch_cap", false);
    cap_baseline = prefs.getUInt("baseline", 0);
    prefs.end();

    return bme_ok;
}

bool sensors_read_bme280(float* temperature_c, float* humidity_pct, float* pressure_hpa) {
    if (!bme_ok) return false;
    bme.takeForcedMeasurement();
    if (temperature_c) *temperature_c = bme.readTemperature();
    if (humidity_pct)  *humidity_pct  = bme.readHumidity();
    if (pressure_hpa)  *pressure_hpa  = bme.readPressure() / 100.0f;
    return true;
}

bool sensors_read_water_capacitive(void) {
    // Read N samples, take the median (robust to outliers from radio noise)
    uint32_t samples[CAP_SAMPLES];
    for (uint8_t i = 0; i < CAP_SAMPLES; ++i) {
        // touchRead() returns ESP32's capacitive count; lower = more capacitance
        // We invert so "water present" = high reading for intuition.
        samples[i] = 65535 - touchRead(HATCH_PIN_WATER_PROBE);
        delayMicroseconds(500);
    }

    // Insertion sort (N=8, trivial cost)
    for (uint8_t i = 1; i < CAP_SAMPLES; ++i) {
        uint32_t key = samples[i];
        int8_t j = i - 1;
        while (j >= 0 && samples[j] > key) { samples[j+1] = samples[j]; j--; }
        samples[j+1] = key;
    }
    cap_last_raw = samples[CAP_SAMPLES / 2];  // median

    // Initial baseline learning: if no baseline ever stored, seed with first read
    if (cap_baseline == 0) {
        cap_baseline = cap_last_raw;
        prefs.begin("hatch_cap", false);
        prefs.putUInt("baseline", cap_baseline);
        prefs.end();
        return false;   // first sample, can't know if wet
    }

    bool wet = ((float)cap_last_raw > CAP_baseline_scaled());

    // Update baseline only during clearly-dry periods (slow IIR)
    if (!wet) {
        float updated = (1.0f - CAP_BASELINE_ALPHA) * (float)cap_baseline
                      + CAP_BASELINE_ALPHA * (float)cap_last_raw;
        cap_baseline = (uint32_t)updated;
        // Persist every 50th update to limit flash wear
        static uint32_t persist_counter = 0;
        if ((++persist_counter % 50) == 0) {
            prefs.begin("hatch_cap", false);
            prefs.putUInt("baseline", cap_baseline);
            prefs.end();
        }
    }

    return wet;
}

uint32_t sensors_get_last_capacitance_raw(void) {
    return cap_last_raw;
}

int sensors_selftest(void) {
    if (!bme_ok) return -1;

    float t, h, p;
    if (!sensors_read_bme280(&t, &h, &p)) return -2;
    if (t < -10.0f || t > 60.0f) return -3;    // sanity range
    if (h < 0.0f || h > 100.0f) return -4;
    if (p < 800.0f || p > 1100.0f) return -5;

    // Probe should produce a reading in a believable range
    sensors_read_water_capacitive();
    if (cap_last_raw == 0 || cap_last_raw > 60000) return -6;

    return 0;
}

// Helper — scaled baseline for wet/dry decision
static float CAP_baseline_scaled(void) {
    return (float)cap_baseline * CAP_WET_RATIO;
}
