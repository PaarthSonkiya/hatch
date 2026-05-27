# Schematic Block Diagram

**Hatch — Single-node electrical architecture description.**

This document describes the electrical block diagram of a single Hatch field node. A formal schematic (KiCad) will be produced during Phase 1 of the grant; this document establishes the design intent that the schematic will implement.

---

## 1. Top-level block diagram

```
                                ┌───────────────────────────────────┐
                                │       SOLAR + BATTERY DOMAIN      │
                                │                                   │
   ☀ 5W panel ──── + ───────────┤  TP4056   ◯  3.7V LiPo  (2000mAh) │
                   - ───────────┤  DW01A    │                       │
                                │   chgr ───┴────► V_BATT (3.0–4.2) │
                                └─────────┬─────────────────────────┘
                                          │
                                ┌─────────▼─────────┐
                                │   MCP1700-3302E   │ (low-Iq LDO)
                                │       LDO         │
                                └─────────┬─────────┘
                                          │ 3V3 (always)
                                          │
                ┌─────────────────────────┼──────────────────────────────┐
                │                         │                              │
        ┌───────▼───────┐         ┌───────▼───────┐             ┌────────▼────────┐
        │  SENSOR RAIL  │         │   MCU CORE    │             │   MIC RAIL      │
        │  (gated)      │         │ XIAO ESP32-S3 │             │   (gated)       │
        │               │         │   Sense       │             │                 │
        │   ┌─────────┐ │         │               │             │  ┌───────────┐  │
        │   │ BME280  │◄┼─I2C─────┤               ├─I2S─────────┼──┤  INMP441  │  │
        │   └─────────┘ │         │               │             │  └───────────┘  │
        │               │         │               │             │                 │
        │   ┌─────────┐ │         │               │             │  ┌───────────┐  │
        │   │ Cap.    │◄┼─touch───┤               │             │  │  Piezo    │  │
        │   │ probe   │ │         │   PSRAM 8MB   │             │  │  hydro.   │◄─┼─analog
        │   │ (PCB    │ │         │               │             │  │  (opt.)   │  │
        │   │  trace) │ │         └───┬──────┬────┘             │  └───────────┘  │
        │   └─────────┘ │             │      │                  └─────────────────┘
        └───────────────┘             │      │                          ▲
                                      │      │ SPI                      │
                                      │      ▼                          │
                                      │  ┌──────────┐                   │
                                      │  │ RFM95W   │                   │
                                      │  │ LoRa     ├──► SMA → antenna  │
                                      │  └──────────┘                   │
                                      │                                 │
                                      │ GPIO (SENSOR_PWR_EN) ───────────┤
                                      │                                 │
                                      │ GPIO (MIC_PWR_EN) ──────────────┘
                                      │
                                      └── ADC ──► V_BATT divider
```

---

## 2. Power architecture

### 2.1 Solar / battery path

The solar panel feeds the TP4056 LiPo charging IC, which manages constant-current / constant-voltage charging of the 2000 mAh LiPo battery. The DW01A protection IC provides over-discharge, over-charge, and over-current protection. The output is `V_BATT`, the unregulated battery rail (3.0–4.2 V depending on state of charge).

### 2.2 3V3 rail (always-on)

`V_BATT` feeds the MCP1700-3302E LDO, which produces a clean 3.3 V rail. The MCP1700 is chosen specifically for its very low quiescent current (~1.6 μA typical), which dominates the deep-sleep current budget. Higher-Iq LDOs (e.g., AMS1117 at ~5 mA Iq) would single-handedly blow the deep-sleep budget.

### 2.3 Gated rails

The sensor rail (BME280 + capacitive probe bias) and the microphone rail (INMP441 + optional piezo amplifier) are switched off in deep sleep. The MCU drives GPIO `SENSOR_PWR_EN` and `MIC_PWR_EN` high to enable them during active phases. This requires either:

(a) low-side N-MOSFETs (BSS138 or similar) controlling the rail's ground return, or
(b) high-side P-MOSFETs (PMV48XP or similar) controlling the rail's positive feed.

Choice: **high-side P-MOSFETs** for both rails. Reason: the sensors share a ground with the MCU for I2C/I2S signal integrity, so low-side switching would cause noise problems.

### 2.4 Battery measurement

A 100k / 100k resistive divider on `V_BATT` brings the worst-case 4.2 V down to 2.1 V, safely within the ESP32-S3 ADC input range. The divider is connected through a 1 MΩ series resistor and only sampled briefly during summary packet construction, contributing negligible idle current (~4 μA continuous worst case at 4.2 V).

---

## 3. Sensor interfaces

### 3.1 I2C bus (BME280)

Standard I2C at 100 kHz with 4.7 kΩ pull-ups to 3V3. BME280 default address 0x76 (fallback 0x77 via SDO pin).

### 3.2 I2S bus (INMP441)

Three-wire I2S: BCLK (bit clock), LRCK (word select), DIN (data in from microphone). The ESP32-S3 I2S peripheral handles clock generation; INMP441 samples on the BCLK edge selected by L/R framing.

INMP441 is mounted with its acoustic port facing **outward** through a small (3 mm diameter) hole in the enclosure side wall, protected from rain by a small hooded cover (3D-printed) and a hydrophobic mesh (waterproof PTFE membrane). This is the standard approach used in outdoor MEMS-mic deployments and is documented to work in IP66 contexts.

### 3.3 Capacitive water probe

The probe is implemented as a pair of interdigitated copper traces on the bottom side of the main PCB, ~30 mm × 15 mm interleaved. The traces are exposed through the solder-mask opening and conformally coated with a thin (~50 μm) layer of acrylic conformal coating (Electrolube APL).

When water bridges the trace pair, capacitance increases by 2–10× depending on water purity. The ESP32-S3 touch-sense peripheral measures this capacitance against an adaptive baseline persisted in NVS. The mathematics of "wet vs dry" detection is handled in `sensors.cpp`.

The conformal coating is the critical engineering choice: bare copper would corrode within weeks in tropical outdoor conditions. Conformal coating extends probe life to years while only marginally reducing sensitivity (water still couples capacitively through the thin coat).

### 3.4 Piezo hydrophone (optional)

A 27 mm PZT (lead zirconate titanate) disc, encapsulated in epoxy in a 3D-printed waterproof housing, connected via a 1m shielded cable to the main PCB. The piezo's high-impedance output is buffered by an MCP6022 op-amp configured as a non-inverting amplifier with gain 100×, then fed to an ESP32-S3 ADC input.

The hydrophone is mounted *in* the water (or in physical contact with the substrate carrying water, such as a drain wall). It picks up the low-frequency surface tension events of mosquito larvae surfacing to breathe — a documented acoustic signature in the larval-detection literature.

This feature is exploratory: only ~half of the deployed nodes will carry hydrophones. Comparison data between hydrophone-equipped and acoustic-only nodes informs whether the hydrophone is worth carrying forward to a larger deployment.

---

## 4. LoRa interface

The RFM95W radio module communicates with the ESP32-S3 over SPI (CS, CLK, MOSI, MISO) plus reset and DIO0 (interrupt) lines. The radio is powered from the 3V3 always-on rail rather than a gated rail; it spends most of its time in its own low-power sleep mode (~0.2 μA), which is more power-efficient than fully cycling it.

The RFM95W is operated in LoRa modulation mode on the AS923-1 frequency plan (923.2 MHz, SF10, BW 125 kHz, CR 4/5). With +14 dBm TX power and a 5 dBi gateway antenna, link budget calculations give ~5 km range in line-of-sight conditions and ~1 km through dense HDB building density.

---

## 5. Deep-sleep current path

When `esp_deep_sleep_start()` is called, the following are still drawing current:

| Component | Sleep current | Notes |
|---|---|---|
| ESP32-S3 RTC + ULP domain | ~10 μA | RTC timer running for wake |
| MCP1700-3302E LDO | ~1.6 μA | Quiescent |
| BME280 (powered down by gate) | 0 | Rail off |
| INMP441 (powered down by gate) | 0 | Rail off |
| RFM95W in LoRa sleep mode | ~0.2 μA | Module's own low-power state |
| Resistive dividers + leakage | ~3 μA | Sum of small things |
| **Total deep-sleep current** | **~15 μA worst case, ~50 μA budget** | Comfortable margin |

The 50 μA budget in the whitepaper is intentionally conservative — actual measured numbers should be substantially better. Validation via μCurrent Gold (item in the lab/test BOM) is a Phase 2 deliverable.

---

## 6. Risks at the schematic level

| Risk | Mitigation |
|---|---|
| PSRAM signal-integrity sensitivity → use a board (XIAO Sense) that has it correctly routed |
| LDO oscillation under varying load → use a stable LDO (MCP1700 is well-characterised) with proper bulk + ceramic decoupling |
| GPIO power leakage during sleep → tie unused GPIOs to ground or pull them with on-chip pulls |
| Solar reverse-bias at night → TP4056 has reverse-current protection; SOL1 + Schottky diode in series adds belt-and-braces |
| LiPo over-temperature → DW01A protects against over-discharge; thermal protection at the cell level requires a NTC thermistor + firmware monitor, deferred from v1 |

---

## 7. Schematic deliverable

The formal KiCad schematic + PCB files will be added to `/hardware/kicad/` during Phase 1, prior to PCB fabrication. They will be CERN-OHL-W licensed at end of project, alongside the firmware MIT and dataset CC-BY release.
