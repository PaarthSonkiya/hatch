# Risk Register

**Hatch — Itemised technical, operational, and programmatic risks with mitigations.**

This register catalogues risks identified during pre-grant design work, scored by likelihood (L: Low / M: Medium / H: High) and impact (L / M / H), with explicit mitigation strategies. Risks are reviewed at every phase gate.

---

## 1. Technical risks

### R1.1 — ML classifier accuracy below 75% in Singapore field conditions

| Field | Value |
|---|---|
| Likelihood | M |
| Impact | M |
| Score | 4/9 |

**Description.** The acoustic classifier is trained predominantly on the HumBug corpus, which over-represents African and Southeast Asian field conditions outside Singapore. Background noise profiles in HDB common areas — dominated by AC condenser noise, traffic, and human voices — may degrade performance below the lab-trained baseline.

**Mitigation.**
1. Architecture chosen explicitly with PCEN normalization and noise-augmentation training to maximise transferability.
2. Phase 3 includes a Singapore-specific data collection campaign; the classifier is fine-tuned on local data before being baselined.
3. The system's primary signal is environmental, not acoustic. Even if the ML stage performs poorly, the environmental-only signal provides standalone value as a stagnant-water detector.
4. The classifier is intentionally a 4-class softmax (Aedes-aegypti, Aedes-albopictus, other-insect, noise). The actionable alert only depends on the union of the two Aedes classes, which makes the operationally relevant accuracy more forgiving than a fine-grained species ID.

### R1.2 — Capacitive water probe fouling or drift

| Field | Value |
|---|---|
| Likelihood | M |
| Impact | L |
| Score | 2/9 |

**Description.** Outdoor capacitive sensors are subject to drift from biological fouling, mineral deposition, and substrate degradation. Resistive moisture sensors are notorious for very short field lifetimes in tropical conditions; capacitive sensors are more robust but not immune.

**Mitigation.**
1. Capacitive probe is implemented as conformal-coated PCB traces, not exposed metal. The conformal coating (acrylic or silicone) is renewed per node during quarterly maintenance.
2. Firmware performs a self-test on each wake (measures probe capacitance against a known reference) and flags drift exceeding 20% for maintenance attention.
3. The water-presence binary signal is derived by thresholding against a recent baseline rather than an absolute value, providing some self-calibration against slow drift.
4. Each node carries a redundant secondary indicator: prolonged absence of any capacitance change despite confirmed rainfall events (cross-referenced against a public weather API) suggests probe failure.

### R1.3 — Enclosure water ingress in tropical conditions

| Field | Value |
|---|---|
| Likelihood | M |
| Impact | H |
| Score | 6/9 |

**Description.** Singapore experiences intense monsoon rainfall and high humidity. Water ingress into the node's main compartment would cause electronic failure. This is the highest-impact technical risk because it can cause silent failure that's only detected by absence of data.

**Mitigation.**
1. IP66-rated ABS enclosure with rubber-sealed cover, IP68 cable glands for sensor leads.
2. Internal silica desiccant pack with replacement at every quarterly visit.
3. Phase 2 includes accelerated thermal-humidity cycling test of three units before field deployment (24-hour cycles, 25°C/90%RH ↔ 35°C/95%RH, ten cycles minimum).
4. Each node sends a daily summary including humidity reading from the internal BME280. A sustained internal humidity above 80% triggers a "service required" flag in the dashboard.
5. Conformal coating on the main PCB provides a secondary defence even if the enclosure seal fails.

### R1.4 — LoRaWAN coverage gap at deployment sites

| Field | Value |
|---|---|
| Likelihood | L |
| Impact | M |
| Score | 2/9 |

**Description.** LoRaWAN range in dense urban environments is variable. A particular node site may be in a coverage shadow despite being geographically close to the gateway.

**Mitigation.**
1. Pre-survey of every candidate site before final selection using a portable LoRaWAN test transmitter operating at SF12 (worst-case sensitivity).
2. Gateway placement is optimised for line-of-sight coverage of the cluster; rooftop installation is preferred.
3. Nodes implement on-device buffering of up to 24 hours of data in flash; intermittent connectivity does not cause data loss.
4. Fallback path: a second LoRaWAN gateway can be added within the grant budget if Phase 2 surveys reveal coverage gaps.

### R1.5 — Power budget exceeded under sustained shading

| Field | Value |
|---|---|
| Likelihood | L |
| Impact | M |
| Score | 2/9 |

**Description.** Some candidate sites (deep drain channels, void deck interior locations) receive less direct sunlight than the 5W solar panel was sized for. Sustained shading could cause battery depletion.

**Mitigation.**
1. Node battery alone (2000 mAh) provides 30+ days of operation in zero-charging conditions, far exceeding any realistic shading scenario.
2. Each node reports battery state in daily summaries; sustained downward trend triggers a "service required" flag.
3. For sites with documented poor solar conditions, the firmware can be configured to reduce the environmental sampling rate from 5-minute to 15-minute intervals, extending battery life by ~3×.

### R1.6 — Component obsolescence or supply disruption

| Field | Value |
|---|---|
| Likelihood | L |
| Impact | L |
| Score | 1/9 |

**Description.** Specific components (e.g., ESP32-S3 modules) could become unavailable during the project lifetime.

**Mitigation.**
1. All chosen components are mainstream (ESP32-S3, BME280, INMP441, RFM95W). None are single-source from boutique vendors.
2. Sufficient stock of all critical components is purchased upfront (Phase 1) to cover the full 10-node deployment plus 10% spares.
3. Firmware is portable: the abstraction layer in `firmware/src/sensors.cpp` allows substitution of the BME280 with BME680 or SHT4x with minimal effort.

---

## 2. Operational risks

### R2.1 — Town council non-engagement

| Field | Value |
|---|---|
| Likelihood | M |
| Impact | M |
| Score | 4/9 |

**Description.** The Phase 3 HDB cluster (5 nodes) requires permission from the relevant town council. This is a relationship a Y1 student does not have pre-built.

**Mitigation.**
1. Engagement begins during Phase 2 (M04–M06), not Phase 3. Six months of lead time is realistic for forging a town-council relationship through formal channels.
2. NTU has existing relationships with multiple town councils via various community-engagement programmes. The applicant will request a warm introduction through the relevant NTU office.
3. The proposal value to a town council is concrete (real-time visibility into breeding sites in their estate, at zero cost to them), which is materially attractive.
4. **Fallback:** if no town council engages within Phase 2, Cluster B nodes are relocated to additional NTU sites. The technical scope is preserved; only the operational-relevance demonstration is reduced.

### R2.2 — Public concern about microphone-based sensors

| Field | Value |
|---|---|
| Likelihood | M |
| Impact | M |
| Score | 4/9 |

**Description.** Members of the public may be uncomfortable with microphone-bearing devices in residential common areas, even with appropriate technical and policy safeguards.

**Mitigation.**
1. Public signage on each node with a QR code linking to a plain-language explanation and FAQ.
2. Firmware-level guarantees: 80 Hz high-pass filter physically renders speech inaudible; microphone is gated off except during environmental-trigger windows; no continuous streaming is possible.
3. Public data policy clearly published.
4. A single point-of-contact email for queries, monitored by the applicant.
5. Decommissioning policy: if a specific node site generates resident complaints, the node is removed within 24 hours.

### R2.3 — Vandalism or theft of field hardware

| Field | Value |
|---|---|
| Likelihood | L |
| Impact | M |
| Score | 2/9 |

**Description.** Field-deployed electronics in public areas are subject to vandalism or theft. The Gravitrap network has documented occasional tampering.

**Mitigation.**
1. Enclosures are tamper-evident, mounted with fixtures requiring tools to remove.
2. The hardware value per unit (~SGD 100) is low enough that loss of one or two nodes does not materially threaten the project.
3. Spare units (10% ratio) are budgeted.
4. Each node's last known LoRa transmission location is logged; sudden silence on a node prompts physical inspection.

### R2.4 — Audio data contains incidental human speech

| Field | Value |
|---|---|
| Likelihood | L |
| Impact | M |
| Score | 2/9 |

**Description.** Despite the 80 Hz high-pass filter, some clips may inadvertently contain low-frequency speech artefacts (especially if a person is shouting near the node).

**Mitigation.**
1. The high-pass filter eliminates most speech-band content at the source.
2. Any clips uplinked to the cloud for fine-tuning are reviewed before being added to a release dataset.
3. The dataset release process explicitly filters for speech-like spectral content using a separate VAD-style classifier.
4. Retention policy purges raw uplinked clips after 30 days.

---

## 3. Programmatic risks

### R3.1 — Schedule slip (solo Y1 applicant)

| Field | Value |
|---|---|
| Likelihood | M |
| Impact | L |
| Score | 2/9 |

**Description.** The applicant is a Y1 student carrying a full course load plus a second major. Solo execution of a 4-phase 12-month project will encounter exam periods, semester transitions, and other concurrent demands.

**Mitigation.**
1. Each phase has a single deliverable gate, not a continuous flow of dependencies. Slippage in one phase does not cascade.
2. The most labour-intensive phases (Phase 2: hardware iteration; Phase 3: field study) are scheduled in periods that align with the academic calendar (winter break and summer break).
3. Pre-grant work has already completed the design and research stages. Grant-funded work is build-and-deploy, not design.
4. The applicant has not under-scoped: a single-modal acoustic-only version of the project would be ~50% the engineering work; Hatch's environmental sensing adds workload but also adds graceful-failure capability.

### R3.2 — Mentor / faculty advisor non-availability

| Field | Value |
|---|---|
| Likelihood | L |
| Impact | L |
| Score | 1/9 |

**Description.** The project plans to engage a faculty mentor for informal advice. Y1 student-faculty relationships are not pre-existing.

**Mitigation.**
1. A mentor is desired but not strictly required for project execution.
2. Outreach to potential mentors will begin in Phase 1 with a clear project brief (this repository).
3. Multiple departments could provide a mentor (EEE, CSE, SBS); failure of one path does not block the project.
4. NTU's Centre for Information Technology Services and various student-research support offices provide secondary advisory channels.

### R3.3 — Grant funds released later than expected

| Field | Value |
|---|---|
| Likelihood | L |
| Impact | L |
| Score | 1/9 |

**Description.** Administrative delays in disbursement could shift the start of Phase 1.

**Mitigation.**
1. Phase 1 work (firmware on existing development boards, ML pipeline) can be performed with personal-equipment and zero hardware purchase. Limited progress is possible even before funds clear.
2. The phased structure means the largest single purchase (10-node hardware run) is in Phase 2, not Phase 1.

### R3.4 — Competition non-submission

| Field | Value |
|---|---|
| Likelihood | L |
| Impact | L |
| Score | 1/9 |

**Description.** The grant explicitly supports competition participation. Failing to submit to any competition would undercut a key grant objective.

**Mitigation.**
1. Five competition targets are pre-identified (see [competition-brief.md](./competition-brief.md)) so missing one does not eliminate the option.
2. James Dyson Award has annual deadlines compatible with project timeline.
3. The repository structure (whitepaper, dashboard, working firmware) is designed to be directly reusable as competition submission material.

---

## 4. Residual risk

After all mitigations, the project's residual risk profile is **low-medium**. The two highest residual risks are:

- **Enclosure ingress (R1.3)** — the only "silent failure" mode that could meaningfully damage outcomes. Mitigations are aggressive but field validation in Phase 2 is the only true test.
- **Town council non-engagement (R2.1)** — the only risk that could materially constrain the demonstration scope, though the technical scope is preserved by the all-NTU fallback.

No risk in this register threatens the project's existence. The system is designed with graceful failure modes throughout, and the pre-grant work has eliminated all design risk: the grant funds execution of an already-designed system.

---

## 5. Risk register review cadence

This register is reviewed at:

- End of each phase (4 reviews during grant)
- Any time a risk's likelihood or impact is observed to change
- Any time a new risk is identified

Reviews are logged in the project changelog. Material changes to the register are escalated to the faculty mentor for advice.
