#pragma once
// ============================================================================
//  HATCH — sensors.h
//  BME280 (env) + capacitive water-presence probe.
// ============================================================================

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialise I2C bus, BME280, and configure capacitive touch peripheral.
// Called on every wake from deep sleep.
bool sensors_init(void);

// Read BME280. Returns true on success; values written via out-pointers.
// If the sensor is not responding, returns false and leaves outputs untouched.
bool sensors_read_bme280(float* temperature_c, float* humidity_pct, float* pressure_hpa);

// Capacitive water-presence read.
//   Returns true if water has been detected at the probe.
//   Internally: read N=8 samples, average, compare to running baseline stored in NVS.
//   The threshold is adaptive — see /docs/whitepaper.md §2.2 on probe calibration.
bool sensors_read_water_capacitive(void);

// Returns the raw capacitance count from the last water-read, for telemetry.
uint32_t sensors_get_last_capacitance_raw(void);

// Run a self-test: returns 0 on pass, negative error code on failure.
// Called on cold boot. Verifies BME280 responds and capacitive probe is in range.
int sensors_selftest(void);

#ifdef __cplusplus
}
#endif
