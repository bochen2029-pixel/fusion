# FUSION — the reactor you can talk to

A tokamak simulation with real (tiered, honest) physics, flown at kHz by a trained neural
controller that must actually hold the plasma or it disrupts — watched, remembered, and
**voiced** by a resident LLM triple on the warpbus, with full-duplex speech: talk to the
reactor through a mic; it talks back, interrupts a dangerous command *mid-sentence*, and
shows you the ghost of the disruption it just simulated to prove it.

**Read first: [`FUSION_ARCHITECTURE_v0.2.md`](FUSION_ARCHITECTURE_v0.2.md)** — the canon
(the QC revision; v0.1 is preserved beside it, frozen): the three-tier stack (spine /
cerebellum / cortex), the two seams (innovation-gated tokenization up;
writs-never-actuators down), the pinned synthetic machine (`contracts/machine.toml`,
FUSOR-1), the physics ladder + disruption model, the M2 control ladder, the voice loop
(sherpa/Nemotron unified on CPU; abortable clause-grain TTS with barge-in), hard
VRAM/CPU/latency budgets, **nine pre-registered falsifiers** (including F-VERACITY and the
three-arm F-PRESENCE-F), and milestones M0–M5 with the M0.5 thesis slice.

Core binary: `fusor`. Status: **spec v0.2, pre-build — M0 unsuspended.** Audits:
`docs/qc/` (five-lane swarm + fresh-eyes second instrument + `DISPOSITIONS.md`). Parents:
the Booster Lander Simulator (the sim constitution), auricle/SYNCYTIUM (the resident
membrane, measured), TinyVillage (the Deadline Law and tokenized experience).
