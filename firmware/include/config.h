#pragma once
// ============================================================================
//  HATCH — config.h
//  Compile-time configuration. Values pulled from platformio.ini via -D flags,
//  with defaults here for completeness and IDE friendliness.
// ============================================================================

#ifndef HATCH_FW_VERSION
#define HATCH_FW_VERSION              "0.1.0-pregrant"
#endif

#define HATCH_FW_VERSION_MAJOR        0
#define HATCH_FW_VERSION_MINOR        1
#define HATCH_FW_VERSION_PATCH        0

// ---- Node identity ----
// Each deployed node gets a unique 16-bit ID; the gateway maps ID → site location.
// 0xFFFF is the "uncommissioned" sentinel; the captive-portal commissioning flow
// writes the real ID to NVS on first boot.
#ifndef HATCH_NODE_ID
#define HATCH_NODE_ID                 0xFFFF
#endif

// ---- Sampling cadence ----
#ifndef HATCH_ENV_SAMPLE_PERIOD_S
#define HATCH_ENV_SAMPLE_PERIOD_S     300U          // 5 minutes
#endif

// ---- Acoustic stage ----
#ifndef HATCH_AUDIO_SAMPLE_RATE_HZ
#define HATCH_AUDIO_SAMPLE_RATE_HZ    16000U
#endif

#ifndef HATCH_AUDIO_BUFFER_S
#define HATCH_AUDIO_BUFFER_S          4U
#endif

#ifndef HATCH_ACOUSTIC_CONF_THRESHOLD
#define HATCH_ACOUSTIC_CONF_THRESHOLD 0.78f
#endif

// Temporal voting window (see whitepaper §4.1)
#ifndef HATCH_TEMPORAL_VOTE_WINDOW
#define HATCH_TEMPORAL_VOTE_WINDOW    5U
#endif

#ifndef HATCH_TEMPORAL_VOTE_MIN_AGREE
#define HATCH_TEMPORAL_VOTE_MIN_AGREE 3U
#endif

// ---- Pinout (XIAO ESP32-S3 Sense) ----
#define HATCH_PIN_I2C_SDA             5
#define HATCH_PIN_I2C_SCL             6
#define HATCH_PIN_I2S_BCLK            13
#define HATCH_PIN_I2S_LRCK            14
#define HATCH_PIN_I2S_DIN             15
#define HATCH_PIN_WATER_PROBE         2   // touch-capable GPIO for capacitive sense
#define HATCH_PIN_LORA_CS             7
#define HATCH_PIN_LORA_RST            8
#define HATCH_PIN_LORA_DIO0           9
#define HATCH_PIN_BATT_ADC            3
#define HATCH_PIN_SENSOR_PWR_EN       1   // enables the env-sensor rail
#define HATCH_PIN_MIC_PWR_EN          4   // enables the microphone rail

// ---- LoRaWAN ----
#define HATCH_LORA_FREQ_HZ            923200000UL   // AS923-1, Singapore
#define HATCH_LORA_SF                 10
#define HATCH_LORA_TX_POWER_DBM       14

// ---- Acoustic label enum (must match Edge Impulse export) ----
typedef enum {
    ACOUSTIC_LABEL_NOISE           = 0,
    ACOUSTIC_LABEL_OTHER_INSECT    = 1,
    ACOUSTIC_LABEL_AE_AEGYPTI      = 2,
    ACOUSTIC_LABEL_AE_ALBOPICTUS   = 3,
    ACOUSTIC_LABEL_NONE            = 255
} acoustic_label_t;
