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

*The following arise from the fresh-eyes second-instrument pass (`docs/qc/FRESH_EYES.md`
+ its Addendum §5, 2026-08-09) and the founding session's recorded response, adjudicated
cross-session with the operator ratifying.*

- **D-018 · 2026-08-09 · The claim gets its instruments (fresh-eyes mandate).** (a) The
  anti-turn claim is NARROWED, precisely: on pre-tokenized lanes the tokenizer is itself
  a threshold-waker, so residency's plant-side value is hot-memory integration across
  events, not wake timing; the full anti-turn claim lives where the stream is raw (human
  lanes, partials) — "the anti-turn advantage lives exactly where the world hasn't
  already been compressed for you." F-PRESENCE-F therefore runs THREE arms — resident /
  event-triggered turn-based twin / polling twin — each arm's information set
  pre-registered; what only residency buys once events are granted to the twin is the
  claim under test. (b) **F-VERACITY** is a new pre-registered falsifier: planted factual
  questions + a citation-fidelity audit graded BY CODE against the tape (L5-clean),
  gating M4's solicited ship — a Speaker that fabricates rev citations must be able to
  lose. (c) The ghost's demo story is **"causal via the sovereign"**: its value separates
  from the governor only on a pre-registered floor-legal-but-lethal scenario class, which
  F-PRESENCE-F's planted errors must draw from — the receipt shows the governor alone
  would not have saved it. (d) Citation rule: derived figures carry their arithmetic
  (the emit rate is "2,823 / 3h03m51s = 921/hr, run C"), inherited figures their receipt
  path. Standing QC pattern, banked: **swarm-then-stranger, freeze-before-the-seal** —
  the swarm audits the machine, the stranger audits the claim.
- **D-019 · 2026-08-09 · The thesis slice comes first (sequencing inversion).** New
  milestone **M0.5**: tier-0 plant + tokenizer + floors + the text-only triple — no GS,
  no voice — fusion's shadow soak, so the three-arm harness and the tape-grader exist and
  dry-run on scripted events by ~session 10 instead of ~session 25. The novel claim meets
  evidence early; the physics ladder serves the bet, not the reverse. Scope pinned in
  v0.2; M3 rescopes onto the same rig (tier-1 fidelity + live ghosts).
- **D-020 · 2026-08-09 · v1 scope rulings.** MPC null: downgraded — PID+LQ is THE null; a
  textbook MPC becomes a later, separately-receipted comparison (CTL-28). MCTS +
  sysid-by-backprop: out of §3, parked post-M5 with named nulls or deleted (CTL-30/31).
  Depths tier: cut from the v1 stack — returns only with a consumer, a budget row, and a
  gate (supersedes D-001's four-tier enumeration for v1; the stack ships as three tiers).
  Every spec range collapses to its cheap end for v1. **Tier-2 MHD: reserved to the
  operator** — recommended cut for v1, sanctioned compromise = renderer raymarches
  tier-1-derived emissivity + artistic noise; the ruling lands as its own D-entry in
  v0.2 either way.
