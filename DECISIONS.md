# fusion — decisions (append-only; supersede, never edit)

- **D-001 · 2026-08-09 · The four-tier stack + two seams.** Spine (deterministic 100 µs) /
  cerebellum (control nets, non-linguistic) / cortex (resident LLM triple, warpbus) /
  depths (optional turn-based). Upward seam = innovation-gated tokenization; downward seam
  = writs-never-actuators through governor floors. The Deadline Law at both seams.
- **D-002 · 2026-08-09 · The booster constitution is law** (spec §2.2): integrator-only
  state, no assist, one dynamics source, Philox determinism + memcmp oracle, headless MC
  with Wilson CIs as the gate. Solver-then-distill (LODESTAR) is the training doctrine.
- **D-003 · 2026-08-09 · Multi-rate plant (QC).** dt = 100 µs compile-time; full
  Grad-Shafranov re-solve at 100–200 Hz; linearized rigid-displacement vertical model at
  10 kHz between solves, re-derived per equilibrium. Full GS at 10 kHz rejected as
  infeasible-and-unrealistic.
- **D-004 · 2026-08-09 · ASR is a seam.** `voice/asr.h` interface; primary backend =
  NVIDIA Nemotron 3.5 ASR streaming 0.6B (160 ms chunks; native punctuation feeds the
  segmenter; domain fine-tune path for tokamak vocabulary); fallback = the proven
  auricle sherpa-onnx zipformer + Parakeet truth lane. Export path unverified → the
  fallback is load-bearing until measured.
- **D-005 · 2026-08-09 · Barge-in triggers on VAD, not ASR (QC).** ~20–30 ms detect →
  playback soft-pause ≤ 150 ms end-to-end; the ASR partial attributes afterward. (First
  ASR partial at 160 ms chunks cannot meet the budget.)
- **D-006 · 2026-08-09 · The innovation source is a named organ (QC).** A lightweight
  EKF / one-step forward model runs beside the policy at all times; MPPI rollout error
  covers only planner-active moments. Without this the upward seam has no signal.
- **D-007 · 2026-08-09 · Speech splits by standing (QC).** Solicited speech (answers)
  ships at M4; unsolicited initiative (interjections/warnings) stays shadow → brief until
  F-INSTINCT passes on this domain's held-out set. Rationale: auricle M4.5 measured 593
  emits/hr at dial zero — out loud that is a fire alarm.
- **D-008 · 2026-08-09 · Honest VRAM budget (QC).** Desktop worst case ~4.6 GB (auricle
  receipts); lean-desktop prescribed for play mode; ASR-on-CPU fallback recovers ~1 GB;
  training mode is headless and LLM-free.
- **D-009 · 2026-08-09 · The ghost guarantee is before-commit** (mid-sentence typical,
  once the parameter lands in the partial). Ghost = `fusor_fork` + tier-0/1 lookahead
  ≥ 20× RT; fork-vs-reality memcmp is the F-GHOST falsifier.
- **D-010 · 2026-08-09 · Repo staged pre-build.** Spec v0.1 + QC pass is canon; the
  multi-session method (SESSION_LOG as state of truth, same-commit discipline, KICKOFF per
  milestone, append-only decisions) is binding from session 1.

*The following arise from the five-auditor swarm QC (146 findings / 39 blockers;
`docs/qc/QC_CONSOLIDATED.md` + five lane reports + the `fusion-qc` Intercom replay).*

- **D-011 · 2026-08-09 · Two fork semantics from one primitive (the clairvoyance trap).**
  Ghost-forks SHARE the plant's Philox counter stream by design — that sharing *is*
  F-GHOST. Planner/teacher rollouts and the batched training envs MUST decorrelate their
  disturbance streams (counter-domain offsets). The training envs are the **fourth
  consumer** of the one-dynamics-source law (supersedes D-002's three-consumer list).
- **D-012 · 2026-08-09 · The policy runs on CPU at fixed precision.** A fp16/int8 CUDA
  policy feeding the integrator is mutually exclusive with the memcmp oracle, and the
  parent's proven path is a ~10 µs C net on CPU. <50 µs stands as a CPU budget. Also
  removes a CUDA consumer from the 100 µs tick (with ASR-on-CPU per D-004/V-04).
- **D-013 · 2026-08-09 · Commit = governor-accept, tick-stamped** — the repo's single
  definition (three QC lanes converged). Numeric setpoint writs from voice run at **H2
  with a spine-side objection window (default 3 s) + readback**; the ghost's
  before-commit guarantee is engineered against that window, not raced against ASR
  finals. Writ grammar gains id/supersedes/ttl/cancel + `formed_tick` staleness gate in
  v0.2 (supersedes the bare grammar in D-001/writs.toml v0).
- **D-014 · 2026-08-09 · Headset required in v1** (auricle's recorded precedent); AEC is
  a v2 node. TTS output is lane-stamped and never enters `opv` (self-echo suppressed at
  the lane, not the microphone). Bluetooth audio is disqualified from the barge-in
  budget.
- **D-015 · 2026-08-09 · Measured-rate and deadline corrections.** The dial-zero
  over-fire citation is **921 emits/hr** (full-day run C; 593/hr was a partial-day
  denominator — supersedes the figure in D-007's rationale, not its ruling). No estate
  receipt reports a p95 → the cortex budget is restated per TinyVillage L2 as a **fixed
  2 s budget + abort + drop-event**, and writs carry `formed_tick` (the effective_tick
  half restored).
- **D-016 · 2026-08-09 · The machine must be pinned before any physics code.**
  `contracts/machine.toml` (R, a, B_T, I_p, κ, δ, wall/vessel L-R, coil set) is v0.2's
  first deliverable; M0's gate re-derives from it. γ⁻¹ corrected to the wall-set
  10–100 ms class (passive conductor model now mandatory in tier-1); §0's
  necessity-of-a-net claim reframed onto F-NULL-C (supersedes the γ and claim language
  in the v0.1 text pre-QC).
- **D-017 · 2026-08-09 · v0.2 revision session mandated before M0.** The five QC reports
  are the line-item worklist; `KICKOFF_SPEC_V02.md` is the bootstrap; M0 is suspended
  until the spec bumps to v0.2 with every BLOCKER dispositioned (fixed, redesigned, or
  explicitly re-registered as a risk). KICKOFF_M0 will be rewritten at that point
  (REPO's three cold-session blockers land there: TOML parser choice + third_party plan,
  the scenario schema template, the exact build-command block).
