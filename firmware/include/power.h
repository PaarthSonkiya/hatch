#pragma once
// ============================================================================
//  HATCH — power.h
//  Rail enable/disable for env-sensor and microphone subsystems;
//  battery voltage measurement via ADC.
// ============================================================================

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void     power_begin_env_phase(void);
void     power_end_env_phase(void);
void     power_begin_acoustic_phase(void);
void     power_end_acoustic_phase(void);
uint16_t power_read_battery_mv(void);

#ifdef __cplusplus
}
#endif
