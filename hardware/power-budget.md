# Hatch — Power Budget

**Detailed energy calculation justifying solar-powered autonomous operation.**

This document supports the power-budget summary in the whitepaper (§3.2). The numbers here are the engineering-grade calculation that the whitepaper summarises; they are the basis for the choice of 2000 mAh LiPo capacity and 5 W solar panel.

---

## 1. Inputs

| Parameter | Value | Source |
|---|---|---|
| Environmental sample period | 300 s (5 min) | `HATCH_ENV_SAMPLE_PERIOD_S` in `config.h` |
| Acoustic capture window | 4 s | `HATCH_AUDIO_BUFFER_S` |
| Audio sample rate | 16 kHz | `HATCH_AUDIO_SAMPLE_RATE_HZ` |
| Daily acoustic events (typical) | 6 | Estimated from breeding-favourable conditions in HDB drains; varies with site |
| Daily acoustic events (worst case) | 36 | Site with persistent stagnant water + high background activity |
| LoRa TX per day (typical) | 4 | 1 daily summary + ~3 alerts |
| Battery capacity | 2000 mAh @ 3.7V nominal = 7.4 Wh | LiPo cell datasheet |
| Solar panel | 5 W @ 6V peak (6 V × 0.83 A) | Voltaic / equivalent |
| Effective sun-equivalent hours per day in Singapore | 4.5 h (typical), 2.5 h (overcast week worst) | Singapore NEA solar atlas |
| Charging efficiency (solar → battery via TP4056) | 0.75 | Conservative |

---

## 2. Per-state current draw

These are measured/expected currents at the 3.7V battery (not at the 3.3V rail). Currents at 3.3V are scaled by 3.7/3.3 when reasoning at the battery.

| State | Current @ V_BATT | Justification |
|---|---|---|
| Deep sleep (S0) | ~50 μA worst case (~15 μA expected) | Sum of MCU RTC + LDO + RFM95W sleep + leakage (see schematic doc §5) |
| Env. sample (S1) — wake, I2C BME280 + cap. probe + log | 18 mA × 1.5 s | ESP32-S3 active @ 240 MHz with light I/O |
| Acoustic capture (S2) — I2S streaming for 4 s | 25 mA × 4 s | INMP441 active + I2S RX DMA |
| Acoustic inference (S3) — 1D-CNN INT8 | 110 mA × 0.2 s | ESP32-S3 vector-instruction sustained load |
| LoRa TX (S4) — packet send at +14 dBm | 130 mA × 0.5 s | RFM95W TX + MCU active |

Numbers are inferred from datasheet typicals plus published ESP32-S3 power-profiling work (Espressif AN, Edge Impulse benchmarks).

---

## 3. Per-day energy accounting (typical site)

### 3.1 State durations per day

| State | Events/day | Duration each | Total active duration |
|---|---|---|---|
| Env. sample (S1) | 288 (= 86400 / 300) | 1.5 s | 432 s |
| Acoustic capture (S2) | 6 | 4 s | 24 s |
| Acoustic inference (S3) | 6 | 0.2 s | 1.2 s |
| LoRa TX (S4) | 4 | 0.5 s | 2 s |
| Deep sleep (S0) | continuous | remainder | 85,940 s |

Cross-check: 432 + 24 + 1.2 + 2 + 85940.8 = 86400 ✓

### 3.2 Charge per state

```
Q(S) = I(S) × t(S) / 3600   [mAh]
```

| State | I × t (mA·s) | mAh per day |
|---|---|---|
| S0 (deep sleep, 50 μA × 85940 s) | 4297 | 1.19 |
| S1 (env. sample, 18 mA × 432 s) | 7776 | 2.16 |
| S2 (acoustic capture, 25 mA × 24 s) | 600 | 0.17 |
| S3 (acoustic inference, 110 mA × 1.2 s) | 132 | 0.04 |
| S4 (LoRa TX, 130 mA × 2 s) | 260 | 0.07 |
| **Daily total** | | **≈ 3.6 mAh/day** |

So roughly **3.6 mAh/day**, with the whitepaper's stated 4.3 mAh/day providing a 20% engineering margin for non-modelled effects (transients on wake, GPIO charging spikes, occasional re-join handshakes, etc.).

### 3.3 Days of operation from full charge, zero solar

```
days = 2000 mAh / 3.6 mAh/day ≈ 555 days
```

The "no-solar" runtime is essentially "the battery's cycle life" rather than "the system's actual endurance." This is the *most pessimistic* scenario; real operation includes solar charging.

---

## 4. Per-day energy accounting (worst case)

### 4.1 What changes

Worst case: site with persistent stagnant water in always-favourable conditions, so the env. gate is open for most wakes. This causes the acoustic stage to fire much more often.

| State | Events/day | Total active duration | mAh/day |
|---|---|---|---|
| S0 (deep sleep) | continuous | 85,200 s | 1.18 |
| S1 (env. sample) | 288 | 432 s | 2.16 |
| S2 (acoustic capture, 36 events) | 36 | 144 s | 1.00 |
| S3 (acoustic inference, 36 events) | 36 | 7.2 s | 0.22 |
| S4 (LoRa TX, ~20 events) | 20 | 10 s | 0.36 |
| **Daily total (worst case)** | | | **≈ 4.9 mAh/day** |

```
days = 2000 mAh / 4.9 mAh/day ≈ 408 days zero-solar
```

Even in worst case, a fully-charged battery alone gives more than a year of operation.

---

## 5. Solar harvest

### 5.1 Typical Singapore conditions

```
Daily solar input ≈ 5 W × 4.5 h × 0.75 efficiency = 16.9 Wh/day
                  = 16.9 Wh / 3.7 V = 4570 mAh/day equivalent
```

Compared to the 3.6 mAh/day consumption, the solar harvest is ~1,200× the daily draw. The system is *vastly* over-provisioned, by design — to remain robust against partial-shade sites.

### 5.2 Overcast week worst case

If a site gets only 2.5 h of effective sun for a week:

```
Daily input = 5 W × 2.5 h × 0.75 = 9.4 Wh = 2540 mAh-equivalent/day
```

Still ~700× the consumption. The system is essentially solar-independent in any realistic conditions.

### 5.3 Persistent deep shade (worst case)

A node placed in a deep drain or interior void-deck location may receive only ~30 minutes of effective light through indirect/reflected paths:

```
Daily input ≈ 5 W × 0.5 h × 0.75 = 1.9 Wh = 510 mAh-equivalent/day
```

Still ~140× the consumption. The system continues to fully charge even in shade — though we'd visit these sites quarterly to check the panel for fouling.

### 5.4 Total enclosure failure / panel destroyed

In the case of total solar loss (panel cracked, completely buried under debris, etc.), the system runs on battery alone:

- Typical: 555 days
- Worst-case site activity: 408 days

In both, the next quarterly maintenance visit catches the issue well before battery depletion.

---

## 6. Sensitivity analysis

Which inputs most affect the budget?

| Parameter | If 2× | Effect on daily mAh |
|---|---|---|
| Deep sleep current (50 → 100 μA) | +1.2 mAh | budget grows to 4.8 — still fine |
| Env. sample period (300 → 600 s) | -1.0 mAh | budget shrinks to 2.6 |
| Acoustic events per day (6 → 12) | +0.2 mAh | negligible |
| LoRa SF (10 → 12) | TX duration 4× | adds 0.2 mAh |

The dominant terms are **deep sleep current** and **env. sample frequency**. This explains the design priority on the MCP1700 low-Iq LDO and the relatively long 5-minute env. sample period.

---

## 7. Validation plan

The numbers above are calculated; they need to be **measured** in Phase 1:

1. **Bench measurement** of deep-sleep current using μCurrent Gold (item in lab/test BOM). Target: ≤ 60 μA.
2. **Bench measurement** of each active-state current using INA219 power monitor in series with the battery during a controlled cycle. Target: within 20% of the calculated values.
3. **24-hour bench profile** — log mAh consumed over a representative 24-hour cycle including simulated event triggers. Target: ≤ 5 mAh/day.
4. **30-day field battery trend** — Phase 2 deliverable: deploy 3 nodes outdoors, log battery voltage daily, verify net-positive charging over the 30-day window.

If validation finds substantial deviation from these calculated numbers, the firmware is the easier lever: env. sample period can be doubled to 10 minutes for a 30%+ reduction in active duty cycle, with only a small loss in temporal resolution that is acceptable for breeding-site detection (mosquito life cycle measured in days, not minutes).

---

## 8. Conclusion

A single Hatch node, sized to a 5 W solar panel and 2000 mAh LiPo, has:

- **Typical** consumption: 3.6 mAh/day (well under the 4.3 mAh/day whitepaper figure).
- **Worst-case** consumption: 4.9 mAh/day.
- **Typical** solar harvest: 4570 mAh/day-equivalent (≫ consumption).
- **No-solar** endurance: 12–18 months on battery alone.

The system is designed with substantial margin in every direction. Field deployment for the 12-month grant period requires no maintenance other than the planned quarterly site visits.
