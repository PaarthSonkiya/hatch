# Hatch — Bill of Materials

**Per-node component-level BOM with sourcing notes and alternates.**

This document covers the BOM for a single Hatch field node. Multiply by the deployment count (10 nodes in Phase 3) and add the shared infrastructure (gateway, lab equipment) for the full project budget. The grant budget category mapping is shown at the end.

---

## 1. Single-node BOM

### 1.1 Compute + connectivity

| Ref | Part | Function | Qty | Unit (SGD) | Source | Notes |
|---|---|---|---|---|---|---|
| U1 | Seeed XIAO ESP32-S3 Sense | MCU + onboard INMP441 mic + 8MB PSRAM | 1 | 19.50 | Seeed / SG distributor | Primary choice. Onboard mic eliminates separate I2S routing for v1. |
| U2 | RFM95W LoRa module (alt: Heltec WiFi LoRa 32 V3) | LoRaWAN AS923 radio | 1 | 22.00 | Cytron / Element14 | Heltec WiFi LoRa 32 V3 (ESP32-S3 + LoRa onboard) is the consolidated alternative; chosen if XIAO+RFM95 routing proves awkward |
| ANT1 | 868–923 MHz quarter-wave antenna, SMA | LoRa antenna | 1 | 4.50 | Cytron / RobotShop | Outdoor IP-rated mounting recommended |

### 1.2 Environmental sensing

| Ref | Part | Function | Qty | Unit (SGD) | Source | Notes |
|---|---|---|---|---|---|---|
| U3 | Bosch BME280 breakout (Adafruit / clone) | T / RH / P | 1 | 7.50 | Cytron / AliExpress | I2C interface, 0x76 or 0x77 |
| PCB1 | Custom capacitive water probe (PCB-trace) | Water presence | 1 | 0.80 | Fab on main PCB | Exposed interdigitated copper, conformal-coated |

### 1.3 Acoustic (if not using Sense board's onboard mic)

| Ref | Part | Function | Qty | Unit (SGD) | Source | Notes |
|---|---|---|---|---|---|---|
| MIC1 | InvenSense INMP441 breakout | I2S MEMS microphone | 1 | 4.50 | Cytron / AliExpress | Only needed if not using XIAO Sense (which has it built in) |
| MIC2 | DIY piezo hydrophone (PZT disc + waterproof housing) | Larval surfacing detection | 1 | 6.00 | Element14 + 3D-printed housing | Optional per-node; deployed on subset for comparison |

### 1.4 Power

| Ref | Part | Function | Qty | Unit (SGD) | Source | Notes |
|---|---|---|---|---|---|---|
| BAT1 | 3.7V 2000 mAh LiPo, JST-PH | Primary energy store | 1 | 12.00 | Cytron / Adafruit | Choose UL/CE-marked source; outdoor temperature rating to 60°C |
| SOL1 | 5W 6V monocrystalline solar panel, 165 × 135 mm | Energy harvesting | 1 | 18.00 | Voltaic / Cytron | IP67-rated edges; epoxy-encapsulated cells |
| U4 | TP4056 + DW01A protection charging module, micro-USB | LiPo charge + protection | 1 | 1.20 | Cytron | Spec for 1A charge; trim trace for 0.5A to extend cycle life |
| U5 | MCP1700-3302E LDO | 3.3V rail | 1 | 0.80 | Element14 | Low Iq (~1.6 μA) — critical to deep-sleep budget |
| C_BAT | 22μF/16V Tant + 0.1μF ceramic | Decoupling | 4 | 0.50 | Element14 | Standard hygiene |

### 1.5 Enclosure + mechanical

| Ref | Part | Function | Qty | Unit (SGD) | Source | Notes |
|---|---|---|---|---|---|---|
| ENC1 | IP66 ABS junction box, ~120×80×55 mm | Main housing | 1 | 9.00 | Element14 | Bud Industries or Hammond preferred |
| GLAND1 | M12 cable gland, IP68 | Probe + antenna pass-through | 2 | 1.50 | Element14 | One for solar lead, one for capacitive probe lead |
| MOUNT1 | Stainless steel wall bracket + cable ties | Site mounting | 1 | 4.50 | Hardware store | Site-specific; some sites need drain-mount adaptor |
| DESIC1 | Silica gel desiccant pack | Internal humidity control | 1 | 0.50 | AliExpress | Replaced quarterly |
| GASKET1 | Self-fusing silicone tape | Gland sealing reinforcement | 1 | 1.00 | Hardware store | Belt-and-braces seal |

### 1.6 PCB

| Ref | Part | Function | Qty | Unit (SGD) | Source | Notes |
|---|---|---|---|---|---|---|
| PCB | Custom 2-layer PCB, ~80×60 mm | Hosts U1, U3, U4, U5, capacitive probe traces, connectors | 1 | 3.00 | JLCPCB (amortised) | $2 each in qty 30 with assembly; trace exposure for capacitive probe |
| HDR | 0.1" headers + JST connectors | Inter-board connectivity | — | 1.50 | Element14 | |

### 1.7 Misc

| Ref | Part | Function | Qty | Unit (SGD) | Source | Notes |
|---|---|---|---|---|---|---|
| LBL1 | UV-resistant info sticker + QR code | Public-facing info | 1 | 0.50 | Print on weatherproof vinyl | |
| WIRE | 22 AWG silicone-jacketed wire, 1m | Internal harness | — | 1.00 | Element14 | |

### 1.8 Per-node total

| Subtotal | SGD |
|---|---|
| Compute + connectivity | 46.00 |
| Environmental + acoustic (using Sense board onboard mic; hydrophone on ~half of nodes) | 11.30 |
| Power | 32.50 |
| Enclosure + mechanical | 16.50 |
| PCB + connectors | 4.50 |
| Misc | 1.50 |
| **Per-node total (rounded)** | **~112** |

**Notes on the per-node figure:**

- Including XIAO ESP32-S3 Sense (with onboard INMP441) means **MIC1 is omitted** for most nodes.
- The **piezo hydrophone (MIC2) is optional** and deployed on roughly half the nodes (5 of 10) for direct comparison against acoustic-only nodes.
- Volume-pricing at qty 10 reduces some items by 10–15% vs the per-unit retail prices above.
- The figure does not include the LoRaWAN gateway, which is amortised across all nodes.

---

## 2. Shared / non-per-node items

### 2.1 LoRaWAN gateway

| Ref | Part | Function | Qty | Unit (SGD) | Notes |
|---|---|---|---|---|---|
| GW1 | RAK7268V2 indoor LoRaWAN gateway (8-channel) | Aggregates uplinks from all nodes | 1 | 580.00 | Or RAK7240 outdoor variant; AS923 plan supported |
| GW2 | Outdoor 5 dBi LoRa antenna + LMR-200 coax | Better gateway gain | 1 | 50.00 | Mounted on rooftop |
| GW3 | PoE injector (gateway model dependent) | Power | 1 | 20.00 | If using PoE variant |

### 2.2 Lab + test equipment

| Item | Cost (SGD) | Notes |
|---|---|---|
| Calibrated reference microphone (loan from NTU EEE) | 0 | Available through NTU labs |
| Audio interface — Focusrite Scarlett Solo or equivalent | 220 | For collecting reference recordings |
| Multimeter — Fluke 117 (already owned) | 0 | |
| µCurrent Gold for deep-sleep characterisation | 180 | Validates the 50 μA deep-sleep claim |
| INA219 power-monitor breakout (×3) | 25 | Continuous current logging during dev |
| Bench LiPo charger + protection tester | 80 | Battery cycle testing |
| Field recorder for SG mosquito audio collection | 150 | Tascam DR-05X or similar |
| Mosquito reference recordings + lab access | 200 | Coordinated with NTU SBS / Biology lab if possible |

Total lab/test: **~SGD 855**, rounded to 800 in the grant budget (some items already available).

### 2.3 PCB iteration

Two PCB revisions × 30 boards each at JLCPCB with basic assembly: ~SGD 1,100 across project. First rev for prototype; second rev for production deployment after lessons learned.

### 2.4 Cloud + tooling

| Item | Annual (SGD) |
|---|---|
| TimescaleDB on small cloud instance (e.g., DigitalOcean 4GB droplet) | 400 |
| Domain + DNS + Let's Encrypt | 50 |
| Edge Impulse Free tier (sufficient for project scale) | 0 |
| GitHub (free for open-source repos) | 0 |
| Backup S3-equivalent storage | 150 |
| Monitoring (Grafana on same droplet) | 0 |
| Buffer | 100 |
| **Total cloud/tooling** | **~700** |

### 2.5 Consumables + spares

| Item | Cost (SGD) |
|---|---|
| 10% spare ratio across all per-node components | 250 |
| Consumable: solder, flux, heat-shrink, zip-ties, sealant | 100 |
| Misc field-deployment items (mounting hardware site-specific) | 200 |
| Extra LiPo batteries for testing | 150 |
| **Total consumables/spares** | **700** |

---

## 3. Mapping to grant budget categories

The grant Q9 budget breakdown maps as follows:

| Grant category | Items | SGD |
|---|---|---|
| Sensor nodes ×10 | All per-node items × 10 (with volume discount) | 2,800 |
| LoRaWAN gateway | GW1 + GW2 + GW3 + install hardware | 650 |
| Enclosures + mounts | Bulk enclosure + bracket + 3D-print iteration (counted separately from per-node) | 900 |
| PCB fabrication | 2 design rev × 30 boards × JLCPCB+assembly | 1,100 |
| Lab + test equipment | Items in §2.2 | 800 |
| Cloud + tooling | Items in §2.4 | 700 |
| Competition + travel | Per `/docs/competition-brief.md` §8 | 1,400 |
| Consumables + spares | Items in §2.5 | 700 |
| Contingency | Component price fluctuation, tariff buffer, FX | 950 |
| **Total** | | **10,000** |

The budget intentionally weights hardware + field deployment over abstract "research & development", because the design and research work has already been completed pre-grant and lives in this repository. The grant is a *build-and-deploy* grant.

---

## 4. Sourcing strategy

- **Primary sources:** Singapore-local distributors (Cytron, Element14 SG, ST Electronics) where available. Reduces lead time and import friction.
- **Secondary:** AliExpress / LCSC for low-cost components where Singapore stock is missing. Order in bulk, with 10–14 day shipping factored into the Phase 1 schedule.
- **Critical-path components** (ESP32-S3 modules, LiPo cells, solar panels) are ordered **all at once during Phase 1** to lock in pricing and avoid mid-project supply disruption.

---

## 5. Component decision notes

### Why Seeed XIAO ESP32-S3 Sense rather than a custom ESP32-S3 board?

Three reasons:

1. **Integrated INMP441.** The Sense variant ships with the microphone already routed to I2S pins, saving PCB area and routing complexity. For a solo-developer Y1 project, this is a meaningful time saving.
2. **Form factor.** At 21 × 17.5 mm, the XIAO is the smallest comfortable ESP32-S3 form factor available. Important for the compact IP66 enclosure.
3. **PSRAM access.** The Sense board ships with 8 MB PSRAM properly wired, sparing the project from a PCB layout error class (PSRAM signal integrity is unforgiving).

Trade-off accepted: the XIAO Sense is ~SGD 7 more than a bare ESP32-S3-WROOM module would be. For a 10-unit deployment, ~SGD 70 total — well within tolerance for the development time saved.

### Why BME280 over BME680 / SHT4x / DHT22?

- **BME280** is the right balance of cost, accuracy, power, and library maturity. The BME680 adds gas-resistance sensing (VOCs) which is *interesting* but not relevant to the breeding-favorability function. BME680 also draws more current. The SHT4x is more accurate for RH but more expensive and provides no pressure measurement. The DHT22 is cheap but slow, less accurate, and notoriously failure-prone in tropical conditions.

### Why capacitive water sensing over resistive / ultrasonic / float / optical?

- **Resistive moisture sensors** corrode rapidly in tropical outdoor conditions and have documented short field lifetimes.
- **Ultrasonic sensors** require an air gap above the water surface, which is incompatible with drain-mount placement.
- **Float sensors** are mechanical and have moving parts; lower MTBF.
- **Optical (IR reflectance)** is feasible but vulnerable to fouling.

A conformal-coated capacitive PCB-trace probe is the best of these options: no moving parts, no exposed metal, well-characterised drift behaviour mitigated by the firmware's adaptive baseline.

### Why LoRaWAN AS923 (not NB-IoT, LTE-M, or WiFi)?

- WiFi: unavailable in drain / void-deck / rooftop sites.
- NB-IoT / LTE-M: per-node modem costs (~SGD 30) + SIM costs (~SGD 5/mo). For 10 nodes over 12 months, that's an extra ~SGD 1,000 the project doesn't need to incur.
- LoRaWAN AS923: free spectrum, single shared gateway, multi-km range, suits low-data-rate alert traffic perfectly.
