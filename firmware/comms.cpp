// ============================================================================
//  HATCH — comms.cpp
//  LoRaWAN AS923 (Singapore) using the sandeepmistry/arduino-LoRa library.
//  In production we'd wrap this in a LoRaWAN MAC (e.g., LMIC / LoRaMac-node)
//  to join a TheThingsNetwork-compatible private gateway, but the on-air
//  packet shape and timing characteristics are identical at the application
//  layer.
// ============================================================================

#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include "esp_log.h"
#include "config.h"
#include "comms.h"

static const char* TAG = "hatch.comms";
static bool        radio_ok = false;
static uint32_t    epoch_offset = 0;       // wall-clock at last sync minus millis()/1000

bool comms_init(void) {
    SPI.begin();
    LoRa.setPins(HATCH_PIN_LORA_CS, HATCH_PIN_LORA_RST, HATCH_PIN_LORA_DIO0);
    if (!LoRa.begin(HATCH_LORA_FREQ_HZ)) {
        ESP_LOGE(TAG, "LoRa.begin() failed");
        radio_ok = false;
        return false;
    }
    LoRa.setSpreadingFactor(HATCH_LORA_SF);
    LoRa.setSignalBandwidth(125E3);
    LoRa.setCodingRate4(5);
    LoRa.setTxPower(HATCH_LORA_TX_POWER_DBM);
    LoRa.enableCrc();
    radio_ok = true;
    return true;
}

bool comms_join(void) {
    // OTAA join would happen here in a full LoRaWAN MAC implementation.
    // For the AS923 private-gateway test setup, this is a no-op + sync handshake.
    return radio_ok;
}

// Tiny CRC-8 (polynomial 0x07) for packet integrity at the application layer.
static uint8_t crc8(const uint8_t* data, size_t len) {
    uint8_t crc = 0;
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (uint8_t b = 0; b < 8; ++b) {
            crc = (crc & 0x80) ? (crc << 1) ^ 0x07 : (crc << 1);
        }
    }
    return crc;
}

static bool send_typed(uint8_t type_id, const void* payload, size_t len) {
    if (!radio_ok) return false;
    LoRa.beginPacket();
    LoRa.write(type_id);
    LoRa.write((const uint8_t*)payload, len);
    LoRa.write(crc8((const uint8_t*)payload, len));
    int rc = LoRa.endPacket();
    if (rc != 1) ESP_LOGW(TAG, "LoRa endPacket returned %d", rc);
    return rc == 1;
}

bool comms_send_alert(const AlertPacket& p) {
    ESP_LOGI(TAG, "TX alert: node=%u stagn=%uh fav=%u%% lbl=%u conf=%u%%",
             p.node_id, p.stagnation_h, p.fav_score_pct, p.acoustic_label, p.acoustic_conf);
    return send_typed(0x01, &p, sizeof(p));
}

bool comms_send_summary(const SummaryPacket& s) {
    ESP_LOGI(TAG, "TX summary: node=%u boot=%u batt=%umV",
             s.node_id, s.boot_count, s.battery_mv);
    return send_typed(0x02, &s, sizeof(s));
}

uint32_t comms_get_epoch(void) {
    if (epoch_offset == 0) return millis() / 1000;
    return epoch_offset + millis() / 1000;
}
