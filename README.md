# Hatch

**Multi-modal edge-AI sensors for proactive *Aedes aegypti* breeding-site detection in Singapore's HDB environment.**

> *Catch breeding sites before they hatch.*

[Landing page](./landing/index.html) · [Whitepaper](./docs/whitepaper.md) · [Literature review](./docs/literature-review.md) · [Firmware](./firmware/) · [ML pipeline](./ml/) · [Dashboard](./dashboard/) · [Hardware BOM](./hardware/bom.md)

---

## Summary

Singapore's dengue surveillance is among the most sophisticated in the world. NEA operates 64,000+ Gravitraps, runs the Wolbachia release program, performs source-reduction inspections, and maintains GIS-based risk maps. **And it still recorded 13,655 dengue cases and 17 deaths in 2024.**

The structural reason: existing tools **count adult mosquitoes after they have emerged.** Gravitraps sample weekly. An *Aedes aegypti* egg becomes a biting adult in 5–7 days under Singapore's climate. By the time a hotspot is flagged, the next generation is already biting.

THis is why I built Hatch, a low-cost edge sensor designed to live *at the breeding site itself* like drains, void-deck planters, bin centres, rooftop gutters and flag a stagnant water collection *before* the adults emerge. It uses two sensing modes:

1. **Environmental Watch (always-on, ~50 μA):** capacitive water presence + stagnation timer + temperature/humidity → on-device breeding-favorability score.
2. **Acoustic Confirmation (triggered, TinyML):** when conditions favor breeding, the node wakes a MEMS microphone and runs a quantized 1D-CNN trained on the HumBug wingbeat corpus to confirm Aedes activity nearby.

A high-confidence alert requires **both** modes to agree. This is what lets the system survive in a noisy HDB common-area environment where pure acoustic classification would drown in false positives.

---

## Why this matters

| | Existing approach | Hatch |
|---|---|---|
| **What it counts** | Adult mosquitoes after emergence | Stagnant breeding sites *before* emergence |
| **Latency** | 7–14 days (weekly manual count + lab ID) | Continuous, alerts within hours |
| **Spatial resolution** | Block-level (Gravitrap Aedes Index) | Site-level (per-drain) |
| **False-positive load** | Low but slow; manual disposal of unrelated catches | Low by design — requires environmental gate |
| **Aligned with NEA doctrine** | Surveillance | **Source reduction** — the stated primary strategy |
| **Per-unit cost target** | Gravitrap: ~SGD 30 + labor; BG-Counter: ~SGD 600+ | **<SGD 100 BOM at scale**, fully autonomous |

---

## Repository map

```
hatch/
├── README.md                  ← you are here
├── landing/                   ← public-facing project page (HTML)
│   └── index.html
├── docs/                      ← all written research and planning
│   ├── whitepaper.md          ← full ~6,000-word technical document
│   ├── literature-review.md   ← annotated bibliography, 25+ papers
│   ├── deployment-plan.md     ← field study plan, site selection, ethics
│   ├── risk-register.md       ← itemised risks + mitigations
│   └── competition-brief.md   ← targeted competition submission notes
├── firmware/                  ← ESP32-S3 PlatformIO project
│   ├── platformio.ini
│   ├── src/
│   │   ├── main.cpp
│   │   ├── sensors.cpp + .h
│   │   ├── acoustic.cpp + .h
│   │   ├── power.cpp + .h
│   │   └── comms.cpp + .h
│   └── README.md
├── ml/                        ← Python ML pipeline
│   ├── notebooks/
│   │   ├── 01_data_exploration.ipynb
│   │   ├── 02_preprocessing.ipynb
│   │   └── 03_model_training.ipynb
│   └── README.md
├── dashboard/                 ← operator dashboard MVP
│   ├── index.html
│   ├── style.css
│   └── app.js
└── hardware/                  ← component selection + power budget
    ├── bom.md
    ├── schematic-block.md
    └── power-budget.md
```

---

## How the system works

### A node's day in the field

```
00:00 ────────────────────────────────────────────────────────────────────► 24:00
       │
       ├─ env. sample @ 5-min intervals  (capacitive H₂O, T, RH, dry-time)
       │
       ├─ env. gate evaluated:  has water been present > 24h AND 25°C < T < 33°C?
       │       │
       │       └─ NO  → return to deep sleep
       │       └─ YES → wake microphone (Mode 2)
       │
       ├─ acoustic capture: 4-second buffer @ 16kHz
       ├─ on-device INT8 1D-CNN inference (Aedes aegypti / Aedes albopictus / other / noise)
       │
       ├─ if Aedes detected with confidence > 0.78  →  ALERT packet via LoRa
       │
       └─ env. + audio + alert flags accumulated; daily summary uplinked
```

Average node power draw is dominated by the 5-minute environmental sample cycle. The acoustic stage only runs when the environmental gate is open, so daily energy consumption is bounded even with a power-hungry ML stage. See [hardware/power-budget.md](./hardware/power-budget.md) for the full calculation.

### From node to dashboard

Each node transmits compact alert packets (~16 bytes) and daily summaries (~64 bytes) via LoRaWAN to a campus gateway. The gateway forwards to a Mosquitto MQTT broker, which writes to TimescaleDB. The operator dashboard reads from the same DB and renders a per-block risk heatmap with drilldown to individual nodes.

This pipeline is conventional IoT plumbing; the novelty sits at the edge node, not in the cloud stack.

---

## Hardware at a glance

| Component | Part | Role |
|---|---|---|
| MCU | ESP32-S3 (Seeed XIAO ESP32-S3 Sense candidate) | Wakes on env. trigger; runs INT8 CNN; sleeps otherwise |
| Microphone | InvenSense INMP441 (I2S MEMS) | Acoustic capture for wingbeat classification |
| Water sensor | Capacitive probe (custom PCB trace) | Non-corrosive water presence + level proxy |
| Env. sensor | Bosch BME280 | Temperature, humidity, pressure |
| Hydrophone | Piezo contact element, drain-mounted | Optional: larval surfacing acoustic signature |
| Comms | Heltec WiFi LoRa 32 V3 module or RFM95W | LoRaWAN AS923 (Singapore) |
| Power | 5W solar + 3.7V 2000mAh LiPo + TP4056 PMIC | Months of unattended operation |
| Enclosure | IP66 ABS housing + cable glands + drain mount | Tropical humidity + monsoon |

Detailed selection rationale, alternates, and unit cost in [hardware/bom.md](./hardware/bom.md).

---

## Software at a glance

**Firmware** (`/firmware`): PlatformIO project targeting Arduino-ESP32 framework. Uses `esp_sleep` for deep sleep, `driver/i2s` for the INMP441, and TensorFlow Lite Micro for inference. Implements an event-loop state machine: `SLEEP → ENV_SAMPLE → ENV_GATE → ACOUSTIC_CAPTURE → INFERENCE → TX → SLEEP`.

**ML pipeline** (`/ml`): Python notebooks for HumBug dataset exploration, mel-spectrogram preprocessing, 1D-CNN training with TensorFlow, INT8 quantization with `tflite_micro`, and on-device latency profiling. Designed to be reproducible end-to-end from raw .wav files to deployed `model.tflite`.

**Dashboard** (`/dashboard`): Static HTML/JS prototype using Leaflet for the map layer and Chart.js for time-series. Reads from a mocked JSON endpoint; production version backed by TimescaleDB query API.

---

## What's in this repo as of grant submission

This repository was built **before** grant submission as a pre-grant artifact. None of the items below required the grant — they were completed at zero hardware cost using open datasets, simulation, and design work.

- ✅ Full system whitepaper with prior-art analysis and signal-processing pipeline design
- ✅ Annotated literature review of 25+ papers
- ✅ Firmware skeleton compilable under PlatformIO (no hardware required to build)
- ✅ ML training pipeline runnable on a laptop using publicly available HumBug data
- ✅ Working dashboard prototype with mock data
- ✅ Component-level hardware BOM with sourcing and power budget
- ✅ Field deployment plan with candidate sites and stakeholder workflow
- ✅ Risk register and competition brief

**What the grant funds:** hardware (10 field nodes + gateway), enclosure iteration, PCB fabrication runs, field deployment costs, competition fees, and the cloud/tooling subscriptions needed for the 12-month field study.

---

## Roadmap (12 months, under grant)

| Phase | Months | Gate |
|---|---|---|
| 1 — Bench prototype | M01–M03 | Solar-powered node running env-gated acoustic inference on lab samples |
| 2 — Enclosure + comms | M04–M06 | 3 nodes operating outdoors unattended for 2 weeks |
| 3 — Field study | M07–M09 | 10 nodes deployed; ≥1 alert acted upon by town council |
| 4 — Dissemination | M10–M12 | Open-source release; ≥1 international competition submission |

Detailed deliverable list per phase in [docs/whitepaper.md](./docs/whitepaper.md).

---

## Target competitions

The grant explicitly supports student participation in competitions. Hatch will be submitted to:

- **James Dyson Award** (Singapore national + international rounds)
- **IEEE R10 Humanitarian Technology Conference** (student paper track)
- **Asia Smart App Challenge** (IoT / health track)
- **NTU President's Entrepreneurship Award** (if Y2 status allows)
- **NEA innovation challenges** (when open)

Tailored submission outlines in [docs/competition-brief.md](./docs/competition-brief.md).

---

## Open source commitment

On successful field validation, this repository will be released under MIT (firmware, ML pipeline, dashboard) and CERN-OHL-W (hardware). The Singapore-specific field-recorded mosquito audio dataset will be released under CC-BY 4.0, contributing back to the global mosquito-acoustic research community whose work this project builds upon.

---

## About the applicant

NTU College of Engineering Y1 student, EEE primary with Entrepreneurship as second major. This proposal is a solo application within the ENGenious Sparks Grant guidelines.

The project deliberately stays within the scope an individual Y1 can credibly execute over 12 months: the technical novelty sits in the *system design* (multi-modal fusion + conditional inference + breeding-site placement), not in cutting-edge ML research that would require a lab of postdocs. Every claim in this document maps to a citation in the literature review or a working artifact in this repo.

---

## License

To be confirmed at first stable release. Intent: MIT for software, CERN-OHL-W for hardware, CC-BY 4.0 for the field dataset.
