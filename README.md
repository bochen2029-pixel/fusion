# FUSION — the reactor you can talk to

A tokamak simulation with real (tiered, honest) physics, flown at kHz by a trained neural
controller that must actually hold the plasma or it disrupts — watched, remembered, and
**voiced** by a resident LLM triple on the warpbus, with full-duplex speech: talk to the
reactor through a mic; it talks back, interrupts a dangerous command *mid-sentence*, and
shows you the ghost of the disruption it just simulated to prove it.

**Read first: [`FUSION_ARCHITECTURE_v0.1.md`](FUSION_ARCHITECTURE_v0.1.md)** — the full
architecture/design spec: the four-tier stack (spine / cerebellum / cortex / depths), the
two seams (innovation-gated tokenization up; writs-never-actuators down), the physics
ladder, the control-net training doctrine, the voice loop (Nemotron 3.5 ASR streaming
option + proven sherpa fallback; abortable TTS with barge-in), hard VRAM/latency budgets
for the reference card, six pre-registered falsifiers, and milestones M0–M5.

Core binary: `fusor`. Status: **spec staked, pre-build.** Parents: the Booster Lander
Simulator (the sim constitution), auricle/SYNCYTIUM (the resident membrane, measured),
TinyVillage (the Deadline Law and tokenized experience).
