// ============================================================================
//  HATCH — power.cpp
//  Two switchable rails: env-sensor rail (BME280 + capacitive probe bias)
//  and microphone rail (INMP441). Both default OFF; the main loop enables
//  them only during their active phase to honour the power budget in
//  /hardware/power-budget.md.
// ============================================================================

#include <Arduino.h>
#include "config.h"
#include "power.h"

static constexpr float    ADC_VREF_MV     = 3300.0f;
static constexpr uint16_t ADC_RES_COUNTS  = 4095;
// Battery divider: 100k / 100k (1:2). Vbatt = Vadc × 2.
static constexpr float    BATT_DIVIDER    = 2.0f;

void power_begin_env_phase(void) {
    pinMode(HATCH_PIN_SENSOR_PWR_EN, OUTPUT);
    digitalWrite(HATCH_PIN_SENSOR_PWR_EN, HIGH);
    delay(5);  // sensors settle
}

void power_end_env_phase(void) {
    digitalWrite(HATCH_PIN_SENSOR_PWR_EN, LOW);
}

void power_begin_acoustic_phase(void) {
    pinMode(HATCH_PIN_MIC_PWR_EN, OUTPUT);
    digitalWrite(HATCH_PIN_MIC_PWR_EN, HIGH);
    delay(20);  // INMP441 wakeup tWAKE ≈ 23ms
}

void power_end_acoustic_phase(void) {
    digitalWrite(HATCH_PIN_MIC_PWR_EN, LOW);
}

uint16_t power_read_battery_mv(void) {
    // ESP32-S3 ADC is noisy; average N reads
    constexpr uint8_t N = 16;
    uint32_t acc = 0;
    for (uint8_t i = 0; i < N; ++i) {
        acc += analogRead(HATCH_PIN_BATT_ADC);
        delayMicroseconds(100);
    }
    float counts = (float)acc / (float)N;
    float vadc_mv = (counts / (float)ADC_RES_COUNTS) * ADC_VREF_MV;
    float vbatt_mv = vadc_mv * BATT_DIVIDER;
    return (uint16_t)vbatt_mv;
}
