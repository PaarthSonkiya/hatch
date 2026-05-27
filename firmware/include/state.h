#pragma once
// ============================================================================
//  HATCH — state.h
//  Shared data structs across firmware modules.
// ============================================================================

#include <stdint.h>
#include "config.h"

// ---- Environmental sample ----
struct EnvReading {
    bool     water_present;
    uint32_t water_capacity;       // raw capacitive count
    float    temperature_c;
    float    humidity_pct;
    float    pressure_hpa;
    uint32_t stagnation_seconds;   // continuous-water-present timer
    float    favorability_score;   // 0.0–1.0, computed by env. gate
};

// ---- Acoustic inference result ----
struct AcousticResult {
    acoustic_label_t label;
    float            confidence;   // softmax probability of the argmax class
};

// ---- LoRa packets (compact binary on the wire) ----
struct __attribute__((packed)) AlertPacket {
    uint16_t node_id;
    uint8_t  fw_version_minor;
    uint32_t epoch;
    int16_t  temperature_dC;       // tenths of °C
    uint8_t  humidity_pct;
    uint16_t stagnation_h;
    uint8_t  fav_score_pct;
    uint8_t  acoustic_label;
    uint8_t  acoustic_conf;        // percent
    uint16_t battery_mv;
};

struct __attribute__((packed)) SummaryPacket {
    uint16_t node_id;
    uint32_t epoch;
    uint32_t uptime_hours;
    uint16_t battery_mv;
    uint8_t  internal_humidity_pct;
    uint32_t boot_count;
    uint32_t last_alert_epoch;
};
