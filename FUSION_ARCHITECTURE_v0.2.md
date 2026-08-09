# FUSION — the reactor you can talk to
### Architecture / design spec v0.2 · 2026-08-09 · Bo Chen · Access Intellect LLC

**Status: SPEC, pre-build — the QC revision.** Core binary: **`fusor`**. Root: `C:/fusion/`.
v0.2 dispositions the five-auditor swarm pass (146 findings; `docs/qc/QC_*.md`) **and** the
fresh-eyes second-instrument pass (`docs/qc/FRESH_EYES.md` + Addendum) — the full ledger is
`docs/qc/DISPOSITIONS.md`. v0.1 is preserved beside this file, frozen. Decisions through
**D-030**. **M0 is unsuspended against THIS revision.**
**Lineage (three proven parents, one law each):** the **Booster Lander Simulator**'s
constitution, the **auricle/SYNCYTIUM** membrane (measured through M4.5b), and
**TinyVillage**'s laws. Machine config: **`contracts/machine.toml` (FUSOR-1)** — every
derived number in this spec cites it.

> **One breath:** a tokamak simulation with real (tiered, honest) physics, flown at kHz by a
> trained neural controller that must actually hold the plasma or it disrupts — watched,
> remembered, and **voiced** by a resident LLM triple on the warpbus, with **full-duplex
> speech**: you talk to the reactor through a mic; the reactor talks back, and when your
> floor-legal-but-lethal command is forming, it shows you the ghost of the disruption it just
> simulated and talks you out of it — **causally, via you, with the receipt**. Every claim
> carries a receipt. Every organ has a null. And every falsifier can lose.

---

## §0 · The claim, and what is honestly being claimed

The claim is **not** a predictive fusion code. Tier-0/1 physics use the real published
formulas on the synthetic FUSOR-1 config; anything decorative is labeled decorative. The
claim **is**:

(a) **A control problem hard enough that continuous kHz-class feedback is mandatory.** An
elongated plasma is vertically unstable; on FUSOR-1 the growth time is **wall-set, γ⁻¹ ≈
25–40 ms at the design point** (derived from the vessel L-R in `machine.toml [vessel]`;
reported from the eigenproblem at each equilibrium, never configured). Without feedback the
plasma is lost in ~5–6 e-foldings (~150–300 ms). Whether a *neural* controller beats a
well-tuned classical one is **not asserted** — it is exactly what F-NULL-C measures, and the
classical null is strong on this problem by design (vertical stabilization is LQ's home
turf — the honest statement of where a net might earn its keep is multi-objective shape +
current + density + burn control under coupled constraints, actuator saturation, and fault
recovery; CTL-15).

(b) **An architecture in which a fast non-linguistic cerebellum and a resident linguistic
cortex share one machine honestly** — with the anti-turn claim stated at its true width
(D-018): on **pre-tokenized lanes** the innovation gate is itself a threshold-waker, so
residency's plant-side value is *hot-memory integration across events*, not wake timing;
the full anti-turn claim lives where the stream is **raw** — the operator's forming speech,
mid-partial perception. F-PRESENCE-F's three arms exist to measure exactly that seam.

(c) **A demonstration that "own the loop" produces a machine that can sense, remember, and
speak about its own life with receipts** — and whose speech is itself gated by a falsifier
(F-VERACITY): a reactor that cites its tape must be checkable against its tape.

The DeepMind/EPFL TCV result (Degrave et al., *Nature* 602, 2022 — deep-RL driving 19
poloidal-field coils at 10 kHz on a real tokamak) is the existence proof for the control
half. Stated precisely (CTL-17): it proves the *net size and rate* (a feedforward MLP at
10 kHz); its training pipeline was **MPO, pure sim-RL, no distillation** — which is rung
(c) of this project's M2 ladder, not its default.

---

## §1 · The stack — three tiers, two seams

*(v0.2: the depths tier is cut — D-020; it returns only with a consumer, a budget row, and
a gate. The spine row is multi-rate per D-003 — the flat "100 µs" of v0.1 was the first
table a cold session read and the first thing it would mis-build; M-N5.)*

```
TIER            GRAIN                     SUBSTANCE                        LAW
──────────────────────────────────────────────────────────────────────────────────────
spine           100 µs tick; burn terms   deterministic C++ plant +        state only through
                sub-cycled at 1 ms;       safety gates; GS equilibrium       the integrator;
                GS re-solve at 200 Hz     re-solves pipelined async          gates always armed
                (pipelined, D-024)        (CUDA), linear vertical model
                                          carries 10 kHz between solves
cerebellum      100 µs                    policy net (CPU, fixed           if it can't hold the
                                          precision) + EKF innovation        plasma, it disrupts;
                                          source; MPPI/CEM is OFFLINE        spine shutdown = a
                                          (teacher only, D-024)              FAILURE class
cortex          ~1–3 s boundaries         resident LLM triple on the       anti-turn on raw
                                          warpbus (Speaker/Skeptic/          lanes; event-clocked
                                          Sentinel), full-duplex voice       on plant lanes
                                          (D-018); boundary-gated
                                          {hold,emit}; molt-on-calm
──────────────────────────────────────────────────────────────────────────────────────
SEAM ↑ (plant→cortex):  INNOVATION-GATED TOKENIZATION — the plant earns tokens by
                        surprising its own cerebellum's EKF (per-cluster Mahalanobis,
                        contracts/events.toml); nominal = ticks = priced silence
SEAM ↓ (cortex→plant):  WRITS, NEVER ACTUATORS — typed revs (contracts/writs.toml v1)
                        through deterministic floors; refusals AND acceptances receipted
BOTH SEAMS:             THE DEADLINE LAW — fixed budgets + aborts + drop-events; the
                        100 µs world never dilates for a model
```

---

## §2 · The plant — `fusor` core (C++20 + CUDA, headless-first)

### 2.1 Physics ladder (real formulas, honest tiers; machine = `contracts/machine.toml`)

- **Tier 0 — the burn (0-D, the M0 game).** D-T reactivity ⟨σv⟩(T) via **Bosch-Hale**
  (validity 0.2–100 keV asserted in code; PHY-28); alpha heating with **E_α = 3.52 MeV
  pinned, one-pole slowing-down lag τ_s(T_e,n_e)** (PHY-19 — the lag is what makes burn
  control a control problem); **Bremsstrahlung** (∝ n²√T Z_eff); **impurity line radiation
  P_line = n_e n_imp L_z(T_e)** with a tabulated coronal L_z for the machine's impurity
  species (PHY-08 — this is what makes radiative collapse, density limit, and the impurity
  puff *reachable*); synchrotron (Trubnikov, wall reflectivity from machine.toml; PHY-18);
  **Ohmic heating + a volt-second budget** (PHY-15 — the ramp is a resource game);
  **particle balance**: dn_e/dt = S_gas + S_pellet + S_NBI − n_e/τ_p (τ_p ≈ 2 τ_E) and
  **helium ash** dn_He/dt = R_DT − n_He/τ*_He (τ*_He ≈ 5 τ_E) with fuel dilution feeding
  ⟨σv⟩ and Z_eff (PHY-07 — the actual antagonist of the sustain-Q game); energy confinement
  via **IPB98(y,2)** solved **implicitly** in W (τ_E is fit against P_loss = P_heat −
  dW/dt; the convention is stated in code and the equation sub-iterated — PHY-09), with
  **κ_a = S/(πa²)** from machine.toml and **H98 exposed as the free multiplier** (it is
  also the H→L transition knob). Live **Q = P_fus/P_aux**. **Integration: fixed-step RK4
  at dt = 100 µs; burn terms sub-cycled at a compile-time ratio of 10 (1 ms), except
  during quench windows, where they run per-tick** (PHY-24).
- **Tier 1 — the shape (2-D, control-grade; authoritative from M1 on).** Free-boundary
  **Grad-Shafranov** (Picard + **fast Poisson via DST/cyclic reduction, von Hagenow
  free-boundary coupling** — the solver is named, PHY-11), grid **65×65 (v1; ranges
  collapsed per D-020)**; PF coil circuits (12 actuated circuits per machine.toml) **plus
  the passive conductor model — 24 axisymmetric filaments with the full mutual-inductance
  matrix (passive–passive, passive–coil, passive–plasma), built general enough to carry
  quench eddy/halo forces later** (PHY-03/CTL-01: without this the vertical mode is
  Alfvén-fast and the whole premise evaporates); **γ reported from the eigenproblem at
  each equilibrium** (it moves with l_i and κ through the ramp; the null's gains schedule
  on it); **current-profile evolution: parameterised p′(ψ)/FF′(ψ) with l_i and β_p as
  evolving 0-D states, neoclassical resistivity, sawtooth crash resets q₀** (PHY-14 —
  GS has its inputs); 1-D radial transport (n(ρ), T(ρ), 20 nodes, prescribed χ **calibrated
  so the volume-integrated τ_E reproduces tier-0's IPB98 at the reference point** — tier-0
  demotes to local source terms + the M0/MC/ghost reduction; **switching tiers invalidates
  goldens**, PHY-10); synthetic diagnostics per `contracts/diagnostics.toml`.
  **Multi-rate law (D-003 + D-024):** the GS solve is **pipelined by construction** — the
  solve scheduled at tick T applies at tick T+50 (200 Hz cadence, one-window latency,
  deterministic in sim time); between solves the **linearized rigid-displacement model
  carries the vertical/position dynamics at 10 kHz**, re-derived at each apply. If a solve
  is not wall-done at its apply tick, **the sim waits** (real-time factor dips — recorded
  as a `[plant] organ gs_late` event; determinism is never traded for pacing) — F-KEEPUP-F
  is defined on aggregate pacing, not per-tick (PHY-11). The linear model's **validity
  bound in ΔZ triggers an event-driven early re-solve** (CTL's open question, answered: a
  VDE simulated on a stale linearization far outside its validity would be a fake
  disruption).
- **Tier 2 — the spectacle (3-D resistive MHD, renderer-only).**
  **[OPERATOR RULING PENDING — D-020.]** Recommended v1: **cut**; the renderer raymarches
  a tier-1-derived emissivity field (n, T on the GS geometry) with artistic noise — the
  beauty stays, the GPU consumer, VRAM row, and GB/s telemetry problem go. If ruled in:
  64³ fp16, 10 Hz, and it **feeds the renderer, never the controller**.

### 2.2 The booster constitution, inherited verbatim (v0.2-hardened)

1. **State changes only through the integrator.** Controller output = actuator vector.
2. **No assist term.** Disruption, VDE, and quench are fully simulated valid outcomes
   (§2.4 — the physics now exists). **Spine-gate shutdowns are a FAILED outcome class**
   (`SPINE_SHUTDOWN` in the MC verdict taxonomy, `contracts/spine_gates.toml`) — a
   controller rescued by the plant has failed, and the cost ordering makes learning to
   induce it a losing strategy (CTL-10).
3. **One dynamics source.** Plant, MPPI/CEM teacher rollouts, ghost-forks, **and the
   batched training envs (`fusor_train_env` — the fourth consumer, D-011)** call the same
   `__host__ __device__` equations, including actuator lag. A ctest asserts
   train-env↔plant parity (bit-exact on CPU; toleranced per-arch on GPU, the parent's own
   scoping).
4. **Deterministic, provably.** Philox4x32-10 counter RNG with the **stream-domain law**
   (`contracts/dispersions.toml [streams]`): ghost shares the plant's stream (that sharing
   *is* F-GHOST); teacher and training envs are counter-decorrelated (the clairvoyance
   trap, CTL-05 — a ctest asserts a teacher rollout cannot predict disturbance onsets
   better than chance). Compile-time dt; no wall-clock in the sim path; no unordered FP
   reductions; `/fp:strict`-class flags. Same seed + scenario + **input tape** ⇒
   bit-identical trajectory (memcmp oracle). **Live-mind sessions are replayable**: the
   run artifact records accepted writs + operator-input timeline as replayable inputs
   (`writs.toml [input_tape]`, FRESH-6 — the booster's recorded-pilot pattern).
5. **Headless must work.** **`fusor_mc`** (separate binary — P-21 ruled) runs seeded
   scenarios and prints GOOD / DISRUPT / **SPINE_SHUTDOWN** / TIMEOUT rates and tracking
   RMS with Wilson 95% CIs (rates) and paired-bootstrap CIs (RMS), per
   `contracts/objective.toml [stats]`. That output is the gate for every controller change.
6. **Fork = physics-space snapshot.** `fusor_fork(state) → handle` snapshots **mutable
   state only** (static Green's/inductance tables shared, not copied — PHY-29; tier-0/1
   < 1 MB). Ghost mode is **declared** (§5.4): frozen-equilibrium tier-0 burn + the
   linearized vertical/circuit model — a named, tested approximation, not silent drift.

### 2.3 Scenario set (schema: `contracts/scenarios/_TEMPLATE.toml`)

`(plasma-initiated start) → current ramp → flat-top → burn → ramp-down` — **breakdown/
burn-through is explicitly out of scope** (runs begin at the machine.toml `[initial_state]`
handoff; PHY-13). Disturbances: sawtooth crash (**state-triggered**: q₀ < 1, Porcelli-type
criterion), ELM burst (**state-triggered**: pedestal criterion), impurity puff (radiative
collapse — now reachable via P_line), density-limit approach, pellet, NBI trip,
single-coil failure, **vde_kick** (impulse in Z — the enemy is in its own curriculum,
PHY-12), **H→L back-transition** (step drop in H98 — the disturbance IPB98 alone cannot
produce), **ramp-down** (a large fraction of real disruptions live there), and combined
cases. **Plus the lethal-legal class** (FRESH-3): pre-registered scenarios whose commands
pass every floor and still kill the plasma inside the ghost horizon (e.g. a floor-legal
I_p ramp that drives the vertical eigenvalue unstable given the current l_i/κ trajectory)
— the only class on which ghost-vs-governor value separates; F-PRESENCE-F's planted-error
battery draws ≥ 5 cases from it. Each scenario = template-conforming TOML + seed; goldens
in `goldens/`.

### 2.4 The disruption model (new in v0.2 — PHY-06; "the token and the physics ship together")

- **Triggers (deterministic, dwelled):** |Z−Z_ref| > wall-contact bound; q95 < 2.0
  (dwell); f_rad > 1.0 (dwell); n/n_GW > 1.05 (dwell); each firing tokenizes with cause.
- **Thermal quench:** T → machine.toml `T_post_TQ_keV` over τ_TQ ≈ 0.5 ms — **runs on the
  100 µs path** (PHY-24).
- **Current quench:** dI_p/dt = −I_p·R_p(T_post)/L_p, exponential with τ_CQ from post-TQ
  Spitzer resistivity, **clamped at the ITPA area-normalized floor** (machine.toml
  `[disruption]`; ≈ 10 ms at FUSOR-1's 15 eV).
- **Out of scope v1, stated:** halo currents, runaway electrons (§12).
- **Event vocabulary:** `events.toml [terminal]` — vde / thermal_quench / current_quench /
  disrupt / soft_shutdown / hard_shutdown, cause required. The M5 "beautiful catastrophe"
  is this model rendering; the Speaker can name what killed it because the token says.

---

## §3 · The cerebellum — control nets (booster doctrine, fusion plant)

- **Observations (synthetic diagnostics only — never true state):** the
  `contracts/diagnostics.toml` channel set (40 magnetics with integrator drift,
  interferometer, Dα, neutrons, coil currents/temps, ECE), each with σ, bias walk,
  latency, dropout. **The fence is mechanical (CTL-14b): `statecheck` — a ctest, hard
  M1 gate — asserts the observation builder and policy runtime link against a header that
  cannot expose the true-state struct; the shipping target fails to compile if it
  references it; a grep gate backs it.** The reward path may read true state (training-
  only); the observation path may never.
- **The policy also eats the innovation vector** (CTL-21 — the parent's
  acceleration-residual law: the one channel that reveals *any* contingency without naming
  it). **Hide-the-flags is an acceptance test**: the policy never reads scenario flags,
  and M2's gate includes evaluation with faults unflagged.
- **Actions:** 12 circuit voltages (machine.toml `[coils]`), gas puff, pellet trigger,
  NBI/ECRH power. Rate-limited, saturating, lagged.
- **Policy:** **MLP + 3-frame stack (decided — CTL-18)**, 2 × 256 hidden (v1 default
  pinned; sweeps live in the trainer, not the contract — CTL-36), **on CPU at fixed
  precision (D-012), < 50 µs p99.9 as a CPU/AVX2 budget**, weights mmap'd flat
  (`trainer/export_weights.py` + byte-equality golden + known-answer test at M2 — CTL-32).
  No Python in the product loop.
- **The innovation source (D-023, the EKF, fully specified):** model = the same linearized
  rigid-displacement + circuit model the spine's 10 kHz tier runs (one dynamics source,
  linearized), driven by `diagnostics.toml` channels only. **Information set (FRESH-4):
  the EKF never sees the disturbance schedule, the blind dispersion truths, or any
  scenario flag — its model carries the pre-registered parameter mismatch of
  `dispersions.toml [plant_scatter]`.** That handicap, not the σ threshold, is what makes
  innovation mean something in a simulator. Covariance carries across re-linearizations
  (bumpless); innovations are normalized by the current S; **a rolling NIS χ² consistency
  test is a pre-registered EKF acceptance criterion at M1** — until it passes, events.toml
  σ-thresholds are provisional by definition. MPPI rollout error is *not* an innovation
  source (the planner is offline — D-024).
- **Training = the M2 ladder (D-025; the parent's measured record respected):** the
  CEM/MPPI **offline** teacher proves each scenario solvable (per-timescale horizons,
  piecewise-constant action segments 1–10 ms, information set decorrelated — CTL-26/05).
  Then, pre-registered rungs: **(a) θ/gain-schedule net over the PID+LQ base — the
  parent's proven GREEN; F-NULL-C must be satisfiable here; (b) residual policy
  (cmd = base + NN); (c) end-to-end distillation — the parent's measured 0/16 null — or
  pure sim-RL (MPO, the TCV-proven third rung; PPO only with KL-to-teacher constraint,
  CTL-33).** Any distillation rung names **DAgger** (covariate shift on an unstable plant
  is worse than on a booster — CTL-19). Batched CUDA training envs (2048; fourth
  consumer). Training mode is headless and LLM-free.
- **The null (CTL-28/29/29b):** **PID + LQ-vertical, CEM-tuned on the training seeds with
  a declared budget, tuning receipted at M0** — and it **ships in the binary as a
  permanent runtime-selectable arm** (TinyVillage L11; the M5 toggle requires it anyway).
  The textbook MPC is a later, separately-receipted comparison (D-020). **The net ships
  only by beating the shipped null** under `objective.toml [stats]`' decision rule,
  published either way; a null win is a completed milestone (DoD).
- *(MCTS scenario planning and sysid-by-backprop are out of §3 — parked post-M5 with named
  nulls if revived; CTL-30/31, D-020.)*

---

## §4 · The membrane — the resident triple on the warpbus

**Engine:** the auricle stack as measured — Qwen3.5-9B Q5_K_M on one `kv_unified` 32k trunk
(assert `n_ctx_seq == n_ctx`), three seats via branch-time role headers (A3-safe; **the
Sentinel's partial-watching branch is torn down at commit** — M-N9), arm-B worked-example
seed (with the M4 caveat carried: 36/39 held-out, OVERLAP not separable — the QLoRA case),
**isomorphic segmenter** + boundary-gated `{hold,emit}`, **molt** at watermark 24576
(scribe rung ≤ 600 tok — the cap binds, receipt's own caveat; ~9 s outage as a drop-event),
surprisal tap logged (**measured to exist, NOT yet validated as a signal — no organ drinks
from it without its own validation; M-N8**), and the **durable tape** (auricle
`src/fabric/durable.h` pattern: committed revs append to a hash-chained on-disk log under
`runs/<session>/tape/`, re-folds intact on restart; partials never persist — FRESH-8).
Reuse: `cortex_llama.cpp` / `soak.cpp` machinery + `fabric.h` two-plane bus.

**Boundary law (M-B6 — the segmenter is linguistic; machine lanes need their own):**
plant-lane boundaries are **structural** — one event line = one boundary, coalesced per
`events.toml [budget] crisis_boundary_min_ms`; the softmax segmenter governs the human
lanes (`opv`/`opt`) only. *(This is also the honest edge of the anti-turn claim — D-018.)*

**Seats (mandates are dials; the vague mandate is the measured fire-hazard):**
- **SPEAKER** — the reactor's voice: answers, reports, debriefs, first person, every claim
  citing a bus rev — **and F-VERACITY audits the citations** (§8).
- **SKEPTIC** — contests the controller's health claims, the Speaker's diagnoses, and the
  operator's assertions, against the record.
- **SENTINEL** — watches `opv` **partials** (reflex plane) + the plant lane; fires
  ghost-forks. **Its emit is fork-gated (M-M2): the Sentinel interjects only when
  `fusor_fork` returned disruption** — the estate's worst-measured seat (1,630/1,817
  emits at dial zero) becomes its only mechanically-verified one. Its null is thereby a
  klaxon (M-M9); what the LLM adds is *communication* — measured under F-PRESENCE-F's
  grading, on the lethal-legal class.

**Room guards (M-B7 — auricle's smoke-1 measured the cascade: 49% fire-rate, the trunk ate
itself; fusion is room-real by construction, so all three guards are law):** (1) self-echo
suppression — minds never judge boundaries on shadow/self lanes (the lane stamp gives it
free); (2) per-seat emission rate caps (pre-registered: SPEAKER 30/hr, SKEPTIC 12/hr,
SENTINEL 12/hr + fork-gate); (3) cascade breaker — `events.toml [cascade]`: rolling
fire-rate > 0.30 auto-demotes the room to pure-watch, and the demotion is a receipt.

**Molt-on-calm (M-M6):** molt triggers on (watermark ∧ plant-nominal ∧ operator-silent)
with a hard ceiling at watermark + 4k; if the ceiling hits **during** crisis, the trunk
sheds oldest plant-lane verbatim (already summarized in storm events) instead of folding —
the ~9 s blackout never lands mid-crisis by design. The estate's own queued fix
(threat-gated consolidation) is the v2 path.

**Bus lanes:** `plant` (innovation-gated events), `opv` (operator voice: partials → reflex
plane, finals → trunk), `opt` (operator typed), `spk/skp/sen` (the room), `writ` (downward
revs), `sys` (drop-events, molts, gate refusals, **writ acceptances**, gs_late,
**mind_died** — the cortex host has a watchdog and its death is a tape event, FE-20).

### 4.1 The tokenizer of machine experience (seam ↑)

The plant speaks only when reality diverges from its cerebellum's EKF (innovation = the
NIS-calibrated residual; §3). **Ownership (M-M11):** the plant emits fixed-size POD event
structs over an SPSC ring; `membrane/` renders text; no `sprintf` exists on the tick. The
vocabulary, thresholds, budget, and cascade guard are **one pre-registered table:
`contracts/events.toml`** (v1: per-cluster Mahalanobis + dwell + refractory + storm
collapse; `[terminal]`/`[organ]`/`[phase]` complete the curriculum's vocabulary; budget:
nominal ≤ 150 tok/hr, crisis ≤ 40 tok/s enforced by coalesce-with-receipt, never silent
drops). **The vocabulary freezes at M3** (append-only after the first logged shadow tick).
**The tokenizer's null (M-M9):** a fixed-cadence summarizer + naive threshold logger at
matched token budget — the upward seam's entire empirical claim is beating it, published
either way.

### 4.2 The writ grammar (seam ↓) — and the governor

Typed revs per **`contracts/writs.toml` v1**: id/rev/supersedes/ttl/`formed_tick`/
`rev_observed`, typed units in the schema, enumerated params, a `cancel` type, a
`schedule` type, and the **autonomy dial in true risk order (CTL-22): A0 log → A1 confirm
→ A2 delayed-cancelable → A3 auto-within-envelopes**; effective autonomy = min(class,
source, global). The deterministic translator compiles writs to reference schedules
through **`contracts/floors.toml`** (dir-typed, unit-keyed, margins below every machine
limit, dormant floors labeled). A writ past a floor — or staler than its class window
(M-B3) — is **refused, and the refusal is a receipt**; **every acceptance is receipted
too** (`[sys] writ accept …`, tick-stamped — the World mints the act receipt, TinyVillage
L5; commit *is* this receipt). The spine's own gates live in **`contracts/spine_gates.toml`**
— predicates, dwell, hysteresis, false-positive budgets, and the SPINE_SHUTDOWN failure
class (CTL-09/10). **v1 ships: A0 for unsolicited initiative, A2 for voice-sourced numeric
setpoints (3 s spine-side objection window + readback), A3 only for envelope-tightening.**

### 4.3 The Deadline Law at the seam

The triple is a guest organ with a **fixed budget + abort (M-B2): judgment budget = 2 s
hard, enforced by an abort in the sampler loop; p95 ≤ 2 s is the performance target under
it; the abort emits `[sys] drop llm rev=N reason=deadline`** onto the trunk — the mind can
later say "I was slow during the ELM burst," with a receipt. (No estate receipt reports a
p95 — the measured means are 327/349 ms, maxima 2.1/5.5 s; the 2 s hard budget is a
design choice staked against them.) Every writ carries `formed_tick` + `rev_observed`; the
governor's staleness gate refuses stale writs structurally (TinyVillage L2, both halves).
The 100 µs world never waits — the GS pipeline's wait (§2.1) is *inside* the deterministic
plant, priced by F-KEEPUP-F's pacing budget, not a dilation for a model.

---

## §5 · The voice loop — full-duplex speech

### 5.1 IN: mic → streaming ASR → the trunk (two-plane law)

- **Capture:** WASAPI (reuse `auricle/src/capture/`, measured ~24 ms), 16 kHz mono.
  **Headset required in v1 (D-014)** — wired, closed-back; Bluetooth is disqualified from
  every latency budget (V-10).
- **ASR — one backend, two model dirs (V-04):** sherpa-onnx (vendored, version-bumped +
  sha256 re-pinned — it **already supports Nemotron 3.5 streaming**; pre-exported ONNX
  exists) running **on CPU** (auricle's recorded rule; removes a CUDA consumer and ~1 GB).
  Primary model: Nemotron 3.5 streaming 0.6B at **160 ms chunks**, native punctuation
  feeding the segmenter; fallback dir: the proven zipformer + Parakeet truth lane, same
  `asr.h` seam. **M4 measures the latency-vs-WER curve across the chunk ladder on tokamak
  vocabulary and picks the operating point by receipt (V-05)** — published WERs exist only
  at 1120 ms chunks, and a misheard in-range numeral defeats floors, so **numeric writ
  params get readback + confirm (A2) and a keyword-boosted decode on the writ lane**.
- **Two-plane discipline (adapted from SYNCYTIUM — M-M10):** partials → reflex plane only
  (the Sentinel's branch, torn down at commit); finals → trunk on lane `opv`; endpoint
  ~0.35 s. **Nothing synthesized can commit to `opv`** (attribution rule at the seam —
  the semantic echo head is closed by construction, V-01ii). The human-lane
  partial→branch-pause mechanism is **new work, budgeted as such** (auricle's reflex
  receipts are peer-to-peer `dep`-triggered; this is not that).

### 5.2 OUT: the Speaker → streaming TTS → abortable playback

- **`tts.h` is a contract (V-15, D-028):** 22.05/16 kHz mono fp32; **clause-grain
  streaming synthesis** (Piper synthesizes per-sentence; the caller splits at the
  segmenter's clause boundaries — V-09's fix); **queued-PCM cap ≤ 60 ms** (V-10's law: a
  pause that merely stops writing takes the queue's length to be heard); abort returns the
  sample-accurate cut point for the voiced-vs-drafted ledger; **first audio ≤ 300 ms after
  the first clause boundary** (re-anchored — not after full emit commit), **last word →
  first phoneme ≤ 1.5 s p95** (chain C, now a budgeted row in §7).
- **Barge-in:** VAD (Silero, 32 ms frames, 1–3 frames past onset) → playback soft-pause
  **≤ 150 ms end-to-end** with the five-term sub-budget in §7; the ASR partial attributes
  afterward. **Self-speech gate driven by the render clock** (while our own PCM is in
  flight, barge-in VAD needs the echo-aware threshold — V-01i).
- **Resume-or-abandon (honest per the receipt — M-M10):** v1 ships **abandon-or-restart at
  the last clause boundary** (never mid-word); free resume is a v2 item in §12. The ledger
  records voiced-vs-drafted with sample-accurate cuts.
- **Etiquette:** per-seat caps + fork-gated Sentinel (§4). Dial-zero over-fire, measured on
  the only full-day run: **2,823 emits / 3h03m51s = 921/hr (run C)** — out loud that is a
  fire alarm; **solicited speech ships at M4** (gated by F-VOICE *and* F-VERACITY);
  **unsolicited initiative stays shadow until F-INSTINCT passes** (D-007, eval set now
  scheduled — §8).

### 5.3 Duplex rules

Both sides may initiate; neither waits for a turn. Operator barge-in soft-pauses the
reactor mid-word (abandon/restart per 5.2). The reactor interjects during operator speech
only through the boundary-gated emit + etiquette caps + (for the Sentinel) the fork gate —
and every interjection cites its rev or its ghost.

### 5.4 The ghost (the killer feature — now with its arithmetic honest)

**Ghost mode, declared (PHY-04):** `fusor_fork` + **frozen-equilibrium tier-0 burn +
linearized vertical/circuit dynamics** — a named approximation (50–100 full GS solves
cannot fit a 25 ms wall budget; pretending otherwise would make F-GHOST fail by
construction). Budget: fork + 500 ms lookahead ≤ 25 ms wall on the shared stream.
**F-GHOST is rescoped accordingly (D-026):** (i) ghost-vs-ghost bit-determinism (memcmp,
same fork twice); (ii) **bounded divergence vs the full plant** — |ΔZ| < 2 cm and
|ΔQ| < 0.5 at 500 ms on the pre-registered validation set, published either way; (iii)
warning-precedes-commit on the tape. The fork **shares the plant's Philox stream** —
counterfactual on the command alone (D-011).

**The pipeline that makes the warning land (M-B5 + V-03):** on fork-returns-disruption the
Sentinel's interjection is a **templated, deterministic line** (the ghost carries the
content: "belay that — fork shows VDE in 300 ms, q95 through the floor") — probe ~65–70 ms
+ TTS pre-warmed on ghost-fire ⇒ audible ≈ 0.4 s, comfortably inside the **3 s spine-side
objection window** that defines commit for A2 writs. **Commit = governor-accept,
tick-stamped (D-013), and for voice-sourced numeric writs the governor accepts only when
the objection window closes** — the before-commit guarantee is structural, not raced.
The generated, cited elaboration follows the templated line at the Speaker's leisure.
**The demo story, stated honestly (D-018): the mind saves the plasma *via the sovereign* —
on the lethal-legal class, where the governor alone would not have refused — and the tape
receipts the whole chain: partial → fork → warning → belay (or override).**

---

## §6 · Telemetry & renderers — pure observers (booster law)

One-way binary stream: scalars (Q, I_p, n̄, β_N, κ, coil currents, controller margins,
innovation norms, γ from the eigenproblem) at 60 Hz; the **play-mode field stream is
tier-1-derived emissivity at 64³ fp16, 10 Hz ≈ 42 MB/s over shared memory** (V-07: the
renderer may drop frames and may never backpressure the plant; device→host copies on a
dedicated async stream; 128³ exists only in headless capture mode). Events piggybacked.
**Delete `ui/` and `fusor` still runs, still holds the plasma, still prints its
Monte-Carlo rates.**

- **Client #1 — three.js/WebGPU (Tauri shell; sanctioned `ui/`-only exception):**
  volumetric raymarch of the emissivity field, instanced-tube field lines from the GS
  ψ-surfaces (q-profile visible as winding pitch), divertor strike glow, the Q meter, coil
  HUD, innovation sparklines, the mind's captions + waveform, **the ghost overlay — drawn
  from the fork's tier-0/1 trajectory as a displaced translucent boundary + dimming core
  (defined path: boundary from the linear model's Z(t), emissivity from the frozen
  profile — FE-19)**, and **the toggle**: cerebellum OFF → watch the VDE take it in
  **~150–300 ms** (derived from machine.toml γ; PHY-17 — slow enough to watch, which is
  better television than v0.1's impossible 50 ms), spine gates still armed and honestly
  unable to save it (`spine_gates.toml [demo_interaction]`).
- **Client #2 — UE5, later, same stream** (parked).

---

## §7 · Hard budgets (RTX 4070 Ti SUPER 16 GB + consumer CPU — play mode)

**The shipping config is the ONLY config §7 claims (V-08):** ASR on CPU, TTS on CPU,
policy on CPU, tier-2 per D-020's pending ruling (rows below assume the recommended cut),
lean desktop.

### 7.1 VRAM (GiB)

| component | VRAM | note |
|---|---|---|
| Qwen3.5-9B Q5_K_M + 32k KV + compute | 7.3–7.6 | verified vs auricle receipts (32 KiB/tok hybrid-SSM KV) |
| plant tier 0/1 (GS grids, Green's tables, circuits) | ≤ 0.3 | policy is CPU (D-012) |
| ghost fork working set | ≤ 0.1 | one concurrent ghost v1 |
| telemetry staging (64³ emissivity, double-buffered) | ≤ 0.1 | |
| renderer (separate process) + desktop | ~2.0 (lean) + **desktop ≈ 4.6 measured — plain desktop, no renderer (provenance per `SESSION_HANDOFF.md:86`; the WebGPU client is an *estimate* +0.5–1.5 until measured — V-08/D3)** | |
| **prescribed total** | **≈ 11.4, fits with headroom** | worst-case stack ≈ 16.2 does NOT fit and is refused |

### 7.2 The GPU arbitration doctrine (V-02; D-027)

One plant-owned CUDA context at greatest stream priority (GS + telemetry staging); the LLM
decodes in its own context and **is the preemptible party**; ASR/TTS/policy never touch
CUDA. The GS pipeline is deterministic-by-schedule (§2.1) — contention costs pacing, never
correctness, and a `gs_late` event prices it. **Tick-jitter (p99.9 tick lateness with the
LLM hot) is measured at the M3 gate**, not M5.

### 7.3 CPU cores (consumer 8P+E class; FRESH-7)

| consumer | affinity |
|---|---|
| spine spin-loop (integrator + gates + governor) | P-core, pinned, isolated |
| policy + EKF (inside the tick) | same P-core (they are the tick) |
| ASR worker | P-core |
| TTS synth + duplex governor | P-core |
| llama.cpp host threads | 2–3 P/E |
| telemetry, tape, OS | E-cores |
Degradation order under pressure: renderer fps → TTS quality → LLM tok/s → **never the tick**.

### 7.4 The 100 µs tick, budgeted (CTL-27)

linear vertical/circuit step 10 µs · burn sub-cycle amortized 8 · diagnostics synthesis 8 ·
EKF predict/update 15 · policy 25 · gates + governor 5 · telemetry ring write 4 · slack 25.
**p99.9 is the binding statistic, measured with the LLM hot (M3 gate).**

### 7.5 Latency chains (measured rigs named in §8 F-VOICE)

| chain | budget | notes |
|---|---|---|
| A: barge-in mic→pause | ≤ 150 ms | capture 24 + VAD 32–96 + hop 5 + fade 10 + queue ≤ 60 (capped) |
| B: ASR partial | ≤ 300 ms | 160 ms chunk + CPU decode ~42 ms (3.8× RT) + hops |
| C: last word → first phoneme | ≤ 1.5 s p95 | endpoint 350 + commit 20 + boundary+probe ~400 + clause-grain TTS ≤ 300 |
| D: ghost warning audible | ≤ 0.5 s from fork verdict | templated line + pre-warmed TTS; window 3 s |
| telemetry field stream | 42 MB/s | 64³ @ 10 Hz shared-memory |

All numbers are targets until a `runs/` receipt says otherwise — **measured-not-claimed
binds from M0**, and derived figures carry their arithmetic (D-018).

---

## §8 · Falsifiers (pre-registered; each can lose; grader laws named)

- **F-BURN-0 (M0 gate).** On `easy.toml` (template-conforming, seeded, dispersed): Q ≥ 1.0
  held ≥ 10 s sim through the pre-registered impurity-puff disturbance, in ≥ 90% of
  N = 1000 gate-domain seeds (Wilson 95% lower bound ≥ 0.88), PID null CEM-tuned with
  receipts, memcmp replay green. *Fails if:* FUSOR-1's arithmetic was wrong, the burn
  model is broken, or the gate seeds leak into tuning (seedcheck).
- **F-SHAPE-1 (M1 gate).** The un-stabilized plant (VS loop off) **demonstrably disrupts
  via VDE → quench** (§2.4 physics firing, tokens on tape); the offline oracle lands
  flat-top through the full disturbance set **including vde_kick and hl_backtransition**
  in ≥ 95% of seeds; the EKF passes its NIS χ² acceptance; determinism suite green at
  tier-1 (including the GS pipeline schedule).
- **F-KEEPUP-F** — play mode holds **aggregate pacing** (sim time never lags wall by
  > 50 ms over any 1 s window; zero ticks are ever dropped — the sim waits and receipts
  `gs_late`) for a full session; LLM/ASR/TTS lateness only ever sheds judgments/speech as
  drop-events. The Deadline Law, made killable.
- **F-NULL-C (M2 gate).** The policy vs the **shipped in-binary null** (PID+LQ, CEM-tuned,
  same objective file): compared under **matched wall-clock and matched VRAM on the same
  tick budget**, common random numbers, pre-registered scenario × gate-seed grid
  (code-enforced disjoint from training), **three reported rates (disrupt / spine-shutdown
  / good) + tracking RMS**, Wilson CIs on rates, paired-bootstrap on RMS, lexicographic
  decision rule (`objective.toml [stats]`). Published either way; a null win completes the
  milestone and rung (a) of the ladder ships.
- **F-GHOST (M3 gate, rescoped — D-026).** (i) ghost-vs-ghost memcmp determinism;
  (ii) bounded divergence vs the plant (|ΔZ| < 2 cm, |ΔQ| < 0.5 at 500 ms lookahead) on
  the validation set; (iii) every Sentinel warning demonstrably precedes governor-accept
  on the tape. If the declared approximation drifts past its bounds, the feature dies.
- **F-VOICE (M4 gate).** Chains A and C measured on the named rig (loopback capture of our
  own render + QPC alignment; t₀ = acoustic onset in the capture stream, t₁ = last audible
  sample after the cut), reported as distributions (p50/p95/max, N ≥ 100), device named in
  the receipt, wired headset; **speakers-mode negative test included** (the failure is a
  receipt, not a surprise); voiced-vs-drafted ledger complete with sample-accurate cuts;
  zero un-receipted audio (unit of account: utterances × samples).
- **F-VERACITY (M4 gate, new — FRESH-2/D-018).** A pre-registered battery of 30 planted
  factual questions (10 current-state, 10 tape-historical, 10 why-did-the-governor-refuse)
  answered aloud and **graded by code against the tape** (mechanical grader — TinyVillage
  L5's "no model grades anything" satisfied by construction): ≥ 27/30 correct, and a
  50-utterance citation-fidelity audit with **zero fabricated rev citations**. Solicited
  speech does not ship while this fails.
- **F-PRESENCE-F (M5 gate, three arms — D-018).** Over scripted operator sessions with
  planted commands (≥ 5 from the lethal-legal class): **resident triple vs event-triggered
  turn-based twin vs polling twin**, all three fed the identical token stream and floors;
  only the resident arm has partials + cross-event KV memory — that difference is the
  claim. Blind-graded by the **operator as a calibrated instrument (TinyVillage L12:
  curation and A/B streams disjoint, 10% blind re-presentations, self-consistency floor
  90% or the batch is invalid; no model grades anything)** on usefulness, timing, and
  pre-commit standing. The estate's core bet, staged where every interjection carries a
  mechanical verifier.
- **F-INSTINCT (inherited; eval set now scheduled — M-B9).** M3's gate **produces the
  labeled ledger** (the ported auricle `--review` one-key valve) and quarantines a 40-item
  held-out set at `contracts/eval/finstinct_holdout.jsonl` (the trainer hard-refuses the
  path); the emit judgment must survive dial-at-zero on it before unsolicited speech
  leaves shadow.

---

## §9 · Milestones (each gate = receipts in `runs/`, booster-style)

- **M0 — the burn.** Tier-0 per §2.1 on FUSOR-1 + PID null (CEM-tuned, receipted) +
  `fusor_mc` + memcmp oracle + committed goldens. *Gate:* **F-BURN-0**.
- **M0.5 — the thesis slice (shadow; D-019).** Fusion's shadow soak: the tier-0 plant +
  `events.toml` tokenizer + floors/governor + the text-only triple — no GS, no voice — so
  the three-arm F-PRESENCE-F harness and F-VERACITY's tape-grader exist and dry-run on
  scripted events before the M1 physics grind. *Gate:* shadow ledger + both nulls wired
  (tokenizer-vs-threshold-logger, triple-vs-both-twins) + dry-run report. No gate claims —
  the rig exists so the bet meets evidence early.
- **M1 — the shape and the enemy.** Tier-1 per §2.1 (GS + passive conductors + current
  profile + 1-D transport + diagnostics + EKF with NIS acceptance) + §2.4 disruption
  model + scenario set incl. lethal-legal class + offline oracle. *Gate:* **F-SHAPE-1**.
- **M2 — the cerebellum.** The ladder (§3), batched envs, export + KAT + latency p99.9.
  *Gate:* **F-NULL-C**.
- **M3 — the membrane (shadow).** Warpbus + tokenizer v1 + durable tape + the triple in
  log-only soak on the M0.5 rig at tier-1 fidelity + ghost-forks live + `--review` label
  valve + F-INSTINCT's held-out set quarantined + tick-jitter measured. *Gate:*
  **F-GHOST** + shadow receipts + fire-rate/load report (M-M7's duty-cycle ceiling).
- **M4 — the voice.** sherpa/Nemotron in (chunk-ladder WER receipt), Piper out via
  `tts.h`, duplex + barge-in, voiced-vs-drafted ledger. *Gate:* **F-VOICE + F-VERACITY**,
  solicited speech only.
- **M5 — the talking reactor.** Writs at A0–A2 live, etiquette caps, the toggle demo
  (~150–300 ms VDE, spine gates armed), unsolicited initiative iff F-INSTINCT passed,
  **F-KEEPUP-F + F-PRESENCE-F** scripted soak, the 90-second video. *Gate:* the video +
  the published three-arm ledger, wins and losses both.

**Parked (post-M5):** UE5; tier-2 MHD (per D-020's operator ruling); MCTS scenario
planning + sysid (named nulls required); the emit-QLoRA; multi-reactor rooms.

---

## §10 · Repo layout & build

```
C:/fusion/
  core/         plant tiers 0/1, integrator, §2.4 disruption, diagnostics, fork/ghost, spine gates
  control/      policy runtime (flat-weight mmap), CEM/MPPI teacher (offline), PID+LQ null (ships)
  membrane/     warpbus glue: tokenizer, writ translator, triple host, durable tape, --review valve
  voice/        asr.h (sherpa backend, two model dirs), tts.h (piper), duplex governor, VAD
  trainer/      Python rig (CEM/ladder/export_weights.py) — dev only, never in product
  telemetry/    binary protocol, SPSC rings, shared-memory field stream
  ui/           Tauri + three.js/WebGPU client (pure observer; the sanctioned exception)
  contracts/    machine, objective, dispersions, diagnostics, events, floors, spine_gates,
                writs, scenarios/ (+ eval/ holdouts, quarantined)
  third_party/  vendored + pinned (M0 opening move): tomlplusplus v3.4 (header-only), later
                sherpa-onnx/piper/llama.cpp per milestone; sha256 in third_party/README.md;
                third_party/dl/ gitignored
  goldens/      seeded bit-exact trajectories (binary per .gitattributes)
  runs/         receipts + session tapes (gitignored; snapshotted)   tools/  dev scripts (PS)
  docs/qc/      the six audit instruments + DISPOSITIONS.md (read-only records)
```
CMake ≥ 3.26, generator `"Visual Studio 17 2022" -A x64`, C++20, CUDA optional until M1;
targets `fusor` / `fusor_mc` / `fusor_train_env` / ctests (`replay_oracle`, `statecheck`,
`seedcheck`, `parity_train_env`, `budget_events`). Build/run/ctest command block lives in
`KICKOFF_M0.md` and `CLAUDE.md`. **C/C++20 + CUDA only in the product loop; Python in
`trainer/` only; JS/TS/Rust in `ui/` only (out-of-process observer); no package managers
in the product build (`ui/` may use npm/cargo — it is deletable).**

## §11 · Doctrine inheritance — now with teeth (the "enforced by" column; M-M8)

| law | source | lands here as | **enforced by** |
|---|---|---|---|
| state only through integrator; no assist; one dynamics source; deterministic+memcmp; headless MC | Booster | §2.2 | `replay_oracle` ctest; `parity_train_env`; SPINE_SHUTDOWN failure class; input-tape replay |
| teacher-then-student with the honest wall respected; DAgger; export-to-C | Booster LODESTAR | §3 M2 ladder | pre-registered rungs; KAT + byte-golden on export; seedcheck |
| anti-turn membrane (at its D-018 width); two planes; segmenter; molt; A3 role-headers | SYNCYTIUM/auricle (measured) | §4 | structural plant-lane boundaries; cascade breaker in events.toml; branch-teardown rule |
| Deadline Law (fixed budget + abort + effective-tick) | TinyVillage L2 | §4.3, writs.toml | sampler abort + `[sys] drop`; governor staleness gate |
| label-factory fence (obs never truth) | TinyVillage L6 | §3 | **`statecheck` ctest (hard M1 gate)** + grep gate |
| rater is an instrument; no model grades | TinyVillage L12/L5 | §8 F-PRESENCE-F/F-VERACITY | disjoint streams, blind re-presentations, consistency floor; code grader |
| null ships in-binary; matched wall-clock+VRAM | TinyVillage L11/L5 | §3, §8 F-NULL-C | runtime-selectable null arm; M5 toggle |
| vocabulary freeze | TinyVillage L3 | §4.1 | events.toml frozen at M3, append-only |
| receipts non-mintable; acceptance receipted | TinyVillage L5 | §4.2 | governor echo on `sys`; F-VERACITY citation audit |
| innovation-gated tokenization; writs-not-actuators | this spec | §4.1–4.2 | tokenizer null; floors + spine_gates tables |

## §12 · Honest limits, up front

Physics: toy geometry on a synthetic machine; prescribed transport calibrated to a scaling
law; **breakdown/burn-through out of scope; halo currents and runaways out of scope;**
tier-2 pending an operator ruling; never claim machine-design relevance — the *control
problem's* difficulty class is real, the numbers are scaling-law grade, and **H98 is a
knob**: the burn game's difficulty is chosen, stated, and pinned in machine.toml. ASR: the
sherpa/Nemotron path is verified upstream but **unmeasured on this box** — chunk-ladder
WER and latency are M4 receipts, not assumptions. Membrane: the 9B judgment ceiling,
over-fire (921/hr full-day measured), and stance confusion are inherited *measured*
problems — unsolicited speech stays shadow until F-INSTINCT passes on the now-scheduled
held-out set; the surprisal tap is logged but **unvalidated as a signal**; free
resume-after-barge-in is v2. The renderer's VRAM row is partly estimate until measured
(V-08). The three-arm F-PRESENCE-F result may show the event-triggered twin closes most of
the gap — **that outcome is a legitimate published result, not a failure** (D-018: it
would locate the anti-turn advantage precisely where the whitepaper claims it lives, and
nowhere else). And the whole spec remains pre-build: every §7 number is a target until a
receipt in `runs/` says otherwise.

---

*v0.2, 2026-08-09 — the QC revision: five specialist lanes + one zero-context stranger +
the receipts themselves, all folded; the machine is pinned, the claim is falsifiable, and
the sequencing puts the bet before the grind. Rent the intelligence, own the loop — and
say only what the tape can back.*
