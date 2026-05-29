# Exuvia— Technical Whitepaper

**A multi-modal edge-AI sensor network for proactive *Aedes aegypti* breeding-site detection in Singapore's HDB environment.**

---

## 1. Problem statement

### 1.1 Dengue in Singapore 

Dengue is endemic in Singapore. Despite one of the most aggressive vector-control programmes in the world, the country has experienced repeated multi-thousand-case outbreaks throughout the last decade, with a record 35,315 cases in 2020 and 13,655 cases with 17 fatalities in 2024. A systematic review of the disease's economic burden estimated total costs of SGD 148 million across the 2010–2020 period. The 2025 case count has dropped to roughly 4,000 — but public-health authorities have publicly warned that a shift in the dominant viral serotype (toward DENV-3, for example) could trigger another major outbreak in subsequent years.

The proximate cause of transmission is well-understood: *Aedes aegypti*, an anthropophilic urban-adapted mosquito species that breeds in small water collections — bottle caps, plant pot saucers, blocked roof gutters, and most importantly the floor traps and drains of high-density public housing.

### 1.2 What NEA already does

The National Environment Agency operates a layered control system:

1. **Surveillance.** A nationwide network of over 64,000 Gravitraps deployed at approximately one trap per twenty households in HDB residential blocks. Mosquitoes are collected weekly and identified manually in the lab. From this data, NEA computes the Gravitrap *aegypti* Index (GAI), a normalized estimate of adult female *Ae. aegypti* abundance.
2. **Wolbachia.** Project Wolbachia – Singapore releases male *Wolbachia*-infected *Ae. aegypti* in selected estates. Field studies have demonstrated up to 75% reduction in dengue cases at release sites.
3. **Source reduction.** NEA officers conduct inspections of residential, construction, and public premises to remove standing water. This is the stated *primary strategy* of the programme.
4. **GIS-based risk mapping.** Random-forest models incorporating Breteau Index, climate data, and historical case counts predict outbreak risk on a grid basis up to 12 weeks ahead.
5. **Inter-agency coordination.** Through the Inter-Agency Dengue Task Force, surveillance signals are translated into coordinated action by NEA, town councils, MOH, and others.

This is a serious, mature programme. Any new technology proposed for the Singapore context has to interlock with it, not replace it.

### 1.3 The structural gap

The programme has one structural limitation that any added sensing capability could address: **all of its detection modalities count adult mosquitoes after they have emerged.** Gravitraps catch gravid adults. Inspections look for current standing water, but only when an inspector physically arrives. The GIS risk model is downstream of these.

Under Singapore's tropical conditions, the *Ae. aegypti* life cycle from egg to biting adult can be completed in **5 to 7 days** depending on temperature. A surveillance system with a weekly cadence is, by construction, one generation behind the breeding event. By the time a Gravitrap signal flags an elevated index in a zone, the adults producing the signal have likely already laid the next batch of eggs.

What is structurally missing is a **continuous, hyper-local, breeding-site-level signal** that detects favorable breeding conditions in the day-to-day rather than week-to-week — and that, ideally, *confirms* the favorable conditions are actually attracting Aedes activity. This is the gap Exuvia is designed to close.

---

## 2. Project concept

### 2.1 The reframing insight

Most prior work in automated mosquito surveillance treats acoustic detection — classifying species from wingbeat sounds — as the primary signal. This has a known failure mode: outdoor environments are acoustically noisy. Wind, traffic, air-conditioner condensers, household electronics, human voices, and other insects all contribute background that degrades classifier performance. Recent work like MosquitoSong+ (2024) has made noise-robustness a research focus precisely because of this issue.

Exuvia's key design insight is to **invert the signal hierarchy**. The primary signal is *environmental*: is there water here, has it been stagnant long enough, is the temperature in the Aedes breeding range? The acoustic signal is *secondary*: given that the environmental conditions are right, does a microphone in the area pick up wingbeat activity that resembles *Aedes*?

This inversion has three consequences:

1. **False-positive collapse.** A single noisy chirp that pattern-matches Aedes-like frequencies cannot trigger an alert by itself; the environmental gate has to be open simultaneously.
2. **Actionable alerts.** The alert is associated with a specific physical breeding site (the location of the sensor), not "an area where mosquitoes were heard." This makes it directly compatible with NEA's source-reduction workflow.
3. **Power efficiency.** The acoustic ML stage — by far the most energy-hungry component — only runs when the environmental gate is open. This makes year-round solar-powered operation tractable.

### 2.2 Sensing modalities

A Exuvia node integrates the following sensors:

| Modality | Sensor | What it tells us |
|---|---|---|
| Water presence | Capacitive PCB trace probe | Is there standing water at this site, right now? |
| Stagnation | Derived from water-presence time series | How long has water been continuously present? |
| Temperature | BME280 | Is T in the Aedes breeding range (≈22–34 °C)? |
| Humidity | BME280 | Tropical conditions confirmation |
| Adult acoustic | INMP441 MEMS microphone | Are Aedes-class wingbeats present in ambient audio? |
| Larval acoustic | Piezo contact element (optional) | Are mosquito larvae surfacing for air in this water? |

The capacitive probe is implemented as exposed copper traces on the bottom of the node's PCB rather than as a discrete water sensor. This avoids the corrosion failure mode of resistive moisture sensors deployed in tropical urban water, which are notorious for short field lifetimes.

### 2.3 On-device decision logic

At each 5-minute wake cycle, the node executes:

```
loop:
  sleep(5 minutes)
  e = read_environmental()
  update stagnation_timer based on e.water_present
  
  score = environmental_favorability(e, stagnation_timer)
  
  if score > THRESHOLD_GATE:
      audio = capture_audio(4 seconds, 16kHz)
      cls   = run_tinyml(audio)
      
      if cls.confidence > 0.78 and cls.label in {AEDES_AEGYPTI, AEDES_ALBOPICTUS}:
          send_alert(node_id, e, cls)
      
  send_summary_if_due()
```

The environmental favorability function is a simple weighted combination:

```
score = 0.45 · I(water_present > 24h)
      + 0.25 · I(25°C ≤ T ≤ 33°C)
      + 0.15 · I(RH > 70%)
      + 0.15 · I(no_recent_flow)
```

This is intentionally a transparent, hand-tuned function rather than a learned model. The grant panel and any operational stakeholder (a town council officer reviewing alerts) needs to be able to interrogate *why* a given alert fired; a black-box gate would undermine the system's credibility. The acoustic stage can use a learned model precisely because its role is narrower: confirm species, not decide whether to act.

---

## 3. System architecture

### 3.1 Layered overview

The system has four layers:

**Edge node.** ESP32-S3 microcontroller with attached sensors, LoRa radio, LiPo battery, and solar charge controller, housed in an IP66 enclosure with appropriate cable glands. Designed for unattended outdoor operation in Singapore's tropical climate.

**LoRaWAN gateway.** A single indoor multi-channel LoRaWAN gateway (e.g., RAK7268V2) covers a deployment area of roughly 1–2 km radius depending on building density. AS923 frequency plan for Singapore.

**Cloud backend.** Mosquitto MQTT broker → TimescaleDB time-series database → REST API. Hosted on a small cloud instance.

**Operator dashboard.** A web application showing per-block aggregated risk scores, individual node history, and an alert log. Designed around the operator workflow of a town council vector-control officer.

### 3.2 Edge node power budget

For a single node sized to a 5W solar panel and 2000 mAh LiPo:

| State | Current | Daily duration (typical) | Daily mAh |
|---|---|---|---|
| Deep sleep | 50 μA | 22 hours | 1.1 |
| Environmental sample | 18 mA | 288 × 1.5s = 432s | 2.2 |
| Acoustic capture + inference | 110 mA | ~6 events/day × 5s = 30s | 0.9 |
| LoRa TX (alert + summary) | 130 mA | ~4 events/day × 0.5s = 2s | 0.07 |
| **Total daily energy** | | | **~4.3 mAh/day** |

Against a 2000 mAh LiPo, this gives more than a month of operation in zero-charging conditions (extreme worst case of total enclosure shading). With even partial sun exposure through a 5W panel — typical for HDB common areas — net charging exceeds net discharge by an order of magnitude. The system is sized for indefinite unattended operation.

### 3.3 Why LoRaWAN

WiFi is unavailable in most candidate deployment sites — HDB drains, void-deck planters, bin centres, rooftop gutters are not WiFi-covered. Cellular (LTE-M/NB-IoT) is feasible but adds per-node SIM costs and module costs. LoRaWAN gives:

- Multi-kilometer range through urban environment
- Single gateway covers an entire estate's worth of nodes
- Very low TX power → low battery drain
- AS923 is a license-exempt band in Singapore
- Existing public networks (e.g., TheThingsNetwork) could be leveraged in expansion

The trade-off — limited uplink bandwidth (~24 bytes/uplink at SF10, with strict duty-cycle limits) — is not a constraint for this application. Alert packets and daily summaries are small.

### 3.4 Why ESP32-S3

The ESP32-S3 is the right MCU class for this application:

- Dual-core Xtensa LX7 @ 240 MHz with vector instructions accelerating INT8 neural-network inference
- 512 KB SRAM + 8 MB PSRAM (sufficient for a small mel-spectrogram pipeline + 1D-CNN)
- Native I2S support for the INMP441 microphone
- Mature deep-sleep modes (~10 μA achievable)
- Extensive community support and Edge Impulse first-class support
- Per-unit cost: USD 5–8 in single-board form (Seeed XIAO ESP32-S3 Sense includes mic and PSRAM in one board for ~USD 14)

Alternatives considered:

- *Arduino Nano 33 BLE Sense*: used by Altayeb et al. (2022), but only 256 KB RAM, no PSRAM, weaker NN performance per watt.
- *Raspberry Pi Zero 2 W*: more compute headroom, but ~20× the standby power. Inappropriate for solar/battery operation.
- *ESP32-S3 with external camera module*: future expansion path (visual larvae detection), evaluated but deferred from grant scope to keep complexity bounded.

---

## 4. Signal processing pipeline

### 4.1 Acoustic stage in detail

When the environmental gate opens, the node executes the following acoustic pipeline:

```
1. Buffer 4 seconds of audio at 16 kHz, 16-bit signed PCM (128 KB raw)
2. Apply pre-emphasis filter (high-pass at 80 Hz to suppress low-freq HVAC rumble)
3. Frame into 25-ms windows with 10-ms hop → 397 frames per 4s capture
4. Compute mel-spectrogram: 40 mel bins, 64-point FFT per frame
5. Per-channel energy normalization (PCEN), shown by HumBug to outperform log-mel for noisy field audio
6. Stack last 64 frames into a (40 × 64) input tensor
7. Run INT8 quantized 1D-CNN: target inference latency < 200 ms on ESP32-S3
8. Output: 4-class softmax → {Ae. aegypti, Ae. albopictus, other_insect, noise}
9. Apply confidence threshold + temporal voting (require ≥3 of last 5 inferences agree)
```

The temporal voting step is critical: a single detection event in a noisy environment is not enough. Five consecutive inference windows that vote majority-Aedes constitute a confirmed acoustic detection.

### 4.2 ML training and quantization

Training is conducted off-device on a workstation:

1. **Data sources.** The HumBug corpus (~6,900 wild-captured mosquito recordings across 6 genera) provides the bulk training data. The Abuzz dataset from Stanford provides additional Aedes/Culex examples. NEA-Wolbachia-related published recordings, where available, supplement.
2. **Augmentation.** Background noise injection (urban traffic, HVAC, rain) drawn from FSD50K dataset; pitch shift ±10% to simulate temperature-driven wingbeat frequency variation; random time masking to simulate partial captures.
3. **Architecture.** 1D-CNN with 4 conv blocks (channels 16/32/64/128, kernel 3, stride 2), batch norm, ReLU, global average pool, dense softmax. ~40 KB INT8 footprint after quantization, well within ESP32-S3 RAM budget.
4. **Quantization.** Post-training INT8 quantization with TensorFlow Lite, calibrated on a held-out subset of the training data.
5. **Validation.** Target accuracy on held-out HumBug subset: ≥90% (4-class). Target accuracy in field-recorded SG validation set (collected during Phase 3): ≥75% (worst case admitted before re-training).

The training pipeline lives in `/ml/notebooks/` and is fully reproducible from the public datasets.

### 4.3 Fine-tuning on Singapore-specific data

A risk of using a corpus collected primarily in Africa and Southeast Asia field studies is that the local Singapore population may have distributional drift — different background-noise profile (HVAC-heavy HDB corridors), slightly different temperature regimes affecting wingbeat frequency, mixed populations of *Ae. aegypti* and *Ae. albopictus* in the same area, etc.

During Phase 3, each deployed node captures audio snippets (privacy-preserving: never speech-band; high-pass filtered at 80 Hz; no continuous streaming, only event-triggered short clips) which are uploaded to the cloud backend for manual or semi-supervised labeling. These snippets fine-tune the production model on a rolling basis. A model versioning system in the dashboard tracks which firmware version is running on each node.

---

## 5. Field study design

### 5.1 Pilot deployment plan

Phase 3 deploys 10 nodes across two contrasting environments:

- **Cluster A (5 nodes): NTU campus.** Specifically the drainage channels around residential halls and the EEE block. Provides easy physical access for maintenance, controlled stakeholder relationships, and a known low-baseline dengue area for false-positive characterisation.
- **Cluster B (5 nodes): adjacent HDB estate.** A neighbourhood in the immediate vicinity of NTU (Jurong West / Pioneer) where the applicant can credibly engage with the town council and where adult Aedes are known from NEA's published Gravitrap data. This cluster tests the system in operationally relevant conditions.

Each node is co-located with an existing NEA Gravitrap where possible — not to replace it, but to enable direct comparison of Exuvia's site-level alerts against the Gravitrap Aedes Index at that location.

### 5.2 Success metrics

The proposal commits to the following measurable outcomes at end of Phase 3:

- **Operational uptime ≥ 90%** per node across an 8-week deployment.
- **Acoustic classifier accuracy ≥ 75%** on field-recorded test set with Singapore-specific noise profile.
- **At least one confirmed breeding site identified and acted upon** by a stakeholder (NTU campus services or HDB town council).
- **Per-node alert false-positive rate ≤ 1 per week** under normal urban acoustic conditions.

These are stretch goals, not minimum-passing thresholds — partial achievement still constitutes a meaningful project outcome, and the proposal does not assume operational adoption by NEA itself within the grant period (that is a multi-year process beyond a Y1 student's scope).

### 5.3 Ethics, permissions, and data handling

The microphone in each node never streams continuous audio. The I2S interface is gated off by the firmware except during the ~5-second event window after the environmental gate triggers, and even then only short clips around suspected detections are uplinked.

For NTU campus deployment, signage will indicate the presence of the sensors and a short FAQ will be posted with a contact channel. For HDB deployment, formal permissioning through the relevant town council and NEA will be obtained before any installation; no node will be installed on private property.

A data-handling plan in `/docs/deployment-plan.md` covers retention policy (raw audio purged after 30 days), publication policy (any released dataset will exclude any clips containing identifiable human speech), and incident response.

---

## 6. Risk register summary

A full risk register lives in [/docs/risk-register.md](./risk-register.md). The most material risks:

| Risk | Likelihood | Impact | Mitigation |
|---|---|---|---|
| ML field accuracy < 75% target | Medium | Medium | Fine-tuning pipeline + transparent environmental gate that still provides value alone |
| Capacitive probe drift / fouling | Medium | Low | Probe geometry calibration; firmware self-test on each wake |
| LoRaWAN coverage gap | Low | Medium | Pre-survey of candidate sites; gateway placement option to expand |
| Enclosure water ingress | Medium | High | IP66 housing; full thermal-cycling test in Phase 2 |
| Town council non-engagement | Medium | Medium | Begin engagement during Phase 1; NTU-only pilot is fallback |
| Component obsolescence | Low | Low | Use mainstream parts; 10% spare unit ratio |
| Schedule slip | Medium | Low | Y1 student is solo, but phases are modular — failure at one phase doesn't cascade |

### 6.1 The graceful-failure property

A meaningful design property of the Exuvia system: even if the acoustic ML stage performs poorly in the field, the environmental-only signal is independently useful. A node that reliably reports "this drain has had standing water for 5 days at 28°C" is *already* a deployable source-reduction tool for town councils, with or without acoustic confirmation. This is what reduces the project's overall execution risk: the most uncertain technical component (field ML performance) is not on the critical path to demonstrable value.

---

## 7. Beyond the Funding

If the field study validates the approach, the realistic next steps — for which Exuvia is *not* requesting grant funding — include:

- Engagement with NEA's Environmental Health Institute for a larger-scale pilot.
- Partnership with one or more HDB town councils for an estate-wide deployment.
- Potential commercialisation pathway through NTUitive (NTU's innovation and enterprise company), with the entrepreneurship-major background a relevant supporting factor.
- Open-source release of all firmware, hardware, ML pipeline, and validated dataset.
- A peer-reviewed publication targeting a venue such as IEEE Sensors Journal, ACM SenSys, or BMC Public Health.

The grant funds the *demonstration*. Everything past that is my own work.
