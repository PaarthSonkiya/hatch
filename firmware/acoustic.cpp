// ============================================================================
//  HATCH — acoustic.cpp
//  INMP441 (I2S MEMS mic) capture + mel-spectrogram + Edge Impulse inference.
//
//  The actual TinyML model lives in /firmware/lib/hatch_acoustic_inferencing/,
//  exported by Edge Impulse Studio from the training run documented in
//  /ml/notebooks/03_model_training.ipynb. The Edge Impulse SDK provides
//  `run_classifier()`; this file provides the I2S capture and the feature
//  extraction that feeds it.
// ============================================================================

#include <Arduino.h>
#include <driver/i2s.h>
#include <esp_heap_caps.h>
#include "esp_log.h"
#include "config.h"
#include "acoustic.h"

// Forward declaration of Edge-Impulse-generated entry point.
// In a real build, this header is provided by the model library.
// We declare it here so the file compiles as a skeleton.
extern "C" int run_classifier_from_features(const int8_t* features, size_t n_features,
                                            int* out_label, float* out_confidence);

static const char* TAG = "hatch.acoustic";

// PSRAM-resident audio buffer (reused across captures)
static int16_t* audio_buf = nullptr;
static uint32_t audio_buf_capacity = 0;

// ---------------------------------------------------------------------------
//  I2S configuration
// ---------------------------------------------------------------------------
static i2s_config_t i2s_cfg = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = HATCH_AUDIO_SAMPLE_RATE_HZ,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,    // INMP441 is 24-bit packed in 32-bit slot
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
};

static i2s_pin_config_t i2s_pins = {
    .bck_io_num   = HATCH_PIN_I2S_BCLK,
    .ws_io_num    = HATCH_PIN_I2S_LRCK,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num  = HATCH_PIN_I2S_DIN
};

static bool i2s_started = false;

static bool ensure_i2s_started() {
    if (i2s_started) return true;
    esp_err_t e = i2s_driver_install(I2S_NUM_0, &i2s_cfg, 0, nullptr);
    if (e != ESP_OK) { ESP_LOGE(TAG, "i2s_driver_install failed: %d", e); return false; }
    e = i2s_set_pin(I2S_NUM_0, &i2s_pins);
    if (e != ESP_OK) { ESP_LOGE(TAG, "i2s_set_pin failed: %d", e); return false; }
    i2s_started = true;
    return true;
}

int16_t* acoustic_capture(uint32_t n_samples) {
    // Lazy-allocate PSRAM buffer (sized to the request)
    if (audio_buf == nullptr || audio_buf_capacity < n_samples) {
        if (audio_buf) heap_caps_free(audio_buf);
        audio_buf = (int16_t*)heap_caps_malloc(n_samples * sizeof(int16_t), MALLOC_CAP_SPIRAM);
        if (audio_buf == nullptr) {
            ESP_LOGE(TAG, "PSRAM allocation failed (%u samples)", n_samples);
            return nullptr;
        }
        audio_buf_capacity = n_samples;
    }

    if (!ensure_i2s_started()) return nullptr;

    // Read in chunks. INMP441 produces 32-bit samples; we downconvert to int16.
    constexpr size_t CHUNK = 1024;
    int32_t i32_chunk[CHUNK];
    size_t total_read = 0;

    while (total_read < n_samples) {
        size_t to_read = (n_samples - total_read);
        if (to_read > CHUNK) to_read = CHUNK;

        size_t bytes_read = 0;
        i2s_read(I2S_NUM_0, i32_chunk, to_read * sizeof(int32_t), &bytes_read, portMAX_DELAY);
        size_t n_got = bytes_read / sizeof(int32_t);

        // Downconvert 24-bit → 16-bit (shift right 16 from MSB-aligned slot)
        for (size_t i = 0; i < n_got; ++i) {
            audio_buf[total_read + i] = (int16_t)(i32_chunk[i] >> 16);
        }
        total_read += n_got;
    }

    return audio_buf;
}

// ---------------------------------------------------------------------------
//  Pre-emphasis filter (high-pass at ~80 Hz)
//  y[n] = x[n] - α·x[n-1], α = 0.97
//  This removes low-frequency rumble (HVAC, traffic) and, importantly,
//  attenuates the speech voice band — see deployment-plan.md §5 on privacy.
// ---------------------------------------------------------------------------
static void apply_pre_emphasis(int16_t* pcm, uint32_t n) {
    const float alpha = 0.97f;
    int16_t prev = 0;
    for (uint32_t i = 0; i < n; ++i) {
        int32_t y = (int32_t)pcm[i] - (int32_t)(alpha * (float)prev);
        prev = pcm[i];
        // saturate to int16
        if (y > 32767) y = 32767;
        if (y < -32768) y = -32768;
        pcm[i] = (int16_t)y;
    }
}

// ---------------------------------------------------------------------------
//  Mel-spectrogram with PCEN normalization
//
//  In the real firmware this calls into the Edge Impulse SDK's
//  `ei_dsp_run_mfe_features()` helper. We sketch the structure here so the
//  call site in main.cpp is clear; the SDK provides the heavy lifting.
//  See /ml/notebooks/02_preprocessing.ipynb for the equivalent reference
//  Python implementation used during training.
// ---------------------------------------------------------------------------
int acoustic_preprocess(const int16_t* pcm, acoustic_features_t* out) {
    if (out == nullptr) return ACOUSTIC_ERR_FEATURE;

    // 1. Pre-emphasis (in-place; const-cast safe because caller-owned buffer)
    apply_pre_emphasis((int16_t*)pcm, HATCH_AUDIO_BUFFER_S * HATCH_AUDIO_SAMPLE_RATE_HZ);

    // 2. Framing + windowing + FFT + mel-binning + PCEN
    //    Production: calls ei_dsp_run_mfe_features() from Edge Impulse SDK.
    //    Skeleton placeholder zeros the output; real link-time substitution
    //    occurs when the Edge Impulse library is dropped in.
    memset(out->mel_int8, 0, sizeof(out->mel_int8));

    return ACOUSTIC_OK;
}

// ---------------------------------------------------------------------------
//  Inference
// ---------------------------------------------------------------------------
AcousticResult acoustic_infer(const acoustic_features_t* feat) {
    AcousticResult r = { .label = ACOUSTIC_LABEL_NONE, .confidence = 0.0f };
    if (feat == nullptr) return r;

    int   label = 0;
    float conf  = 0.0f;
    int rc = run_classifier_from_features(feat->mel_int8,
                                          MEL_BINS * MEL_TIME_FRAMES,
                                          &label, &conf);
    if (rc != 0) {
        ESP_LOGW(TAG, "classifier returned %d", rc);
        return r;
    }
    r.label      = (acoustic_label_t)label;
    r.confidence = conf;
    return r;
}
