# Field Deployment Plan

**Hatch — Phase 3 pilot study, Singapore.**

This document describes the operational plan for the 10-node field study that constitutes Phase 3 of the Hatch grant proposal. It covers site selection, stakeholder engagement, installation logistics, data handling, and ethics.

---

## 1. Objective

The Phase 3 field study has three objectives, listed in priority order:

1. **Validate** that the multi-modal sensor approach reduces false-positive rates below the rates published for pure-acoustic systems in noisy outdoor conditions.
2. **Collect** a Singapore-specific labelled dataset of *Aedes* acoustic events under realistic HDB ambient conditions, for fine-tuning the production classifier.
3. **Demonstrate** the operational workflow: a node identifies a candidate breeding site, an alert reaches the dashboard, and a stakeholder responds.

Objective 3 is the success criterion that distinguishes a research demo from a deployable system.

---

## 2. Deployment topology

Ten field nodes plus one LoRaWAN gateway. The nodes are split into two clusters of five.

### Cluster A — NTU Campus (5 nodes)

Selection criteria: easy physical access for the applicant; existing stakeholder relationships through NTU's Office of Estates Management; controlled environment for diagnosing system failures; low expected dengue baseline (good for false-positive characterisation).

Candidate sites:

| Node | Location | Site type | Rationale |
|---|---|---|---|
| A1 | EEE block — drainage channel near loading dock | Open drain | High shade, often has standing water after rain |
| A2 | Hall of Residence 2 — common-area planter | Container | Plant pot saucer simulator; controlled scenario |
| A3 | The Hive (LT) — rooftop drain | Gutter | High-humidity microclimate |
| A4 | School of Biological Sciences — bin centre | Bin area | Documented Aedes-conducive environment |
| A5 | Garden by NIE — collected-water feature | Pond edge | Boundary case: water permanent, what is the alert behaviour? |

Gateway: rooftop installation on EEE block, providing line-of-sight to all five nodes plus the HDB cluster.

### Cluster B — Adjacent HDB Estate (5 nodes)

Selection criteria: real operational conditions; existing town-council stakeholder relationship needed; known historical Aedes activity from public NEA data.

Pre-selection process: identify HDB blocks within 1 km of the campus gateway with at least one historical dengue cluster in the past 24 months (from NEA's public cluster data). Approach the relevant town council with the proposal during Phase 2 of the project (M04–M06) so that the deployment can begin by M07.

Indicative sites (subject to town-council approval):

| Node | Location | Site type |
|---|---|---|
| B1 | HDB block common corridor — floor trap | Drain |
| B2 | HDB block bin chute base | Container |
| B3 | HDB rooftop — drainage trough | Gutter |
| B4 | Void deck — planter base | Container |
| B5 | Multi-storey carpark — floor trap | Drain |

The exact addresses are deliberately not listed in this public document; site-level identification will be coordinated through formal town-council channels and not published.

---

## 3. Stakeholder engagement

### NTU side

- **Office of Estates Management:** notify by Phase 2; obtain formal permission for sensor installation on campus property. Provide one-page system description and the data-handling policy.
- **Hall management:** notify the relevant hall masters for any node sited near a residence.
- **EEE Programme leadership:** notify the Associate Chair (Students) and proposed faculty mentor as a courtesy; obtain a soft letter of awareness if requested.
- **Sustainability and Smart Campus initiatives:** flag the project early — Hatch fits both portfolios and they may be willing to publicise it.

### External side

- **Town Council (TBD pending block selection):** formal letter introducing the project, the technology, the data-handling policy, and the proposed sites. Request a single contact officer for coordination. Offer real-time dashboard access to the council's vector-control team during the study.
- **NEA Environmental Health Institute:** courtesy notification, not a permission request. The project does not modify NEA's existing Gravitrap operations. A one-page summary plus a copy of the deployment plan will be sent for awareness during Phase 2.
- **Faculty mentor (TBD during Phase 1):** a single NTU faculty member to act as informal advisor. Candidate departments: EEE (for the embedded systems advisor), School of Computer Science & Engineering (for the ML advisor), School of Biological Sciences (for the entomology/dengue advisor). One mentor is sufficient; cross-school advice can be sought informally.

---

## 4. Installation and maintenance

### Installation procedure (per node)

1. Site survey (one visit, no install): photograph site, measure access dimensions, confirm LoRaWAN coverage with a portable test transmitter.
2. Permissioning sign-off from site owner.
3. Installation visit: mount enclosure with appropriate fixtures (cable tie, drain bracket, or wall anchor depending on site); commission node via captive WiFi config portal; verify LoRaWAN join and first uplink.
4. Posted signage indicating presence of sensors and contact for questions.

### Maintenance schedule

- Weekly remote health-check via dashboard (no site visit required).
- Monthly physical inspection per node (clean enclosure, inspect cable glands, sanity-check capacitive probe).
- Quarterly full audit (verify each node against ground-truth manual mosquito catch for that site).

The applicant is the sole operator for the duration of the grant. A maintenance run for 10 nodes split across two locations is approximately 4 hours per month, well within a student schedule.

---

## 5. Data handling and ethics

### Audio data

The microphone is gated off in firmware except during active acoustic capture, which occurs only when the environmental gate has triggered. Capture is bounded: 4-second buffers, captured at most once per minute during an active event. Continuous audio streaming is technically prevented by the firmware state machine.

Captured audio is processed on-device. Only the *features* (mel-spectrograms) and *labels* (classifier output) are uplinked by default. Raw audio is uplinked only on explicit operator request from the dashboard, and only for short windows around suspected detections, for fine-tuning purposes.

A high-pass filter at 80 Hz is applied before any storage or uplink, which suppresses the speech voice-band; the system is not capable of capturing intelligible human speech even when it does record.

Any audio data uplinked to the backend is retained for at most 30 days, after which it is purged. Datasets released to the research community will be filtered to exclude any clips containing identifiable speech artefacts.

### Location data

Node positions are recorded in the backend at install-time and never broadcast in alert packets (only an opaque node ID is sent). The public dashboard, if exposed externally, displays only aggregated block-level risk scores, never individual-node coordinates.

### Public posting

Each node site has a small printed label on the enclosure with a QR code linking to a public information page describing the project, the data policy, and a contact email for queries.

### Approvals checklist

- [ ] NTU Office of Estates Management permission letter (Phase 2)
- [ ] Town Council formal MOU (Phase 2)
- [ ] NTU IRB consultation (likely not required since no human-subjects research, but to be confirmed in Phase 1)
- [ ] Hall/Block management consent letters where applicable
- [ ] Signed faculty mentor advisor agreement
- [ ] NEA awareness notification (Phase 2)

---

## 6. Risk-aware deployment principles

The deployment plan is designed to fail gracefully:

- If the town council does not engage, Cluster B can be relocated to additional NTU sites without changing the technical scope.
- If a node is damaged or stolen, the system continues operating with N-1 nodes; spare units (10% ratio) are kept on hand.
- If the LoRaWAN gateway fails, nodes buffer up to 24 hours of summaries in flash and re-uplink on reconnection.
- If a stakeholder objects to a specific node after install, it is removed within 24 hours.

---

## 7. End-of-study handover

At the end of Phase 4, the deployment is decommissioned as follows:

- All nodes physically retrieved.
- Town council and NTU stakeholders provided with a closing report summarising the data collected, alerts generated, and outcomes.
- The Singapore-specific dataset is anonymised and released under CC-BY 4.0 alongside the technical paper.
- Hardware retained by the applicant for potential follow-on work.

A small farewell briefing for the town council, if they wish, presents the findings and any recommendations for operational adoption that may follow from the study.
