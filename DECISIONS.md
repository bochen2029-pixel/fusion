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
