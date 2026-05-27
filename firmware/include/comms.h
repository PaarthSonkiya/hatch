#pragma once
// ============================================================================
//  HATCH — comms.h
//  LoRaWAN AS923 transmission, time synchronization.
// ============================================================================

#include <stdbool.h>
#include <stdint.h>
#include "state.h"

#ifdef __cplusplus
extern "C" {
#endif

bool      comms_init(void);
bool      comms_join(void);
bool      comms_send_alert(const AlertPacket& p);
bool      comms_send_summary(const SummaryPacket& s);
uint32_t  comms_get_epoch(void);    // Unix epoch from RTC, set at last LoRa downlink

#ifdef __cplusplus
}
#endif
