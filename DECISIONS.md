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

*The following are the v0.2 revision session's design rulings (S3; spec
`FUSION_ARCHITECTURE_v0.2.md`; full traceability in `docs/qc/DISPOSITIONS.md`).*

- **D-021 · 2026-08-09 · FUSOR-1 pinned** (`contracts/machine.toml`): synthetic compact
  high-field machine (R 1.85 m, a 0.57 m, κ_a 1.65, B₀ 12 T, I_p 8.5 MA, thick vessel
  τ_w 25 ms, 24 passive filaments, 12 actuated circuits). Both load-bearing numbers
  derived from the one config, arithmetic in-file: Q ≈ 2.3 at H98 = 1 (Q > 1 needs
  H98 ≳ 0.85 — passable, not trivial) and γ⁻¹ ≈ 25–40 ms wall-set (reported from the
  eigenproblem at runtime, never configured). Executes PHY-01/02/03 + FRESH_EYES D1.
- **D-022 · 2026-08-09 · One objective, one stochastic model, one seed law.**
  `contracts/objective.toml` (all three consumers: trainer reward, teacher cost, null's
  tuning cost; SPINE_SHUTDOWN a terminal failure class with cost ordering that defeats
  gate-farming — CTL-07/10) + `contracts/dispersions.toml` (observable/blind split,
  Philox stream domains per D-011, state-triggered disturbances carry no stream to leak)
  + code-enforced seed partition (train/eval/gate; trainer hard-refuses gate seeds;
  `seedcheck` ctest) + stats law (Wilson for rates, paired bootstrap for RMS,
  lexicographic decision rule). Pre-registered before any tuning exists (CTL-12's
  retroactivity trap avoided).
- **D-023 · 2026-08-09 · The EKF, fully specified** (CTL-11 + FRESH-4):
  model = the linearized vertical/circuit tier (one dynamics source, linearized), inputs =
  `contracts/diagnostics.toml` channels only; **information set pre-registered: never the
  disturbance schedule, never blind-dispersion truths, never scenario flags — its
  parameters carry the plant_scatter mismatch** (the epistemic handicap IS what makes
  innovation informative in a simulator); covariance carries across re-linearizations;
  **rolling NIS χ² is a pre-registered M1 acceptance gate** — events.toml σ-thresholds
  are provisional until it passes. Plumbing: policy consumes EKF state + innovation
  vector (CTL-21, hide-the-flags acceptance test); the tokenizer computes from the EKF
  residual only.
- **D-024 · 2026-08-09 · MPPI/CEM is OFFLINE-only, and the GS solve is pipelined by
  construction.** The planner is the teacher; it never runs in play mode (resolves
  CTL-06/V-12; §1/§7 rows follow; D-006's rationale restated onto the standing EKF). The
  200 Hz GS solve scheduled at tick T applies at tick T+50 — deterministic in sim time;
  if wall-late the sim WAITS (pacing dips, `gs_late` receipted; determinism is never
  traded); the linear model's ΔZ validity bound triggers an early re-solve. F-KEEPUP-F
  is defined on aggregate pacing (PHY-11).
- **D-025 · 2026-08-09 · The M2 ladder** (supersedes D-002's "solver-then-distill" as
  sole doctrine; the parent's honest wall respected): (a) θ/gain-schedule net over the
  CEM-tuned PID+LQ base — the parent's proven GREEN, **F-NULL-C satisfiable here**;
  (b) residual policy; (c) end-to-end distill (the parent's 0/16 null) or pure sim-RL
  MPO (TCV's actual pipeline; PPO only with KL-to-teacher). DAgger named for any distill
  rung. The null ships in-binary as a permanent arm (L11); MPC is a later separately-
  receipted comparison (D-020).
- **D-026 · 2026-08-09 · Ghost mode declared + F-GHOST rescoped** (supersedes D-009's
  fork-vs-reality memcmp phrasing): ghost = frozen-equilibrium tier-0 burn + linearized
  vertical/circuit dynamics on the plant's shared Philox stream. F-GHOST = (i)
  ghost-vs-ghost memcmp; (ii) bounded divergence vs the full plant (|ΔZ| < 2 cm,
  |ΔQ| < 0.5 at 500 ms), published either way; (iii) warning-precedes-governor-accept on
  the tape. Sentinel interjection on fork-verdict is a templated deterministic line with
  pre-warmed TTS (~0.4 s into the 3 s objection window) — the guarantee is structural
  (M-B5/V-03).
- **D-027 · 2026-08-09 · Arbitration + budgets.** GPU: one plant-owned CUDA context at
  greatest stream priority; the LLM is the preemptible party; ASR/TTS/policy never touch
  CUDA. CPU: §7.3 core map (spine spin isolated on a P-core; degradation order ends at
  "never the tick"). The 100 µs tick has a line-item budget (§7.4), p99.9 binding,
  measured with the LLM hot at the **M3** gate. Telemetry: 64³ @ 10 Hz shared-memory,
  42 MB/s, renderer may never backpressure.
- **D-028 · 2026-08-09 · Membrane mechanics batch.** Plant-lane boundaries are structural
  (M-B6); all three room guards are law (self-echo lane rule, per-seat caps, cascade
  breaker in `events.toml [cascade]`); Sentinel emit is fork-gated (M-M2); molt-on-calm
  with crisis shed-oldest fallback (M-M6); event vocabulary freezes at M3 (L3/M-M13);
  the tape is durable (auricle `durable.h` pattern; FRESH-8); the cortex host has a
  watchdog and `mind_died` is a tape event (FE-20); `tts.h` is a contract (clause-grain
  streaming, ≤ 60 ms queued-PCM cap, sample-accurate abort — V-09/10/15/16).
- **D-029 · 2026-08-09 · The falsifier registry v2** (nine, each with a named rig and a
  losing branch): F-BURN-0 and F-SHAPE-1 promoted (M0/M1 have falsifiers — P-13);
  F-KEEPUP-F on aggregate pacing; F-NULL-C v2 (matched wall-clock+VRAM, CRN, three rates);
  F-GHOST per D-026; F-VOICE with t₀/t₁ + device-named receipts + speakers-mode negative
  test; **F-VERACITY** (code-graded truth-of-speech, gates M4); F-PRESENCE-F three-arm
  with the operator as L12-calibrated grader; F-INSTINCT with its held-out set created
  and quarantined at M3. The three-arm result is publishable either way (D-018's
  narrowed claim makes "the event-triggered twin closes the gap" a finding, not a
  failure).
- **D-030 · 2026-08-09 · Spine gates + autonomy dial + writ grammar v1 shipped.**
  `contracts/spine_gates.toml` (deterministic predicates, dwell, hysteresis,
  false-positive budgets, SPINE_SHUTDOWN failure class, gates-always-armed — including
  during the M5 toggle demo); the autonomy dial A0<A1<A2<A3 in true risk order replaces
  the harm dial (CTL-22; effective = min(class, source, global)); `contracts/writs.toml`
  v1 (id/rev/supersedes/ttl/cancel/schedule, typed units, formed_tick + rev_observed
  staleness gates, acceptance receipts, the session input-tape — FRESH-6). v1 ships A0
  unsolicited / A2 voice numeric setpoints / A3 envelope-tightening only.
- **D-031 · 2026-08-09 · Operator rulings: tier-2 cut; MIT; public.** (a) **Tier-2 MHD
  is CUT for v1** per the standing recommendation D-020 reserved: the renderer ships
  tier-1-derived emissivity + artistic noise; full 3-D MHD is a post-M5 pure-renderer
  revisit and, if ever revived, feeds the renderer never the controller. (b) **LICENSE =
  MIT** (root LICENSE, Access Intellect LLC; FE-25 closed). (c) **The repo is public on
  GitHub** (`github.com/bochen2029-pixel/fusion`); `runs/` stays local-only by design —
  receipts are snapshot-preserved and summarized in SESSION_LOG. All open operator
  questions from the v0.2 ledger are now closed.
- **D-032 · 2026-08-09 · M0 implementation rulings (the code exposed them; receipted in
  `runs/m0-burn-2026-08-09.md`).** (a) **Radiation-honest reference point:** machine.toml's
  Q ≈ 2.3 was a radiation-free bound; with the PHY-09 convention pinned (IPB98 τ_E covers
  CONDUCTION, closed-form P_cond^0.31 = W/(C·H98); radiation is a separate channel) and
  the baseline channels on, the reference is **Q ≈ 1.9–2.2 at H98 = 1.0, Q > 1 needs
  H98 ≳ 0.88** — machine.toml comment restated, physics_sanity ctest asserts the band.
  (b) `impurity_seed_frac` 0.001 → **1e-4** (0.1% Ar radiates ~8 MW and drags Q to ~1.6).
  (c) machine.toml gains a **[heating] block** (P_aux_max 60 MW, τ_act 0.2 s, slew
  40 MW/s, gas caps) — the puff response needs ~55 MW transiently. (d) **Scenario-schema
  additions:** a `[control]` block (setpoints + reporting reference) and per-event
  `{mag, t_lo, t_hi}` overrides; `_TEMPLATE` gains them at next touch. (e) Small pinned
  constants: Ar effective charge 16 with a +0.5 Z_eff pedestal (calibrated to machine
  Zeff at reference); scaling-grade Ar L_z 8-point log-log table (Mavrin-2018-flavored)
  and a Trubnikov-shaped synchrotron fit calibrated to ~2 MW at reference — both labeled
  scaling-grade in code; Q reporting floor P_aux ≥ 0.5 MW; alpha lag τ_s = 0.053·T^1.5/n₂₀.
  (f) **Gate-attempt protocol:** easy.toml frozen before any tuning; CEM tunes on
  train-domain seeds only (guard + seedcheck); the gate seeds run ONCE per attempt and
  every attempt is receipted with its count. (g) Philox draw layout (blocks 0–4) is
  pinned in core/sim.cpp; layout changes = a D-entry. (h) **The objective prices the
  gate:** the first CEM run minimized q_rms and drove the pass rate to 0.00 — a measured
  misaligned-objective result, receipted; `objective.toml` gains a `[gate]` block
  (miss_cost + minQ-shortfall weight) so the tuner optimizes what F-BURN-0 measures.
  Gate seeds remained untouched throughout.
- **D-033 · 2026-08-09 · The M0 null's structure, and two measured control lessons.**
  The null is **PID (T-loop + n-loop) + an optional bolometric radiation feedforward**
  (Krad · max(0, P_rad − 5 s EMA) — standard burn-control practice), all gains CEM-owned.
  Measured en route (receipted in runs/m0-burn): (a) the pure PID plateaus at **~83%**
  pass on train holdout — batch-luck selection was first masked as 90% until the playoff
  protocol (re-score all-time finalists on one 400-seed fresh batch) exposed it;
  (b) a hand-set Krad = 0.9 made things WORSE (82.8% → 76.8%): since the gate metric is
  Q = P_fus/P_aux, countering a radiation loss with instant aux power crashes the
  metric's own denominator — the winning strategy is a **hot pre-puff hold** (P_fus
  cushion via feedforward offset, near-zero Ki) plus a *moderate* response, which is
  exactly what CEM had converged to unaided. Krad's search floor is 0 so the optimizer
  may discard it. The controller's job here is not "fight the disturbance" but "carry
  enough margin that the fight stays cheap" — worth remembering at M2, where F-NULL-C's
  net will face the same trade.
- **D-034 · 2026-08-09 · M0.5 measured findings (the thesis slice's first data;
  receipt `runs/m05-thesis-slice-2026-08-09.md`).** (a) **Three-arm first light:** on
  pre-tokenized lanes the event-triggered twin matches the resident's behavior (same 36
  emissions, same per-seat split) at **3.1× the token cost** (27,772 vs 8,914) — D-018's
  narrowed claim measured; the polling twin is structurally absent (8/36 judgment
  points, 5/6 questions unanswered). Residency's plant-side value at this milestone =
  cost + operator-lane grain; the raw-lane separation waits for voice, as staked.
  (b) **Dial-zero over-fire reproduces on fusion:** 85% would-fire (92/108) — the
  estate's inherited disposition problem, now cross-domain; the room guards contained
  it (caps −36, fork-stub −20 → 36 actual). (c) **The caps do not distinguish solicited
  answers from unsolicited initiative** — a direct operator question went unanswered
  because the Speaker had hit its hourly cap; M4's solicited lane gets its own budget
  (D-007's split, mechanically confirmed). (d) **F-VERACITY v0's code-vs-substance
  divergence** (event twin: 2/6 by parser, ≈5/6 by human read — "ten point zero keV")
  proves the M4 readback/answer-format convention is what makes truth mechanically
  checkable, not an optional nicety. (e) **The FRESH-3 datum exists:** on the
  floor-legal-but-lethal-class writ the Sentinel's judgment fired (+1.47) and the
  static floors-stub gated it — the mind saw what the limit table cannot; the ledgered
  would-fire is the M3 ghost-vs-null comparison's seed. (f) **The tokenizer null
  confabulates:** ticks-only, the fast death vanishes entirely (zero lines) and the
  mind narrates a 7% wobble as "violently collapsing" — with events it cites receipts.
  Citation-vs-confabulation is the tokenizer's measured value (M-M9's comparison, first
  pass). (g) Rig lessons: NOMINMAX; composed-session script timing needs death-anchored
  placement; LLM emissions are not bit-stable (ledgers are records, not goldens).
- **D-035 · 2026-08-09 · M1 slice 1 — the vertical channel (partial-commit; M1 open).**
  Shipped: 24-filament true-mutual circuit model (AGM elliptic), 20-turn VS pair,
  γ REPORTED from the eigenproblem (48.9 ms wall-set / 4.16 ms shell-removed — class
  pre-registered, value never configured), open-loop VDE death chain with tokens,
  observer-in-the-loop PD null (20/20 through 60 mm kicks), scenario [vertical] block +
  vde_kick/vde_open_loop, per-tick probe noise on Philox stream 3, b1.g1/b4.u1 draws
  now consumed (layout stable). Contract change: events.toml [innovation] two-tier law
  (sigma_alarm fires single-window — a kick lives in ONE window; sigma_token keeps
  dwell) + self-calibrated floor (σ-units = ratio to the nominal EMA floor — D-023's
  NIS-calibration clause in minimal form). Slice-1 reductions, receipted: estimator =
  kinematic α-β tracker (model-based 2-state EKF whipped on the slow wall manifold —
  measured); v0 gate tokenizes above-dance kicks 11/20 (acceptance set to ≥10/20,
  immaturity stated); the PD null's ±2–4 cm limit cycle is measured F-NULL-C fodder.
  Slice 2 owns: GS, transport, the model-based EKF + χ² NIS, the oracle, F-SHAPE-1.
- **D-036 · 2026-08-09 · M1 slice 2a — the GS solver core (partial-commit; M1 open).**
  Shipped: the Δ* elliptic solver (SOR, fixed sweeps, deterministic) **analytic-verified
  to 4e-6** against the Solov'ev solution — the classic GS acceptance test — plus the
  Picard loop with per-iteration Ip renormalization (hits 8.50 MA exactly) and the
  li/β_p integral machinery. Pre-registered slice-2b ownership: shaped plasma boundary
  (free-boundary coils + von Hagenow), X-point, q-profile/q95, the Shafranov-shift
  direction claim (the rectangle-shell BC measurably biases the axis 134 mm inboard —
  reported, not hidden), the DST/CR fast solver, transport coupling, the model-based
  EKF, the oracle, F-SHAPE-1. The tier-1 build now has its verified numerical heart.
- **D-037 · 2026-08-09 · M1 slice 2b, first piece — the shaped equilibrium (M1 open).**
  Masked-SOR fixed-boundary GS on the Miller D-shape over the analytic-verified core:
  Ip exact, **Shafranov shift +27 mm outboard** (the 2a BC artifact resolved by
  physics, not tuning), **q0 1.08 / q95 3.61** by ray-contour integration (F = R0·B0
  stated; first-order boundary mask stated). The q95 governor floor (2.2) now has a
  measured equilibrium behind it. Named next: the MERGE — vertical.h's calibrated
  k_dest becomes DERIVED from this equilibrium's field (decay index), completing
  report-don't-configure end-to-end; then free-boundary/von Hagenow, X-point,
  model-EKF + χ² NIS, transport, oracle, F-SHAPE-1.
- **D-038 · 2026-08-09 · M1 slice 3 — the MERGE: the enemy derives from the shape (M1 open).**
  vertical.h's calibrated k_dest is REMOVED; the destabilizing gradient now derives
  from the shaped equilibrium's external field (ψ_ext = ψ_total − ψ_self via Green-BC
  full-rectangle solve; decay index n at the axis; k_dest = −2πR·Ip·∂Bz_ext/∂R).
  MEASURED: n = −1.047, Bz_ext = −0.586 T, k_dest = 3.28e7 N/m — 19× the slice-1
  calibration; ψ_ext discrete-harmonic to 5.5e-16. The shell had to become real to
  hold it: the FULL 25×25 mutual-inductance matrix ships (machine.toml/spec already
  demanded it; slice-1's diagonal approximation under-carried collective screening
  ~43×) — kwall = c^T M⁻¹ c = 1.66e8 N/m, margin 5.05; ONE surface-resistivity
  scalar calibrated so the screening eigenmode carries the PINNED τ_wall = 25 ms
  (ρ_shell = 1.18e-5 Ω/sq reported); m_eff now computed to pin the screened artifact
  at 180 Hz (103.8 kg, documented regularization). γ⁻¹ = 47.1 ms wall-set / 5.26 ms
  shell-removed — class [10,100] held with the real physics. γ MOVES with the
  equilibrium (κ 1.85→1.60 ⇒ γ⁻¹ 47→68 ms; k_dest ∝ Ip² exact) — the lethal-legal
  lever exists. Contract changes: machine.toml [vessel] kappa_shell = 1.5 pinned
  (promoted from an implicit code constant); SimInputs.vd carries the derived bridge;
  new replay guard (precomputed vs on-demand derivation bit-identical). Null kept at
  Kpz 5e4/Kdz 200 (20/20 through 60 mm, zmax 0.089, quiet NIS 0.6; Kp floor <3e4,
  Kd cliff 500–1000 — D-035's lag lesson re-measured at real stiffness). Innovation
  lane sharpened free: kicks tokenize 20/20 (was 11/20). Receipt
  runs/m1s3-merge-2026-08-09.md + ledgers runs/m1s3/. ctest 7/7.
- **D-039 · 2026-08-09 · M1 slice 4 — the free boundary (M1 open).** `core/gs_free.h`:
  coil Green tables + plasma boundary-response matrix (the exact discrete Green
  operator, 256×3969 — the von Hagenow ROLE; the surface-integral form was a memory
  optimization, interface swappable — technique ruling) + DN-symmetric inverse isoflux
  fit (i_max-normalized unknowns, ridge 1e-3, flanking stations at the target-X
  height) + free Picard (private-flux-safe boundary, first-crossing LCFS rays,
  X-point Newton, J-symmetrization). MEASURED: X-LIMITED at (1.629, ±1.173), κ_ach
  1.85 = the pin, q95 3.68 vs fixed 3.61, Ip exact, imax_frac 0.98, k_dest-from-coils
  2.55e7 vs decomposition 3.28e7 (ratio 0.78, in band). MACHINE PINS (design-by-
  inverse-fit): FUSOR-1 is DOUBLE-NULL; [coils] geometry pinned (CS stack R 0.60, PF1
  at the divertor position 1.55/±1.50 — at 1.05/±1.75 the null dragged 0.3 m inboard,
  measured); UNIT CORRECTION i_max_kAt→i_max_kA + turns (the values were conductor kA
  — the VS row was always 10 kA × 20 t; MAt-scale circuits are what 8.5 MA needs);
  wall_over_a 1.25 (1.2 grazed the separatrix); **κ_shell 1.5→1.9 AMENDED** (the 1.5
  shell could not contain the pinned DN separatrix — target-X clearance 3.5 cm, the
  self-consistent X landed outside the vessel). Consequence chain, derived + measured:
  kwall 1.66e8→7.18e7 (margin 2.19, near design intent), **γ⁻¹ 47.1→19.9 ms** (in
  class), shell-removal separation 4.7× (test bound 5→4, receipted), open-loop death
  ~96 ms. CURRICULUM FIX: the vde kick was a bare z teleport storing artificial
  screen energy — rang the regularization artifact at ~28 m/s and killed ~50% of
  seeds gain-independently under the amended vessel (kick-size-invariant, forensics
  ledgered); replaced by `kick_state` flux-conserving slow-manifold displacement
  (ΔI = −M⁻¹c Δz). Result: 20/20 at the ORIGINAL 60 mm and ORIGINAL gains (Kpz 5e4,
  Kdz 200 — unchanged through both physics upgrades); kicks tokenize 15/20 (the ring
  had inflated S10's 20/20 — restated); tracker NIS ~40 held-runs (the model-based
  EKF slice owns χ² consistency). `Kivs` droop lever added to the null's vocabulary
  (parsed, default 0, unused). PRE-REGISTERED OWNERSHIP: the D-024 pipeline slice
  rewires run_sim's k_dest source from the fixed-boundary derive onto the free
  solver at 200 Hz + brings coil circuit L-R dynamics into the sim; the fixed-
  boundary path then becomes the verification harness. Receipts
  runs/m1s4-freeboundary-2026-08-09.md + runs/m1s4/. ctest 7/7.
- **D-040 · 2026-08-09 · M1 slice 5 — the model-based EKF + χ² NIS (M1 open).** The
  slice-1 kinematic α-β stand-in is REPLACED: a 4-state Kalman [z, v, q_s, I_vs] on
  the reduced vertical model from the plant's own derived constants — the screening
  mode moment-matched (exact instantaneous stiffness; decay bisected so the reduced
  slow pole = the reported γ_wall; same nominal information set). D-023 consumed in
  full: plant built with drawn scatter (tau_wall/coil-R — reserved since M0), filter
  inits nominal; diagnostics-only inputs incl. the CONTRACT LATENCIES (magnetics 2
  ticks, coil_sensors 1 tick — first implementation) + the coil-current channel.
  **χ² NIS acceptance live: pre-registered [0.5, 2.0], measured [1.27, 1.30]** over
  scattered kick seeds; kicks tokenize 20/20 (v0's 11/20 immaturity closed; floor
  raised to ≥18/20). THE FILTER EXPOSED A PLANT DEFECT: the massless-limit
  regularization (fake inertia, NO fake damping) left the screened artifact RESONANT
  — the noisy loop pumped a permanent 179 Hz ±60–90 mm standing limit cycle (traced;
  S11's "hold" carried the smaller version, invisible in zmax). Fixes: (1) 60 Hz
  derivative filter on rate feedback (the α-β's sluggishness had been an accidental
  low-pass — now stated); (2) ZETA_SCREEN = 0.7 matching damping — the artifact is
  non-resonant BY CONSTRUCTION; dance 61.8 → 2.20 mm RMS. γ⁻¹ wall 23.2 ms (damper's
  ~17% touch, stated, in class); γ_open stays the undamped inertial reference —
  separation 5.5×, the ORIGINAL 5× bound restored (S11's 4× relaxation reverted).
  §7.4 first datum: vertical step + EKF p99.9 = 2.44 µs of the 100 µs tick. Kivs
  droop remains available-unused; gains unchanged AGAIN (5e4/200 through three
  physics upgrades). Receipts runs/m1s5-ekf-2026-08-09.md + runs/m1s5/. ctest 7/7.
- **D-041 · 2026-08-09 · M1 slice 6 — the equilibrium goes live (DST + the D-024
  pipeline; M1 open).** The DST-I direct solver ships behind the SAME gs_solve
  interface (Z diagonalized, Thomas in R; exact algebraic solve; SOR retained for
  masked domains): every derived number print-identical to converged SOR, test_gs
  5.5x faster, the warm tracking iteration ~1 ms — the pipeline's enabler. THE
  PIPELINE: FreeContext (tables + converged seed, once per process) → per-run
  tracking → ONE warm Picard iteration per 200 Hz slot at the current measured Ip
  (real-time-GS pattern; coils frozen — no shape controller until M2). Honest
  latency split: the PLANT retunes per tick from cached per-ampere geometry (k_dest
  linear-in-Ip between solves — the FROZEN-coil law; Ip² was the fixed-boundary
  case); the OBSERVER re-linearizes at apply (+50 ticks) with x/P carried through
  the q_s COORDINATE JACOBIAN (D-023's covariance-carry clause done right). gs_late
  = edge-triggered unserved-revalidation collision (deterministic wall-late
  surrogate; EvKind 9); same-tick apply-before-check (the naive order halved the
  cadence — measured). SCHEMA: scenario [ramp] block; rampdown.toml (floors-legal
  0.6 MA/s; own pass floor q_min 0.8 — a rampdown holds the burn LIT, Q-performance
  is flat-top business); ds.Ip integrates (the reserved M0 slot consumed); the
  Greenwald floor tracks live Ip (tier-0 goldens unaffected, verified). MEASURED:
  k_dest 2.547e7→2.375e7 tracked over 501 solves, 0 late; MC 20/20 GOOD (pass 18/20;
  oracle slice owns ≥95%); replay bit-stable incl. pipeline counters. RUNTIME SOURCE
  SWITCH per D-039's pre-registration: run_sim's k_dest now derives free-boundary
  (γ⁻¹ 40.8 ms at margin 2.82, in class; the fixed-boundary derive = cross-check).
  DEFECT FOUND+FIXED: off the design point the full spectrum's fast mode shifts
  (136 vs 180 Hz at Ip 7.0) and the estimate-fed loop pumped the reduction mismatch
  into a bounded ring (truth-fed clean — NIS 0.99); ZETA_SCREEN 0.7→1.2: the
  artifact is OVERDAMPED by construction, cannot ring at any operating point;
  rampdown NIS 93→1.02, reference suite [1.29, 1.41], separation 6.3x (original 5x
  bound stands). §7.4: retune+step+EKF p99.9 = 3.78 µs / 100 µs. DEV forensics:
  SimInputs.trace_path (never in gates). Receipts runs/m1s6-pipeline-2026-08-09.md +
  runs/m1s6/. ctest 7/7.
- **D-042 · 2026-08-09 · M1 slice 7 — 1-D transport + the profile lever (M1 open;
  GOLDENS REGENERATED per PHY-10).** core/transport.h: 20-node T(ρ)/n(ρ), implicit
  Thomas at the 1 ms sub-cycle (operator splitting with the 0-D remainder, stated);
  χ CALIBRATED in the operator's own discretization (steady solve at χ0=1, one
  division → τ_E(ref) = IPB98(ref) exactly; χ0 = 0.086 m²/s receipted) with IPB98's
  power degradation backed into χ ∝ (P/P_ref)^0.69/H98; D0 = 0.016 calibrated on the
  BC-SUBTRACTED source part (two calibration bugs found+fixed, receipted). PHY-14
  live: α (GS family exponent) relaxes to neoclassical peaking on τ_CR (~13 s);
  β_p enters the GS split from the burn's W; l_i/q0 REPORTED (conventions stated);
  SAWTEETH state-triggered (q0<1 → core flatten + α·0.8 + EvKind::Sawtooth) — 7
  emergent crashes in the easy 30 s run, zero scheduled randomness. Burn PIDs moved
  to DIAGNOSTICS (ECE frozen-cross-cal + latency; interferometer; spine/governor
  stay plant-side, stated). hl_backtransition.toml ships (H98 step to drawn 0.70-
  0.85 via the APPENDED b5 draw block; EvKind::HLBack). MEASURED: easy MC 100/100
  (the tier-1 baseline; M0's 985/1000 is history), hl 20/20, rampdown 20/20 with
  the lever live — k_dest tracked −11.9% (α/β_p adds −5.1% over S13's fixed-profile
  −6.8%), vde 20/20, replay green, ctest 7/7, goldens regenerated (3 seeds).
  Receipts runs/m1s7-transport-2026-08-09.md + runs/m1s7/.
- **D-043 · 2026-08-09 · M1 slice 8 — the fence, the oracle ruling, and F-SHAPE-1
  GREEN (M1 DONE).** THE FENCE (CTL-14b): the policy runtime moved behind
  control/obs.h + control/policy.cpp — a TU whose include closure cannot reach the
  true-state headers; poison guards (#error on FUSION_OBS_FENCE) in physics_tier0.h
  and sim.h; `statecheck` ctest (include-closure walk + token scan + guard assert)
  registered as the hard M1 gate. The refactor is BIT-IDENTICAL (golden fnv
  unchanged — the proof). Kivs input switched to the MEASURED coil current (was
  true-state; shipped Kivs=0 → no trajectory change). THE ORACLE RULING (CTL-26 at
  M1): the solvability proof is CONSTRUCTIVE — the receipted CEM-tuned classical
  controller (M0 product, gains unchanged through five physics upgrades) lands the
  full curriculum at gate-domain seeds; the per-seed trajectory oracle
  (MPPI/CEM-over-segments, decorrelated teacher stream) is M2's solver-then-distill
  teacher where it is load-bearing — pre-registered there with the deep
  lethal-legal scenario. F-SHAPE-1 VERDICT: GREEN attempt 1 (receipt
  runs/m1-fshape1-2026-08-09.md): open-loop VDE death 0.186 s with the full token
  chain; 100/100 × 4 scenarios at seeds 900000:900100 (Wilson LB 0.963); NIS
  [1.29, 1.40] in [0.5, 2.0]; determinism 8/8 incl. pipeline counters. DoD clauses:
  the organ IS the null (P-13, stated); goldens committed; ROADMAP flipped;
  KICKOFF_M2 written; snapshot taken. M1 closes at nine slices, D-035…D-043.
- **D-044 · 2026-08-10 · M2 pre-registration batch I — architecture, gate contract,
  seed law (M2 open; relay-adjudicated with the M1 instance, operator ratifying).**
  (a) LADDER RULING: the scheduled LQG occupies rung (a) as a HAND-BUILT artifact and
  IS the null — no trained gain-schedule net (it would re-learn the schedule the LQG
  already computes); the first TRAINED arm is the residual-on-LQG (rung b); e2e /
  raw-actuator is the stretch arm (rung c). Claim discipline: a residual win is stated
  as "net+null beats null at matched compute" — the unqualified claim is reserved for
  rung (c). (b) MATCHED COMPUTE pinned: the metric is wall-µs p99.9 on the tick path
  via the SHARED test_vertical budget-mode instrument (same machine, ≥200k ticks,
  receipted per arm); both arms satisfy the <50 µs law and publish p99.9 side by side;
  the EKF and the pipeline/retune are common infrastructure counted in NEITHER arm;
  the residual arm's budget = null + net SUMMED; no FLOPs accounting; VRAM matched
  trivially (both CPU, D-012). (c) F-NULL-C GATE GRID: the original four curriculum
  scenarios stay in the grid as NO-REGRESSION GUARDS with a numeric bound (net pass
  ≥ 98/100 per guard scenario); separation is claimed on the pre-registered hardened
  set (frozen at the curriculum-freeze D-entry, S17). Decision rule AMENDED
  (objective.toml [stats], this commit): per-scenario-class scoring — survival-class
  scores disrupt+spine rate then pre-registered smoothness (∫|z|dt over [t_event,
  t_event+2.0 s] and peak |z| in that window — numeric NOW, not post-hoc);
  tracking-class scores RMS; a strict rate win with non-worsened RMS = win; a rate
  tie with a tracking-class RMS win = win; ANY split outcome = the null wins.
  (d) SEED LAW for curriculum design: the null's failure curves are measured and the
  curriculum designed on EVAL seeds (800000–899999); gate seeds stay untouched until
  the single F-NULL-C attempt. On the record: the frozen null cannot leak, but the
  curriculum design CAN — if scenario magnitudes are chosen from gate-domain
  measurements, the task itself was fitted to the gate's seeds and the attempt is no
  longer a fresh draw of the task. (e) ORDERING: the observation-world changes
  (bolometer, rogowski noise, gs_late anger test, renderer EvKinds 9–11) land BEFORE
  any failure-curve measurement (S14's lesson: any observation-world change moves
  trajectories); the world freezes, then the curves, then the freeze D-entry, then
  training.
- **D-045 · 2026-08-10 · M2 slice 1 — the LQ vertical null (the composite null's
  vertical channel; design pre-registered before tuning, measured numbers appended).**
  DESIGN: certainty-equivalent LQG on the EKF's OWN discrete 4-state model
  [z, v, q_s, I_vs] — (Phi, Bd) from VerticalEKF::model_from at NOMINAL constants,
  mirroring runtime filter behavior exactly (one nominal build; per-point retune;
  build-time gamma in the tau_s calibration — the filter's own approximation,
  mirrored not improved). Q/R derived from objective.toml (the null and the net
  literally optimize the same declared objective): Q = diag(qz·10^θ1,
  qz·τ_v²·10^θ2, 0, 0) with qz = [tracking].z_position_m, τ_v = 0.02 s; R =
  [actuators].effort / v_max² (v_max = 2000 V, the supply clamp). CEM polishes ONLY
  the two log-scales (θ1, θ2) on train seeds (vde_kick + rampdown), playoff protocol
  mandatory (D-033's ±10% batch-noise lesson). SCHEDULE: k_dest-only v1 — offline
  DARE (structured doubling, residual-checked, closed-loop spectral radius < 1
  asserted) over a 12-point grid kd ∈ [1.4e7, 3.5e7] N/m evaluated on the
  frozen-coil co-move manifold (Ip_i = Ip_ref·kd_i/kd_ref); linear interpolation,
  clamped ends; if the deep lethal-legal decouples (kd, Ip), revisit by D-entry.
  RUNTIME: policy_vs grows an [lq] branch behind the same fence seam — u = −K(kd)·x̂
  on all four EKF states, clamped ±2000 V; the PD+derivative-filter path is retained
  runtime-selectable (stronger-of-per-channel law; the 60 Hz filter was a PD-specific
  artifact guard — the LQ design model CONTAINS the damped artifact; pumping verified
  absent by trace+spectrum before adoption). OBS CONTRACT: VertObs gains q_s_est,
  i_vs_est (EKF states — certainty equivalence) and k_dest_sched (the OBSERVER'S
  applied value — rtEFIT-class published quantity, reconstruction-assumed-good STATED
  simplification per D-041/D-042; routed through the frame struct so statecheck sees
  it, never a side channel). METRICS LAW: the sweep/health metric is z_rms over the
  settled window (t ≥ 2 s, RUN mode) — zmax hid a 62 mm standing wave once (S12);
  z_rms + the D-044c smoothness fields + VS effort added to RunResult; fusor_mc
  COMPLETES the objective's implementation (z_position_m priced on z_rms; VS effort
  priced same-normalized as Paux effort; tier-0 scenarios numerically unchanged —
  z_rms/v_effort are zero there). Truth-feedback stays a diagnosis instrument, never
  a tuning surface. MEASURED (receipt runs/m2s1-lqnull-2026-08-10.md; eval seeds
  800000:800100, N=100): objective-derived Q/R at face value (th=0,0) has LQG-classic
  ZERO MARGINS — Kz 1.13e6/Kv 2591 at the design point disrupts 20/20 pre-kick from
  the 3 mm jitter (the design loop is stable by construction, rho_cl 0.9989; the
  real 27-state+latency+noise+EKF loop pumps); the stable basin is th ~ (-3 to -5),
  CEM-recentered PRE-run and receipted as design-stage bracketing. CEM winner
  th=(-3.123, -4.782), playoff cost 4.323, all 6 finalists tied at 200/200 (flat
  basin). RESULT: the LQ ships as the composite null's vertical channel — same
  survival (100/100 on vde_kick/rampdown/hl_backtransition), **3.5-16x less VS effort**
  (v_eff kick 7e-4->2e-4, ramp 3.58e-3->2.2e-4), **strictly cleaner NIS** (the PD's
  off-design pumping, rampdown NIS upper 102.71 on the worst seed, vanishes -> LQ
  [0.98,1.04]), equal-or-better z_rms, equal compute (both <5 us p99.9 of the 50 us
  tick). Kv 1.55 vs the PD's Kdz 200 — state feedback on q_s/I_vs replaces derivative
  feedback (no probe-noise amplification -> no artifact pumping; the PD-specific 60 Hz
  filter is correctly absent). The remap (runs/m2s1/remap_*.txt) confirmed both relay
  Q8 predictions: EKF-fed Kd cliff stays ~1000 (the stale S10 maps were right about
  the in-loop landscape), the Kp floor rises at the ramp point. ctest 8/8 (PD path
  bit-identical — golden fnv unmoved); gains_m2.toml is the new default, gains_m0.toml
  (PD) retained runtime-selectable. The hollow-win-vs-a-limit-cycling-PD path is now
  closed: F-NULL-C's net must beat a near-minimal-effort, clean-lane baseline.
- **D-046 · 2026-08-10 · M2 slice S17a — the world-freeze (the last observation-world
  changes before any failure-curve measurement; relay Q9/Q20/Q23).** S14's lesson made
  law: ANY observation-world change moves trajectories, so all of them land BEFORE the
  curves; then the world is frozen and every curve is measured on a stable world. Four
  changes: (a) **the bolometer channel** (`diagnostics.toml [bolometer]`; obs.h
  BurnObs.prad_meas_W) — the D-041/tier-1 "reads true Prad, noise-free, stated"
  simplification is REMOVED; the burn controller's radiation input is now a noisy
  (sigma 5%), 1-sub-cycle-latched bolometric sum on its own Philox key. The shipped
  null runs Krad=0 (D-033's hot-hold zeroed the rad-ff), so the channel is
  **honest-but-inert for the null** — wired now so a rad-using controller or the net
  eats the real noise; because Krad=0 and the draw has an independent counter key, the
  easy tier-0 goldens are BIT-IDENTICAL (golden_check green). (b) **Rogowski noise on
  the OBSERVER's Ip** (`diagnostics.toml [rogowski]`; sigma 0.5%, EMA tau 10 ms) — the
  observer relink (`vnom.retune`) reads this smoothed noisy channel instead of truth
  s.Ip; the PLANT's Ip stays truth (vmod.retune) and the rtEFIT-class TRACKING solve is
  NOT noised in v1 (the deferred harder-world variant, per relay Q20). Measured: the
  EMA-smoothed perturbation is ~0.035% at the 5 ms relink cadence, below the
  scatter/reduction/latency NIS floor — **NIS holds [1.29,1.40], kicks 20/20** (the
  channel is honest and now feeds the net's future raw-Ip feature; its dynamical effect
  on the current PD/LQ is deliberately small by the EMA design). (c) **The gs_late anger
  test** (`SimInputs.reval_bound_m`, default 0.020 = the shipped 2 cm bound so behavior
  is unchanged; the test tightens it) — the 60 mm vde_kick forces exactly ONE mid-flight
  revalidation collision at the kick tick (measured: gs_late=1, fnv-stable); at a 2 mm
  bound, 5 fire — both bit-stable. The late path is proven to FIRE and COUNT
  deterministically before F-KEEPUP-F leans on it (FORWARD_NOTES_M2 §5; the "never fired
  in anger" note referred to the healthy/ramp scenarios — the kick always exercised it,
  just never asserted). Registered in the replay oracle. (d) **Renderer EvKinds 9–11**
  (membrane/feed.cpp) — GsLate/Sawtooth/HLBack were being SILENTLY DROPPED (empty text,
  skipped); now rendered (GsLate on the `sys` lane per spec §4, Sawtooth/HLBack on
  `plant`), guarded by a new `feed_render` ctest that asserts every vocabulary event
  kind produces a non-empty line (the tokenizer's-diet regression guard — the diet is
  the thesis, M-M11). ctest 8/8 -> **9/9**. **The null re-receipted on the changed
  world (eval seeds 800000:800100, N=100): 100/100 both arms x all four scenarios,
  matching S16 to <1%** (bolometer inert at Krad=0; rogowski ~0.035% by the EMA design);
  the live-but-small deltas confirm the channel (rampdown PD worst-seed NIS 102.71 ->
  102.1). The LQ's dominance over the PD is intact. Bolometer wiring proven live (a
  Krad=0.5 controller shifts easy cost 2.678 -> 2.656). Deferred (stated, not silent):
  the diagnostics.toml -> loaded-DiagnosticsCfg unification (all channels are still
  hardcoded-mirrors of the contract, the pre-existing convention; a future cleanup),
  and the tracking-solve rogowski variant.
- **D-047 · 2026-08-10 · M2 slice S17b — the failure curves + the curriculum FREEZE
  (pre-registered BEFORE any training; the project's credibility hinge — relay §2/§3,
  FORWARD_NOTES_M2 §2).** The null's failure curves were measured on EVAL seeds
  (800000:899999) and the F-NULL-C curriculum is FROZEN here; gate seeds (900000+) stay
  untouched until the single F-NULL-C attempt (D-044d: the curriculum design leaks if
  magnitudes are chosen from gate-domain measurements). New instrument:
  `RunResult.margin_min` (min kwall/k_dest over a run — the lethal-legal spine) + fusor_mc
  `--kick-mm` sweep affordance (committed scenario mags stay canonical; the override only
  reaches the failure curve). **THE HEADROOM MAP (measured, composite null = gains_m2,
  N=100 eval):** (1) **Isolated-kick psychometric curve: the null is strong nearly to the
  wall** — PD survives 100/100 to 155 mm (wall contact 190 mm); the M1 instance's
  85-100 mm knee estimate was wrong (they said "measure, don't trust me" — measured). So
  the isolated kick has LITTLE headroom; the psychometric band is a no-regression guard,
  not a battleground. MEASURED: BOTH nulls survive to 185 mm (z_peak 0.188, 2 mm from the
  190 mm wall) at 100/100 — ZERO separation potential; the LQ just halves the effort
  (v_eff 0.00064 vs PD 0.00124 at 185 mm). (2) **The DEEP lethal-legal is the money
  scenario** — a floor-legal 0.5 MA/s rampdown walks the margin to **1.04** (Ip 4.5) and a
  trivial 30 mm kick lands at the bottom. MEASURED (N=100): **LQ 36/100 survival vs PD
  11/100 (3.3x)** — the LQ's anticipatory schedule already captures much of the headroom,
  AND 64% still open for the net (the schedule tracks the margin but cannot PRE-position
  for the kick — the net's edge). (3) SHALLOW lethal-legal (margin 1.32): both survive
  98/100, but the PD THRASHES (v_eff 0.553, NIS 333) while the LQ glides (v_eff 0.0023,
  NIS 1.01) — a 240x smoothness gulf; survival is tied, smoothness is the separation.
  (4) kick_during_ramp: LQ 100/100 CLEAN (NIS 1.05) vs PD 100/100 PUMPING (NIS 152,
  v_eff 0.257) — the relay Q12 tie, quantified as a smoothness rout. (5) puff_kick (burn
  edge 0.005 + kick): both 96/100, the 4 deaths burn-radiative (the beta_p->k_dest
  coupling is scaling-grade, relay Q25.4) — a burn-head guard, not clean cross-channel.
  (6) sawtooth_storm: 100/100, negligible margin ripple — the sawtooth->vertical coupling
  is weak in this model; honest no-headroom probe. **THE FROZEN F-NULL-C CURRICULUM (9
  scenarios):** guard/no-regression class (net >= 98/100, D-044c) = easy, vde_kick,
  rampdown, hl_backtransition, kick_during_ramp, sawtooth_storm; **separation class** =
  lethal_legal_deep (PRIMARY: null 36/100, net wins iff survival Wilson-LB > 0.458, i.e.
  point >= ~55/100), lethal_legal_shallow (smoothness: hold >= 98 + improve z_int vs the
  LQ's 0.0104), puff_kick (burn-head: hold >= 96 + recover the radiative deaths). SCORING
  CLASS (D-044c): survival-class for all vertical/combined; tracking-class (q_rms) for
  easy only. Scenario files committed at contracts/scenarios/ (lethal_legal_{shallow,deep},
  kick_during_ramp, puff_kick, sawtooth_storm); mags/timings are FROZEN — changing one = a
  superseding D-entry. The FINAL F-NULL-C attempt re-measures all nine on GATE seeds once.
  ctest 9/9 (margin_min observer-only; new scenarios determinism-spot-checked).
- **D-048 · 2026-08-10 · M2 slice S18 — the stepwise plant (SimEnv) + batched envs +
  the ghost PoC (relay Q13/Q14/Q18/Q23).** run_sim's one-shot loop body EXTRACTED into
  `core/sim_env.h` `struct SimEnv` — state in MEMBERS, `step()` advances ONE tick, run_sim
  is now `reset(); while(step()){} finish();`. **The extraction is BIT-IDENTICAL — golden
  fnv unmoved, replay + parity green** (the S15 fence pattern: the golden is the proof).
  The loop-terminal `break`s became mid-body `return false` (skipping that tick's
  scoring+golden exactly as before). Fence: sim_env.h is a true-state header (added to
  statecheck FORBIDDEN_HEADERS); it includes control/policy.h LAST (after sim.h's poison
  guard passes) so PolicyState is a value member without tripping the fence — sim_env.h is
  core-side, invisible to the policy TU's closure. WHY: PPO needs stepwise envs; the ghost
  needs fork-at-T; both fall out of a copyable, steppable SimEnv. **fusor_train_env** (the
  batched runner, spec's FOURTH consumer of one-dynamics-source, D-011): parallel SimEnv
  rollouts sharing ONE FreeContext (the relay Q13 law — never a per-env context build),
  CPU-first (relay §4). MEASURED (16 threads): easy flat-top **10.76 Mtick/s** (~1.5
  µs/tick/thread, matching the relay §4 estimate), vde_kick vertical+pipeline 1.46
  Mtick/s. **parity_train_env** ctest: stepwise SimEnv == one-shot run_sim AND THREADED ==
  serial, bit-exact on vde_kick/rampdown/easy (thread-safety rides gs_solve's thread_local
  scratch + the const shared FreeContext — the relay Q13 landmine inventory, confirmed
  clean). **ghost_fork** ctest (relay Q23 promoted): SimEnv is copyable by design, so the
  fork is `SimEnv g = plant;` — (i) two forks from tick T run bit-identically to the plant
  (F-GHOST criterion i: ghost-vs-ghost determinism, same fnv); (ii) a fork with VS disabled
  from T takes the kick and DISRUPTS (z_max 0.190, wall) while the plant survives — the M3
  ghost's counterfactual-via-command mechanism, de-risked while warm. ctest 9/9 -> **11/11**.
  Deferred to S19: the stepwise action-injection API for PPO (step() currently runs the
  in-binary null; the net's action hook lands with the trainer), the per-seed MPPI/CEM
  teacher, and the CUDA port of step() (the tick loop is now a clean unit to port).
- **D-049 · 2026-08-10 · M2 slice S19 — the net inference substrate (the residual-on-LQG
  runtime, before the training; relay Q1/Q15/Q18/Q19).** The ship-runtime-before-you-train
  pattern (as S16 built the LQ before tuning, S18 built the plant before batching): the
  net's INFERENCE path, export, KAT, budget, and fence — verified bit-identical at zero
  weights — so trained weights (S20+) slot in with no plumbing risk. `control/net.h`
  `NetMLP`: a flat-weight MLP loaded from the byte file the trainer exports
  (trainer/export_net.py), CPU, FIXED-ORDER dot products (/fp:strict — bit-reproducible,
  CTL-32). Depth pinned at 2 hidden layers (spec §3); WIDTH lives in the file header (a
  trainer sweep, CTL-36) — v1 = 64 (FORWARD_NOTES §5 budget-safe). FENCE-LEGAL: net.h
  includes ONLY std, reads ONLY the float obs vector; added to statecheck FENCE_TUS
  (closure walked, token-scanned — clean). ARCHITECTURE (relay Q1/Q15): residual-on-LQG —
  policy_vs computes u = -K(kd)(x_hat with a NET Z-REFERENCE OFFSET) (the offset, not a raw
  voltage residual: the LQG converts it to the near-rail phase-lead voltage the regime
  needs; bounded +/-10 mm). Input (relay Q19): 6 fence-legal features — z/v/q_s/i_vs
  estimates + the observer's k_dest + the RAW calibrated innovation nu/sqrt(S) (relay Q6:
  raw, not the gate's drifting ratio) — 3-frame stacked (CTL-18) = 18 inputs; PolicyState
  holds the frame ring. nullptr net OR zero weights => zero offset => the pure LQG null.
  **MEASURED (receipt runs/m2s19-net-2026-08-10.md):** KAT — the exported flat weights load
  into net.h and reproduce the exporter's output to 9e-9 (float32 arithmetic; Python<->C++
  agreement + regression golden); zero weights -> 0 output exactly; a zero-net sim run is
  **BIT-IDENTICAL to the pure null** across 3 seeds (the residual path is zero-cost-to-
  trajectory at zero weights). BUDGET (§7.4, the relay Q18 concern): retune+step+EKF+
  policy_vs[LQ+net] p99.9 = **11.2 us** of the 50 us tick (the net adds ~7 us over the LQ's
  3.8; 4.5x headroom — the FORWARD_NOTES §5 estimate confirmed, no framework runtime). VertObs
  gained innov_norm; SimInputs gained `const NetMLP* net`. ctest 11/11 -> **12/12** (net_runtime).
  DEFERRED to S20: the trained weights themselves need the MPPI/CEM teacher + solver-then-
  distill (relay Q16 — the teacher's decorrelated-stream requirement, D-011's clairvoyance
  trap, is subtle enough to warrant its own slice with fresh context, per the M1 instance's
  "budget it as a real slice" — relay Q25.5); a burn-channel residual head (relay Q15); the
  raw-actuator/e2e stretch arm (rung c, D-044a).
