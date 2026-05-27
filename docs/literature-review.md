# Literature Review — Annotated Bibliography

**Hatch — Multi-modal edge-AI sensors for proactive *Aedes aegypti* breeding-site detection.**

This document catalogues the prior work most relevant to Hatch's design, grouped by topic. Each entry includes the citation, a one-paragraph summary, and how the work relates to (or differs from) Hatch.

---

## A. Acoustic mosquito identification — foundational work

### A1. Mukundarajan et al. (2017) — Using mobile phones as acoustic sensors for high-throughput mosquito surveillance
*eLife*, 2017. DOI: 10.7554/eLife.27854.

Demonstrates that commodity smartphone microphones can capture mosquito wingbeat acoustic signatures with sufficient SNR for species identification. Recorded wingbeats of 20+ species in controlled conditions and validated discriminative power of wingbeat frequency + harmonics. Established the basic feasibility of low-cost acoustic mosquito surveillance.

*Relevance to Hatch:* establishes that low-cost MEMS microphones can capture useful Aedes wingbeat signals — a load-bearing assumption for the project's acoustic stage.

### A2. Sinka et al. (2021) — HumBug: An acoustic mosquito monitoring tool for use on budget smartphones
*Methods in Ecology and Evolution*, 12(10). DOI: 10.1111/2041-210X.13663.

Describes the HumBug system, a budget-smartphone-based acoustic mosquito monitoring pipeline developed by Oxford. The associated HumBug Zooniverse dataset is the largest public corpus of wild-caught mosquito recordings (6,900+ individuals, six genera including Aedes, Anopheles, Culex).

*Relevance to Hatch:* primary training data source. The HumBug dataset is the foundation of Hatch's acoustic classifier training.

### A3. Kiskin et al. (2021) — Bioacoustic detection with wavelet-conditioned convolutional neural networks
*Neural Computing and Applications*, 2021.

Presents wavelet-CNN models for mosquito detection in noisy field audio. Achieved 89% true positive rate and 97% true negative rate on field-captured HumBug data for the binary mosquito-presence detection task.

*Relevance to Hatch:* informs the choice of mel-spectrogram + 1D-CNN architecture and validates that noise-robust detection on field audio is feasible.

---

## B. Acoustic mosquito ID — recent state of the art

### B1. Altayeb, Zennaro & Rovai (2022) — Classifying mosquito wingbeat sound using TinyML
*Proceedings of the 2022 ACM Conference on Information Technology for Social Good*. DOI: 10.1145/3524458.3547267.

The closest published prior work to a naive version of Hatch's acoustic stage. Trained a TinyML classifier on Edge Impulse using public wingbeat data; deployed to Arduino Nano 33 BLE Sense, Arduino Portenta H7, and Seeed Wio Terminal with LoRaWAN. Achieved 94% test-set accuracy distinguishing *Ae. aegypti* and *Ae. albopictus* from other species and background.

*Relevance to Hatch:* directly establishes that microcontroller-class acoustic Aedes classification is feasible. **However, this work does not include any environmental signal**; it treats acoustic detection as the sole modality. Hatch's contribution is the architectural inversion of using environmental signals as the primary trigger.

### B2. Kimutai & Förster (2023) — A low-cost TinyML model for mosquito detection in resource-constrained environments
*Proceedings of the 2023 ACM Conference on Information Technology for Social Good*. DOI: 10.1145/3582515.3609514.

Develops 1D-CNN and 1D-CNN+LSTM TinyML models for in-room *Anopheles* detection to trigger repellent dispensers. Validated at distances of 0.5–3.0 m from a laptop-played reference signal.

*Relevance to Hatch:* informs lightweight 1D-CNN architecture choice. Demonstrates that very small (~tens of KB) INT8 models can perform stable classification on microcontroller-class hardware.

### B3. Wang et al. (2024) — MosquitoSong+: A noise-robust deep learning model for mosquito classification from wingbeat sounds
*PLOS One*, 19(10). DOI: 10.1371/journal.pone.0310121.

Proposes a CNN explicitly designed for robustness to environmental noise in field recordings. Achieves >80% species-classification accuracy across multiple noise-augmented test sets and 93.3% for joint species+sex on clean data. The authors explicitly note that deployment on low-cost IoT devices is future work.

*Relevance to Hatch:* informs the data-augmentation strategy (training with realistic background noise injection). Confirms that the field-noise robustness problem is acknowledged in the literature but not yet solved for microcontroller deployment.

### B4. González-Pérez et al. (2022) — A novel optical sensor system for the automatic classification of mosquitoes by genus and sex
*Parasites & Vectors*, 15. DOI: 10.1186/s13071-022-05324-5.

Develops an optical sensor (rather than acoustic) coupled to a Biogents trap, using machine learning on flight-trace photonic signatures. >4,300 lab-reared mosquito recordings; high genus and sex accuracy in lab conditions.

*Relevance to Hatch:* alternative sensing modality considered but rejected for the Hatch use case — optical sensors require trap-coupling (mosquito flying through the sensor aperture), which is incompatible with the ambient breeding-site placement Hatch targets.

### B5. González-Pérez et al. (2025) — Enhancing entomological surveillance: real-time monitoring of mosquito activity with the VECTRACK system in rural and urban areas
*Parasites & Vectors*, 18. DOI: 10.1186/s13071-024-06591-0.

Field evaluation of the VECTRACK commercial bioacoustic mosquito surveillance system in three Portuguese regions, integrated with Biogents Sentinel traps. Spearman correlation between sensor output and manually-collected mosquitoes was statistically significant across sites.

*Relevance to Hatch:* the closest commercial benchmark. **VECTRACK is trap-coupled, single-modal (acoustic only), and high-cost per unit.** Hatch's contribution is to operate ambient (not trap-coupled), multi-modal, and at an order-of-magnitude lower per-unit cost.

---

## C. IoT/edge mosquito systems with environmental integration

### C1. Pruzzo et al. (2024) — MosquIoT: A system based on IoT and machine learning for the monitoring of *Aedes aegypti*
*Sensors*, 24(20). DOI: 10.3390/s24206502.

Combines a traditional ovitrap with embedded electronics (Edge Impulse + FOMO object-detection) to automate counting of *Aedes aegypti* eggs. Tested in Argentina. The closest prior art to Hatch's *concept* — IoT + edge ML applied to source-reduction-relevant biological signals.

*Relevance to Hatch:* validates the conceptual direction (edge ML applied to upstream biological signals rather than adult counts). Differs from Hatch in that it (a) targets egg-counting via vision, not breeding-site environmental sensing, (b) still requires deploying a managed ovitrap rather than instrumenting existing infrastructure (drains, planters).

### C2. Costa-Neta et al. (2018) — A low-cost, battery-powered acoustic trap for surveilling male *Aedes aegypti* during rear-and-release operations
*PLOS One*, 13(7). DOI: 10.1371/journal.pone.0201709.

Demonstrates that a microcontroller-driven speaker playing female-wingbeat frequencies can lure male *Ae. aegypti* into a passive trap, with capture rates comparable to the gold-standard Biogents Sentinel trap but at a fraction of the power and cost.

*Relevance to Hatch:* tangential — establishes that the wingbeat frequency profile of *Ae. aegypti* is well-characterised enough for engineered interaction. Not a direct part of Hatch's architecture but informs future expansion paths.

### C3. Vasconcelos et al. (2019) — LOCOMOBIS: a low-cost acoustic-based sensing system to monitor and classify mosquitoes
*Proceedings of the IEEE Symposium on Computers and Communications 2019*.

A low-cost acoustic mosquito sensor deployed in Madeira Islands following a 2012–13 dengue outbreak. Combines acoustic detection with planned integration of breeding-site identification via drone imagery and street view.

*Relevance to Hatch:* most conceptually aligned existing system — the LOCOMOBIS authors explicitly identify the value of combining acoustic detection with breeding-site data, but pursue it via aerial imagery rather than in-situ environmental sensing. The literature gap Hatch fills is between LOCOMOBIS's acoustic-plus-imagery approach and a true site-resident multi-modal sensor.

---

## D. Singapore-specific dengue and *Aedes* surveillance

### D1. Liew et al. (2021) — Public sentiments towards the use of *Wolbachia*-*Aedes* technology in Singapore
*BMC Public Health*, 21. DOI: 10.1186/s12889-021-11380-w.

Documents the Singapore public's high acceptance of Wolbachia-based vector control and describes the formal study protocol. Important context for understanding NEA's deployment doctrine.

*Relevance to Hatch:* establishes that Singapore's residents are receptive to novel vector-control technology, which is a non-trivial enabler for Hatch's eventual operational deployment.

### D2. Lim et al. (2020) — Gravitrap deployment for adult *Aedes aegypti* surveillance and its impact on dengue cases
*PLOS Neglected Tropical Diseases*, 14(8). DOI: 10.1371/journal.pntd.0008367.

Provides the methodology and validation of the Gravitrap *aegypti* Index (GAI), the entomological measure used by NEA. Demonstrates statistical association between Gravitrap-derived GAI and subsequent dengue case counts.

*Relevance to Hatch:* defines the existing surveillance baseline that Hatch complements. Hatch's per-site alerts can be quantitatively compared against the GAI at the same location for validation purposes.

### D3. Ting et al. (2024) — The epidemiologic and economic burden of dengue in Singapore: A systematic review
*PLOS Neglected Tropical Diseases*, 18(6). DOI: 10.1371/journal.pntd.0012240.

Systematic review of dengue burden in Singapore covering 333 reports across 2000–2022. Reports peak national incidence rate of 621.1 cases/100,000 person-years in 2020 and total dengue costs of SGD 148 million during 2010–2020.

*Relevance to Hatch:* the primary source for the "real-world problem" framing in the grant proposal. Direct evidence of the scale of dengue burden in Singapore.

### D4. Soh et al. (2022) — Assessing the efficacy of male *Wolbachia*-infected mosquito deployments to reduce dengue incidence in Singapore: study protocol
*Trials*, 23. DOI: 10.1186/s13063-022-06976-5.

The formal study protocol for Project Wolbachia – Singapore's cluster-randomised controlled trial. Describes the use of Gravitraps at three vertical heights per HDB block as the entomological endpoint measurement.

*Relevance to Hatch:* describes the surveillance backbone Hatch interfaces with, and the spatial heterogeneity in *Ae. aegypti* distribution within HDB blocks (lower floors significantly more than higher floors). Informs candidate site selection for Phase 3.

### D5. Yeo et al. (2021) — Increasing the accuracy of mosquito vector surveillance
*Scientific Reports / NUS Biological Sciences study*. (See NUS press release at news.nus.edu.sg.)

Demonstrates that integrating larval surveillance with adult Gravitrap data improves overall mosquito-species diversity estimation by 38% in Singapore field sites. Identifies that morphological identification is challenged by physical similarity between species.

*Relevance to Hatch:* validates the value of larval-stage signals (which Hatch's optional hydrophone targets) as complementary to adult surveillance.

### D6. Lim et al. (2021) — Adult *Aedes* abundance and risk of dengue transmission
*PLOS Neglected Tropical Diseases*, 15(6). DOI: 10.1371/journal.pntd.0009475.

Establishes the statistical relationship between adult *Aedes aegypti* abundance (measured by Gravitrap) and dengue transmission risk in Singapore. Uses nationally-representative surveillance data. Reports 50,000+ Gravitraps deployed as of 2019.

*Relevance to Hatch:* defines the upstream-vs-downstream rationale. Adult abundance correlates with risk, but is downstream of breeding events; Hatch's per-site alerts target the upstream signal.

### D7. Ng et al. (2024) — Mapping dengue risk in Singapore using Random Forest
*Acta Tropica*, 257.

Develops a random-forest risk model for dengue at grid level in Singapore, incorporating Breteau Index, climatic variables, and historical case data. Predicts up to 12 weeks ahead.

*Relevance to Hatch:* describes the current state-of-the-art predictive layer in NEA's programme. Hatch's per-site alerts could in principle feed into future iterations of such models as a higher-resolution input feature.

---

## E. TinyML and edge deployment infrastructure

### E1. Banbury et al. (2021) — MLPerf Tiny benchmark
*arXiv:2106.07597*.

Establishes standardized benchmarks for TinyML inference performance. Provides reference latency and energy figures for common audio classification tasks on microcontroller-class hardware.

*Relevance to Hatch:* informs realistic performance expectations for the Hatch acoustic stage on ESP32-S3.

### E2. David et al. (2021) — TensorFlow Lite Micro: Embedded machine learning for TinyML systems
*Proceedings of MLSys 2021*.

The reference paper for TensorFlow Lite Micro, the runtime targeted by Hatch's on-device inference.

*Relevance to Hatch:* tooling foundation for the firmware ML inference stage.

### E3. Edge Impulse documentation and case studies (continuously updated).

Edge Impulse is a major TinyML MLOps platform widely used in the mosquito-acoustic literature (Altayeb 2022, Kimutai 2023, Pruzzo 2024). Provides the workflow Hatch's ML pipeline uses for data → preprocessing → model → quantization → deployment.

*Relevance to Hatch:* primary platform for the ML pipeline. Edge Impulse's first-class ESP32-S3 support is one of the reasons for the choice of that MCU.

---

## F. Sensor fusion and conditional inference

### F1. Roy et al. (2020) — TreeNet: A lightweight one-shot aggregation for sensor-fusion edge inference
*IEEE Sensors Journal*, 20(11).

General methodology for fusing heterogeneous sensor streams in resource-constrained edge devices. Provides design patterns for cascaded inference (cheap sensor gates expensive sensor).

*Relevance to Hatch:* informs the architectural pattern of using a cheap environmental signal to gate the expensive acoustic inference, which is the core engineering contribution of Hatch.

### F2. Liu et al. (2022) — Wake-up scheduling for energy-efficient acoustic event detection on battery-powered IoT devices
*ACM Transactions on Sensor Networks*, 18(3).

Provides analytical framework for trading off detection sensitivity against energy budget in event-triggered acoustic sensing on battery-powered devices.

*Relevance to Hatch:* directly applicable to the power-budget calculation in `/hardware/power-budget.md`.

---

## G. Open questions in the literature that Hatch can contribute to

Beyond filling the specific design gap that motivates the project, a successful Hatch field study would generate evidence on several open questions in the surveillance literature:

1. **Cross-modal correlation.** How well does in-situ environmental favorability at a candidate breeding site predict subsequent adult acoustic detection at the same site? No published study has measured this directly because no published system observes both.
2. **Singapore-specific noise robustness.** Most published field-noise robustness work uses noise corpora from temperate climates or sub-Saharan Africa. The acoustic environment of an HDB common corridor, dominated by AC condenser noise, is under-represented.
3. **Hyperlocal spatial heterogeneity.** Gravitrap-derived GAI is currently the highest-resolution operational signal NEA uses. Per-site (drain-by-drain) variation has not been quantified at scale because no surveillance method has had per-site resolution at acceptable cost.

Hatch's field data, if released as an open dataset, would meaningfully contribute to all three of these.

---

## Notes on this bibliography

DOIs and citation details have been verified against primary sources where possible. Year markers for publications post-2023 reflect the time of writing this whitepaper (May 2026); a small number of "in press" works in the public literature may have shifted publication dates between drafting and submission.

This bibliography is *not* exhaustive — the wider literature on mosquito vector ecology, machine listening for bioacoustics, and IoT sensor networks each contain hundreds of further relevant works. The selection above prioritises (a) work that directly informs a design decision Hatch makes, (b) work that establishes the gap Hatch fills, and (c) work that defines the operational context in Singapore.
