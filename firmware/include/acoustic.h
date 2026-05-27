#pragma once
// ============================================================================
//  HATCH — acoustic.h
//  INMP441 I2S capture, mel-spectrogram preprocessing, TinyML inference.
// ============================================================================

#include <stdbool.h>
#include <stdint.h>
#include "state.h"

#define ACOUSTIC_OK            0
#define ACOUSTIC_ERR_PSRAM    -1
#define ACOUSTIC_ERR_I2S      -2
#define ACOUSTIC_ERR_FEATURE  -3

// Mel-spectrogram feature dimensions, matched to the trained model.
// 40 mel bins × 64 time frames (post-pooling) = 2,560 INT8 values
#define MEL_BINS              40
#define MEL_TIME_FRAMES       64

typedef struct {
    int8_t  mel_int8[MEL_BINS * MEL_TIME_FRAMES];
} acoustic_features_t;

#ifdef __cplusplus
extern "C" {
#endif

// Capture `n_samples` from INMP441 at HATCH_AUDIO_SAMPLE_RATE_HZ.
// Returns pointer to PSRAM-resident buffer, or NULL on failure.
// Caller does NOT free; buffer is owned by the acoustic module and reused.
int16_t* acoustic_capture(uint32_t n_samples);

// Pre-emphasis -> framing -> mel-spectrogram -> PCEN normalization.
// Writes into out->mel_int8. Returns ACOUSTIC_OK on success.
int acoustic_preprocess(const int16_t* pcm, acoustic_features_t* out);

// Run quantized 1D-CNN. Returns AcousticResult with label + softmax confidence.
// Uses the Edge Impulse Inferencing SDK with the model exported from
// /ml/notebooks/03_model_training.ipynb.
AcousticResult acoustic_infer(const acoustic_features_t* feat);

#ifdef __cplusplus
}
#endif
