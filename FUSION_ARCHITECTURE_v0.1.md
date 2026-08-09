# FUSION — the reactor you can talk to
### Architecture / design spec v0.1 · 2026-08-09 · Bo Chen · Access Intellect LLC

**Status: SPEC, pre-build.** Core binary: **`fusor`**. Root: `C:/fusion/`.
**Lineage (three proven parents, one law each):** the **Booster Lander Simulator**'s
constitution (engineering-sim-first, renderer-as-pure-observer, hard-to-fake by construction),
the **auricle/SYNCYTIUM** membrane (the resident anti-turn LLM triple on the two-plane warpbus,
measured through M4.5b), and **TinyVillage**'s laws (the Deadline Law, tokenized experience,
the label factory, rungs ship only by beating their null). This document is the introduction
of the three to each other.

> **QC STATUS (2026-08-09, same day):** a five-auditor Opus swarm pass (Intercom room
> `fusion-qc`) logged **146 findings, 39 blockers** — see `docs/qc/QC_CONSOLIDATED.md` and
> the five lane reports. Consensus corrections are applied in-place below (D-011…D-017);
> the remaining blockers are design work mandated for **v0.2** (`KICKOFF_SPEC_V02.md`).
> **M0 is suspended until v0.2 exists — do not build against this revision.**

> **One breath:** a tokamak simulation with real (tiered, honest) physics, flown at kHz by a
> trained neural controller that must actually hold the plasma or it disrupts — watched,
> remembered, and **voiced** by a resident LLM triple on the warpbus, with **full-duplex
> speech**: you talk to the reactor through a mic; the reactor talks back, interrupts you when
> your forming command would kill it, and shows you the ghost of the disruption it just
> simulated to prove it. Every claim carries a receipt. Every organ has a null.

---

## §0 · The claim, and what is honestly being claimed

The claim is **not** a predictive fusion code. Tier-0/1 physics below use the real published
formulas on toy geometry; tier-2 is eye-candy-grade MHD (real equations, coarse grid). The
claim **is**: (a) a control problem hard enough that continuous kHz-class feedback is
*mandatory* (an elongated plasma is vertically unstable — growth times are **wall-set,
~10–100 ms class with a passive conducting structure, not κ-set**; QC corrected v0.1's
1–5 ms here — and without feedback it is lost; whether a *neural* controller beats a
well-tuned classical one is not asserted — it is exactly what F-NULL-C measures), (b) an
architecture in which a fast non-linguistic cerebellum and a
resident linguistic cortex share one machine honestly, and (c) a demonstration that "own the
loop" produces a machine that can **sense, remember, and speak** about its own life with
receipts. The DeepMind/EPFL TCV result (Degrave et al., *Nature* 2022 — deep-RL driving 19
poloidal-field coils at 10 kHz on a real tokamak) is the existence proof that the control
half is real, not fiction.

---

## §1 · The stack — four tiers, two seams

```
TIER            GRAIN        SUBSTANCE                        LAW
─────────────────────────────────────────────────────────────────────────────
spine           100 µs       deterministic C++/CUDA plant     state only through the
                             integrator + safety gates          integrator; gates close
cerebellum      100 µs–1 ms  policy net + MPPI (non-          if it can't hold the
                             linguistic, trained on error)      plasma, it disrupts
cortex          ~1–3 s       resident LLM triple on the       anti-turn; boundary-gated
                             warpbus (Speaker/Skeptic/          {hold,emit}; molt;
                             Sentinel), full-duplex voice       dial→weights (QLoRA)
depths          minutes      optional turn-based consults     turn-based is correct here
─────────────────────────────────────────────────────────────────────────────
SEAM ↑ (plant→cortex):  INNOVATION-GATED TOKENIZATION — the plant earns tokens
                        only by surprising its own cerebellum (Kalman innovation =
                        the surprise channel; nominal = silence/ticks = L-SILENCE)
SEAM ↓ (cortex→plant):  WRITS, NEVER ACTUATORS — typed setpoint/mode/envelope revs,
                        translated + gated by deterministic floors (the governor)
BOTH SEAMS:             THE DEADLINE LAW — the world never dilates for a model; a
                        late judgment is a dropped judgment, recorded as an event
                        the mind later perceives
```

---

## §2 · The plant — `fusor` core (C++20 + CUDA, headless-first)

### 2.1 Physics ladder (real formulas, honest tiers)

- **Tier 0 — the burn (0-D power balance).** D-T reactivity ⟨σv⟩(T) via the **Bosch-Hale
  parameterization**; alpha self-heating P_α = n²/4 ⟨σv⟩ E_α V; **Bremsstrahlung**
  P_br ∝ n²√T Z_eff; synchrotron losses (Trubnikov fit); energy confinement via
  **IPB98(y,2)** scaling τ_E(I_p, B_T, n̄, P, R, κ, ε, M); Lawson accounting → live
  **Q = P_fus / P_aux**. Runs at 1 ms cadence on CPU. This alone is the sustain-Q game.
- **Tier 1 — the shape (2-D, control-grade).** Free-boundary **Grad-Shafranov** equilibrium
  solver (FreeGS-style Picard iteration, ported to CUDA; grid 65×65 → 129×129), PF coil
  circuit equations (L-R dynamics, ~8–12 coils v1), **vertical position instability**
  (γ⁻¹ **wall-set, 10–100 ms class — which requires modeling the passive conducting
  structure** (a vessel L-R circuit; without it the mode is ideal-fast and uncontrollable;
  QC corrected v0.1's "1–5 ms at κ≈1.7" — the controller's true opponent), 1-D radial
  transport (n(ρ), T(ρ), 20–40 radial nodes, prescribed diffusivities + sawtooth crash
  model). This is the tier the controller flies against — the same shape real machines'
  control systems use. **Multi-rate law (QC fix):** the full GS equilibrium re-solves at
  **100–200 Hz**; between solves, a **linearized rigid-displacement model carries the
  vertical/position dynamics at 10 kHz**. Full GS at 10 kHz is neither feasible nor how
  real control stacks work; the linearization is re-derived at each equilibrium solve.
- **Tier 2 — the spectacle (3-D resistive MHD, renderer-only).** Coarse finite-volume MHD,
  **128³ fp16 fields** (ρ, p, v⃗, B⃗ ≈ 8 fields ≈ 0.25–0.5 GB), CUDA. Kinks, tearing,
  ELM-like edge filaments — qualitatively real, quantitatively decorative. **Feeds the
  renderer, never the controller.**

### 2.2 The booster constitution, inherited verbatim

1. **State changes only through the integrator.** Controller output = actuator vector
   (coil voltages, gas/pellet commands, heating powers). Nothing else writes state.
2. **No assist term.** If the controller can't hold it: VDE → current quench → run over.
   Disruption, tip-over equivalent, and quench are fully simulated valid outcomes.
3. **One dynamics source.** Plant, MPPI rollouts, and ghost-forks call the same
   `__host__ __device__` equations of motion, including actuator lag (coil L/R, valve delay).
4. **Deterministic, provably.** Philox4x32-10 counter RNG; compile-time **dt = 100 µs**;
   no wall-clock in the sim path; no unordered FP reductions. Same seed + scenario ⇒
   bit-identical trajectory, enforced by a **memcmp replay oracle** in the self-test.
5. **Headless must work.** `fusor mc --n 1000` runs seeded scenarios and prints
   sustained-Q rate, disruption rate, and tracking RMS with **Wilson 95% CIs**. That
   number is the gate for every controller change.
6. **Fork = physics-space `seq_cp`.** `fusor_fork(state_h) → handle` snapshots full plant
   state (tier 0/1 ≈ <1 MB); a fork runs K ms of lookahead at ≥20× real-time (tier 0/1
   only) and is discarded. This is the **ghost** primitive (§5.4).

### 2.3 Scenario set (the curriculum and the demo script)

`breakdown → current ramp → flat-top → burn` plus disturbances: sawtooth crash, ELM burst,
impurity puff (radiative collapse threat), density-limit approach (Greenwald fraction),
pellet, NBI trip, **single-coil failure** (the engine-out of fusion), and combined cases.
Each scenario = seed + config TOML; goldens in `goldens/`.

---

## §3 · The cerebellum — control nets (booster doctrine, fusion plant)

- **Observations (synthetic diagnostics only — never true state):** ~40 magnetic probes +
  flux loops (with noise + drift), interferometer line densities, Dα, neutron rate, coil
  currents/temps. The observer problem is part of the problem.
- **Actions:** PF coil voltages (8–12), gas puff rate, pellet trigger, NBI/ECRH power,
  (v2: current drive). Rate-limited, saturating, lagged — like real actuators.
- **Policy:** MLP (Degrave et al. ran a 3-hidden-layer MLP at 10 kHz on TCV), 2–4 ×
  256–512 hidden, **on CPU at fixed precision** (D-012: a fp16/int8 CUDA policy feeding
  the integrator breaks the memcmp oracle, and the parent's proven path is a ~10 µs C
  net on CPU — which also removes a CUDA consumer from the 100 µs tick). **< 50 µs
  inference as a CPU budget**; weights mmap'd from a flat file
  (`trainer/export_weights.py`). No Python in the product loop.
- **Training = oracle-guided, with the parent's measured null respected (QC correction):**
  the booster's own LODESTAR record is titled *"the honest wall"* — end-to-end
  distillation of the compound case plateaued at 0/16 (null D-041); what shipped green
  was a **θ-predictor over an analytic base law**. So: CEM/MPPI offline oracle proves
  each scenario solvable (unchanged — that part is the parent's proven half) → the M2
  session **decides the student's architecture against that record** (θ-style structured
  policy vs direct distill vs pure sim-RL à la TCV/MPO — Degrave et al. used *no*
  distillation) with a pre-registered losing branch. PPO/MPO polish on **batched CUDA
  envs (2048–4096 parallel reactors)** — the training envs are the **fourth consumer of
  the one-dynamics-source law** (D-011), and planner/teacher rollouts must decorrelate
  their disturbance streams from the plant's Philox counters (the clairvoyance trap)
  while ghosts deliberately share them (that sharing *is* F-GHOST).
- **The innovation source (named organ — QC fix):** a lightweight **EKF / one-step learned
  forward model runs beside the policy at all times** as the standing prediction the
  tokenizer's innovation is computed against (a distilled policy has no native state
  prediction; MPPI's rollout error covers only the moments the planner is active).
- **The null:** a tuned PID + linear-quadratic vertical controller and a textbook MPC.
  **The net ships only by beating the null** on disruption rate + tracking at matched
  compute (Wilson CI, published either way). No organ outlives its null.
- **MCTS lives one level up:** discrete scenario planning (setpoint/heating/fueling
  schedules through the campaign), AlphaZero-shaped over the fast policy/value; and
  **sysid-by-backprop**: tier 0/1 are small ODE/PDE systems — make them differentiable,
  gradient-fit free coefficients to the published scalings. The Verifier Law licenses all
  of it: this domain is a closed board with a mechanical verifier.

---

## §4 · The membrane — the resident triple on the warpbus

**Engine:** the auricle stack as measured — Qwen3.5-9B Q5_K_M on one `kv_unified` 32k trunk
(assert `n_ctx_seq == n_ctx`), three seats via branch-time role headers (A3-safe), arm-B
worked-example seed, **isomorphic segmenter** + boundary-gated `{hold,emit}` (dial → QLoRA
per the emit runbook), **molt** at watermark 24576 (scribe rung ≤ 600 tok; outage as a
drop-event), **surprisal tap** on every boundary. Reuse: `cortex_llama.cpp` / `soak.cpp`
machinery + `fabric.h` two-plane bus. All measured numbers inherited from `C:/auricle/runs/`.

**Seats (mandates are dials — M4.5's lesson: vague mandates over-fire):**
- **SPEAKER** — the reactor's voice: answers, reports, debriefs. Speaks *as the machine*,
  first person, every claim citing a bus rev.
- **SKEPTIC** — contests: the controller's health claims, the Speaker's diagnoses, and the
  operator's assertions, against the record.
- **SENTINEL** — watches the operator's *forming* commands (lane-H partials) + plant lane;
  fires **ghost-forks** (§5.4) and interjects **before commit** with the receipt.

**Bus lanes:** `plant` (innovation-gated events), `opv` (operator voice: ASR partials →
reflex plane, finals → trunk), `opt` (operator typed), `spk/skp/sen` (the room), `writ`
(downward references), `sys` (drop-events, molts, gate refusals).

### 4.1 The tokenizer of machine experience (seam ↑) — first event vocabulary

The plant speaks only when reality diverges from its cerebellum's model. **Innovation** =
sensor minus prediction (the Kalman residual the controller already computes; MPPI rollout
error where no filter exists — the free-tail razor applied to the cerebellum).

```
[plant] mode RAMP->FLAT_TOP                     mode transitions
[plant] innov mag=4.2s cluster=vertical         innovation spike (σ units, sensor cluster)
[plant] limit greenwald f=0.92 rising           proximity to named limits (q95, beta_N,
[plant] ctrl sat coil=PF3 0.8s                    density); controller saturation
[plant] ctrl drop tick=... (deadline)           the cerebellum's own drop-events
[plant] health nbi2 drift 1.4sigma/6min         slow-organ health
[tick +30s nominal Q=8.1]                       silence, priced: the world as anticipated
```

**Budget law (single source of truth: `contracts/events.toml [budget]` — QC fixed the
three-way contradiction here):** nominal hour ≤ 150 tokens (5-min ticks ≈ 96/hr + slack);
crisis second ≤ 40 tokens, with storms collapsing to summary events. Adaptive
10³–10⁴ : 1 down-conversion, done *by the plant's own surprise* — no scheduler, no polling
summarizer. Thresholds live in one table (`contracts/events.toml`), pre-registered.

### 4.2 The writ grammar (seam ↓) — and the governor

```
writ setpoint Ip 12.5MA ramp=0.1MA/s        writ mode RECOVERY
writ envelope greenwald max=0.85            writ priority divertor>Q horizon=60s
```
Typed revs on the `writ` lane. A deterministic translator compiles them to reference
schedules **through hard floors the mind cannot cross** (the governor: q95 ≥ 2.2, Greenwald
f ≤ 0.95 — `contracts/floors.toml` is the single source of truth; QC caught the 1.0 here —
coil current/temp limits, ramp-rate limits). A writ past a floor is **refused, and
the refusal is a receipt** on the tape. The LLM steers like an executive; it can never grab
the stick. Harm dial: H0 log-only (shadow) → H1 auto-apply within envelopes → H2 apply
after cancelable delay → H3 operator-confirm. **v1 ships at H0/H1.**

### 4.3 The Deadline Law at the seam

The triple is a **guest organ** with a **fixed budget, not a percentile** (QC: TinyVillage
L2 demands a fixed tick budget declared at design time plus an abort — a p95 has neither;
and no estate receipt reports a p95 anyway, only means 327/349 ms and maxima 2.1/5.5 s):
**judgment budget = 2 s hard; at 2 s the branch is aborted**, the judgment is dropped, and
`[sys] drop llm ...` enters the trunk — the mind can later say "I was slow during the ELM
burst," and mean it, with a receipt. Every writ carries the **tick its evidence was formed
at** (`formed_tick`) and the governor refuses writs whose staleness exceeds the writ
class's window — TinyVillage L2's `effective_tick` half, restored (D-015). The 100 µs
world never waits.

---

## §5 · The voice loop — full-duplex speech (the point of the whole thing)

### 5.1 IN: mic → streaming ASR → the trunk (two-plane law)

- **Capture:** WASAPI (reuse `auricle/src/capture/` + resampler, proven). 16 kHz mono.
- **ASR, primary option — NVIDIA Nemotron 3.5 ASR streaming 0.6B**
  (`huggingface.co/nvidia/nemotron-3.5-asr-streaming-0.6b`): cache-aware
  FastConformer-RNNT, 40 language-locales, native punctuation + capitalization,
  **selectable chunk 80 / 160 / 320 / 560 / 1120 ms** — run at **160 ms** chunks for felt
  latency (vendor-reported H100 throughput 240–2400 concurrent streams; we need 1–2).
  Integration path: NeMo → ONNX/TensorRT export (or Riva container) → in-process C++
  runtime; **fp16/int8 ≈ 0.7–1.2 GB VRAM** (or CPU execution if export allows — measure).
  **Why it earns the slot:** native punctuation feeds the segmenter real clause marks, and
  the **fine-tune path** (`huggingface.co/blog/nvidia/fine-tuning-nemotron-35-asr`) lets us
  domain-tune the vocabulary — *Greenwald, q95, ELM, divertor, gyrotron, beta_N* — the same
  Unsloth-era muscle the estate already has, pointed at ASR.
- **ASR unification (QC finding V-04 — the risk was overstated):** sherpa-onnx **already
  supports Nemotron 3.5 streaming** (upstream PRs #3044/#3671, pre-exported ONNX, ~3.8×
  real-time on CPU int8) — so "primary" and "fallback" are the **same backend**: the
  proven, already-vendored auricle sherpa stack, version-bumped and sha256 re-pinned.
  M4's ASR task is a pin bump, not an export project; running ASR **on CPU** (auricle's
  recorded rule) also removes a CUDA consumer and ~1 GB of VRAM. The zipformer +
  Parakeet-TDT truth lane remains the in-tree alternative behind the same `asr.h` seam.
- **Two-plane discipline (verbatim from SYNCYTIUM):** partials → reflex plane only
  (perception; revocable; the Sentinel may act on a *forming* command); finals → trunk
  commits on lane `opv`. Endpoint ~0.35 s. A backspaced word never poisons the trunk.

### 5.2 OUT: the Speaker → streaming TTS → abortable playback

- **TTS options:** **Piper** (ONNX, local, faster-than-real-time on CPU, ~50–100 MB — v1
  default), Kokoro-82M (quality bump), NVIDIA TTS (if the Riva path is already up for ASR).
  Requirement: **streaming synthesis, first audio < 300 ms** after emit commit.
- **Abortable at word grain — the demesne pattern applied to audio out:** barge-in fires on
  **VAD (~20–30 ms energy/Silero detect) → playback soft-pause ≤ 150 ms end-to-end**; the
  ASR partial *attributes* the speech afterward (QC fix: at 160 ms chunks the first ASR
  partial cannot arrive inside the 150 ms budget — VAD is the trigger, ASR the witness).
  The ledger
  records **voiced-vs-drafted** — what was actually said aloud versus what the mind
  committed — a first-class receipt (the estate's composition-public law, honest at the
  speaker grille).
- **Etiquette (M4.5's measured lesson):** per-seat emission rate caps + the interruption
  budget; dial-zero over-fire (593/hr measured on text) is *unlivable* out loud — the
  voice loop splits by standing (QC fix): the measured dial-zero over-fire is **921
  emits/hr on the only full-day run** (auricle run C, molt v1; v0.1's 593/hr was run A,
  which covered only ~55–62% of the day — QC-MEMBRANE caught the denominator); **solicited
  speech** — answers to the operator's
  questions and commands — is reactive, safe, and **ships at M4**; **unsolicited
  initiative** — interjections, warnings, reach-outs — stays shadow (H0, brief-only) until
  **F-INSTINCT** passes. A reactor that answers when asked is a tool; one that interrupts
  well is the bet, and the bet waits for its gate.

### 5.2b Acoustic law (QC blocker V-B1 + MEM, independently — the mic hears the TTS)

**Headset required in v1** (auricle's recorded precedent: deterministic me/them attribution
beats AEC complexity; AEC is a v2 node, not a v1 gate). Without it, VAD-triggered barge-in
self-pauses the reactor on its own first word, and worse: the reactor's own speech
transcribes onto lane `opv` and enters the trunk *as the operator*. Additionally, the TTS
output is lane-stamped and its transcript **never enters `opv`** regardless of acoustics —
self-echo suppression at the lane, not the microphone (D-014).

### 5.3 Duplex rules

Both sides may initiate; neither waits for a turn. Operator barge-in pauses the reactor
mid-word (soft-pause → resume-or-abandon, model's choice at the next boundary). The reactor
may interject during operator speech **only** through the boundary-gated emit with the
etiquette caps — and every interjection cites its rev or its ghost.

### 5.4 The ghost (the killer feature, and it's nearly free)

The Sentinel, on a forming dangerous command (partial, pre-commit): `fusor_fork` → apply
the command → run 300–500 ms of tier-0/1 physics at ≥20× real-time (< 25 ms wall) →
if disruption: interject with the fork's trajectory streamed to the renderer as a
**translucent ghost overlay** of the plasma dying. **The hard guarantee is
before-commit; mid-sentence is the typical case** — the ghost can only fire once the
command's parameter has landed in the partial, which usually leaves trailing words and
always leaves the endpoint (QC fix: the guarantee is stated at the commit boundary, not
the sentence). The warning is not an opinion; it is a receipt from the same physics,
pre-verified (the Verifier Law's instant horizon).

**Commit, defined once for the whole repo (D-013 — three QC lanes converged on it):** a
command **commits when the governor accepts it, tick-stamped** — not when ASR finalizes,
not when the operator stops talking. And because the warning pipeline (probe ~65–70 ms +
generation ~1–2 s + TTS start) cannot beat an ASR-final at H1 speeds (QC blocker V-B3),
**numeric setpoint writs from voice run at H2: governor-accept opens a spine-side objection
window (default 3 s, config) with a readback**, during which the Sentinel's ghost — or the
operator's "belay" — cancels cleanly. The before-commit guarantee is thereby *engineered*
rather than raced. If the operator proceeds anyway (H-dial permitting), the fork-vs-reality
memcmp is itself a falsifier run (§8 F-GHOST).

---

## §6 · Telemetry & renderers — pure observers (booster law)

One-way binary stream (lock-free ring → socket): scalars (Q, I_p, n̄, β_N, κ, coil currents,
controller margins, innovation norms) at 60 Hz; tier-2 fields as **fp16 3-D textures
(64³–128³) at 10–30 Hz**; events piggybacked. **Delete `ui/` and `fusor` still runs, still
holds the plasma, still prints its Monte-Carlo rates.**

- **Client #1 — three.js/WebGPU (Tauri shell, the booster pattern):** volumetric raymarch of
  the emissivity field (T,n → physically-inspired palette: white-hot core through violet to
  the Balmer-pink edge), instanced-tube field lines (RK4 traces, q-profile visible as
  winding pitch), divertor strike glow, ELM filament bursts, the **Q meter**, coil HUD,
  innovation sparklines, **the ghost overlay**, the mind's captions + waveform, and **the
  toggle** (§9 M5) — controller OFF → watch the VDE take it in ~50 ms.
- **Client #2 — UE5, later, same stream:** Niagara + heterogeneous volumes; the IMAX
  theater. The protocol is renderer-agnostic by design; the core never changes.

---

## §7 · Hard budgets (RTX 4070 Ti SUPER 16 GB — play mode)

| component | VRAM | latency target |
|---|---|---|
| Qwen3.5-9B Q5_K_M + 32k KV (32 KiB/tok) + compute | ≈ 7.3–7.6 GB | probe ~65 ms (measured M4.5); emit gen p95 ≤ 1.5 s |
| Nemotron 3.5 ASR 0.6B (fp16/int8 ONNX) | 0.7–1.2 GB (or CPU fallback: 0) | 160 ms chunks; partial ≤ 300 ms |
| TTS (Piper) | ≤ 0.3 GB (CPU-capable) | first audio ≤ 300 ms; barge-in pause ≤ 150 ms |
| plant tier 0/1 + policy net + MPPI buffers | ≤ 0.3 GB | dt 100 µs; policy ≤ 50 µs; RT factor ≥ 1.0 |
| tier-2 MHD **solver working set** (fields are 32 MiB at 128³×8×fp16; the budget is state copies + fluxes + staging — QC re-derived) | 0.25–0.5 GB | 10–30 Hz field updates |
| renderer (separate process, WDDM) + desktop | **measured up to ~4.6 GB on this box** (auricle receipts) — lean-desktop play mode prescribed; budget 2–4.6 GB | 60 fps |
| **total** | **9.9–14.5 GB by the rows; worst case ~16.2 GB does NOT fit** (QC re-summed) | **the shipping config is prescribed, ≈ 11.4 GB:** ASR on CPU (D-004/V-04), policy on CPU (D-012), lean desktop — that is the configuration every §7 claim binds to |

**Training mode is headless and LLM-free** (no contention): batched envs + PPO own the GPU.
The ghost budget: fork + 500 ms lookahead ≤ 25 ms wall (tier 0/1 at ≥ 20× RT). All numbers
above are targets to be **measured and receipted** in `runs/` — measured-not-claimed applies
from M0.

---

## §8 · Falsifiers (pre-registered; each can lose)

- **F-KEEPUP-F** — play mode holds real-time factor ≥ 1.0 with **zero plant-tick drops**
  for a full session; LLM/ASR/TTS lateness only ever sheds *judgments/speech* (recorded as
  drop-events), never dilates the world. The Deadline Law, made killable.
- **F-NULL-C** — the policy net beats the tuned PID/MPC null on disruption rate + tracking
  RMS at matched compute, Wilson 95% CI, published either way. No net ships without it.
- **F-GHOST** — ghost forecasts match reality bit-exactly under identical inputs (memcmp,
  determinism oracle), and every Sentinel warning demonstrably **precedes** the command
  commit on the tape. If ghosts drift from the plant, the one-dynamics-source law is broken
  and the feature dies.
- **F-VOICE** — full-duplex: barge-in pause ≤ 150 ms measured end-to-end (mic → pause);
  voiced-vs-drafted ledger complete for every utterance; zero un-receipted audio.
- **F-PRESENCE-F (the control-room dave test)** — over scripted operator sessions with
  planted dangerous/erroneous commands, blind-grade the triple's interjections
  (usefulness, timing, pre-commit) against a turn-based twin fed the same telemetry
  summaries at its best cadence. The estate's core bet, staged in the one domain where
  every interjection carries a mechanical verifier.
- **F-INSTINCT** (inherited) — the emit judgment must survive dial-at-zero on this
  domain's held-out set before the voice loop leaves shadow.

---

## §9 · Milestones (each gate = receipts in `runs/`, booster-style)

- **M0 — the burn.** Tier-0 + PID + headless MC + minimal three.js dashboard (Q meter on
  a glowing shader ball). *Gate:* memcmp replay green; PID sustains Q > 1 on the easy
  scenario; MC prints CIs.
- **M1 — the shape and the enemy.** Tier-1 GS + vertical instability + synthetic
  diagnostics + coil circuits; MPPI oracle proves every curriculum scenario solvable.
  *Gate:* oracle lands flat-top through the disturbance set; instability demonstrably
  kills an open-loop run.
- **M2 — the cerebellum.** Distill → PPO on batched envs. *Gate:* **F-NULL-C**.
- **M3 — the membrane (shadow).** Warpbus + event tokenizer + the triple watching in
  log-only soak mode (the M4.5 pattern: ledger, margins, surprisal, briefs) + ghost-forks
  live. *Gate:* **F-GHOST** + shadow receipts + fire-rate report.
- **M4 — the voice.** Nemotron/sherpa in, Piper out, duplex + barge-in, voiced-vs-drafted
  ledger. *Gate:* **F-VOICE**, tested on **solicited speech** (the operator asks, the
  reactor answers aloud); unsolicited initiative remains shadow → brief until F-INSTINCT.
- **M5 — the talking reactor.** Writs at H0/H1, etiquette caps, the **toggle demo**, and
  the 90-second video: breakdown → flat-top → disturbance survived → operator error →
  Sentinel ghost interjection *mid-sentence* → toggle OFF → the beautiful catastrophe →
  replay with the controller back on. *Gate:* **F-KEEPUP-F + F-PRESENCE-F** scripted run,
  ledger published.

---

## §10 · Repo layout & build

```
C:/fusion/
  core/       plant tiers 0/1/2, integrator, diagnostics, fork/ghost, safety gates (C++20/CUDA)
  control/    policy runtime (flat-weight mmap), MPPI, PID/MPC nulls
  membrane/   warpbus glue: event tokenizer, writ translator, triple host (reuses
              auricle's cortex_llama/fabric.h/soak patterns; vendored or submoduled)
  voice/      asr.h seam (nemotron | sherpa backends), tts.h seam (piper), duplex governor
  trainer/    Python rig (PPO/CEM/distill/export_weights.py) — dev only, never in product
  telemetry/  binary protocol, ring, socket server
  ui/         Tauri + three.js/WebGPU client (pure observer)
  contracts/  events.toml, writs.toml, floors.toml (the governor tables), scenarios/
  goldens/    seeded bit-exact trajectories
  runs/       receipts (gitignored; snapshotted)   tools/  dev scripts (PS allowed)
```
CMake, targets `fusor` / `fusor_mc` / `fusor_train_env` / tests; vendored third_party,
pinned; **C/C++20 + CUDA only in the product; Python confined to `trainer/`**; forward
slashes in all shell commands (Git Bash eats backslashes — machine law).

## §11 · Doctrine inheritance (one line each, so nothing is re-derived wrong)

| law | source | lands here as |
|---|---|---|
| state only through the integrator; no assist; one dynamics source; deterministic+memcmp; headless MC gates | Booster | §2.2 |
| solver-then-distill; teacher quality; export-weights-to-C | Booster LODESTAR | §3 |
| anti-turn membrane; two planes; segmenter; {hold,emit}; molt; surprisal tap; A3 role-headers | SYNCYTIUM / auricle (measured) | §4 |
| Deadline Law; tokenized experience; rungs beat nulls blind; label factory | TinyVillage | §1, §4.1, §8 |
| Verifier Law (search where verifiers, Skeptic where not); L-SILENCE; governor floors; receipts non-mintable; no organ outlives its null | the estate canon | §3, §4.2, §8 |
| innovation-gated tokenization; writs-not-actuators; the surprise cascade (innovation → event-model → logits) | this spec (new) | §4.1–4.2 |

## §12 · Honest limits, up front

Physics: toy geometry, prescribed transport, decorative tier-2 — never claim machine-design
relevance; the *control problem's* difficulty class is real, the numbers are scaling-law
grade. ASR: the Nemotron ONNX/TensorRT export path is unverified on this box — hence the
proven sherpa fallback behind one seam; H100 throughput figures are vendor-reported and
irrelevant at N=1 anyway (we need latency, which must be measured here). Membrane: the 9B
judgment ceiling, the over-fire/etiquette gap, and stance confusion are inherited *measured*
problems — the voice loop stays in shadow until the emit QLoRA passes F-INSTINCT, because a
reactor that talks ~900 times an hour (921/hr, the full-day measured rate — D-015) is a
fire alarm, not a colleague. F-INSTINCT's held-out eval set for THIS domain does not exist
yet and no milestone creates it (QC blocker) — v0.2 must assign it a home. And the whole spec is
pre-build: every number in §7 is a target until a receipt in `runs/` says otherwise.

---

*v0.1, 2026-08-09 — spec staked before the build, falsifiers first, same rule as always:*
*rent the intelligence, own the loop — and this time, give the loop a voice.*

*QC pass (same day): six catches applied — the multi-rate GS law (full solve 100–200 Hz +
10 kHz linearized vertical model), the named innovation source (EKF/forward model beside
the policy), barge-in on VAD not ASR-partial, solicited-vs-unsolicited speech standing,
honest worst-case desktop VRAM, and the ghost guarantee restated at the commit boundary.
Logged as D-003…D-008 in `DECISIONS.md`.*
