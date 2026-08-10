# FORWARD NOTES — for the M2 instance, from the M1 instance
*Written 2026-08-10 at the M1 gate, with the entire S10–S15 arc in working memory.
Not a contract — a brain dump of predictions, design forks, traps, and headroom
analysis that would otherwise be lost. Read KICKOFF_M2.md first; this is the layer
underneath it: WHY those items are listed and what I'd do in your seat.*

## 0 · The honest difficulty map

Behind us: physics with verifiable right answers and tight feedback loops. Ahead:
- **M2 is a falsifiable bet against a STRONG null.** The CEM-tuned classical stack
  lands 100/100 on the whole curriculum and its gains survived five physics upgrades
  untouched. On the CURRENT curriculum there is almost no headroom to beat it —
  see §2. Losing honestly is survivable (DoD P-26) but winning hollow is not.
- **M3–M5's claims get softer as the stack rises.** The falsifier discipline
  (three-arm, code-graded, pre-registered — the M0.5 instrument) is the defense;
  operating it under demo pressure is the cultural hard part. The physics is done
  being the hard part; honesty engineering becomes it.

## 1 · Do the LQ null FIRST, and let it be strong

The 4-state EKF model (z, v, q_s, I_vs — moment-matched, calibrated slow pole) IS
the LQR design model. `VerticalEKF::model_from` already produces the A/B pair;
solve the Riccati offline, gain-schedule on the pipeline's k_dest exactly like
everything else retunes, and you have LQG nearly for free. Predictions:
- LQ kills the PD's residual limit cycle (the S9 handoff predicted this; the damped
  artifact makes it cleaner now). It becomes the REAL baseline.
- The stale Kd-cliff sweep maps (pre-damping) will mislead you — remap first.
- The LQ solution doubles as a distillation sanity check: a net that can't match
  LQ in the linear regime is broken, full stop.

## 2 · The headroom problem (the most important section)

F-NULL-C compares disruption rate + tracking RMS at matched compute. Where the
null is at 100/100, the net can only tie. **The curriculum must get harder BEFORE
training — pre-registered, with the null's failure curve measured FIRST** (that
ordering is everything for credibility; curriculum-shopping after results is the
project's death). Where real headroom plausibly lives, in order of my confidence:

1. **The kick psychometric curve.** Sweep vde_kick magnitude 60→120 mm at gate
   seeds; find the null's 50% point (my estimate: 85–100 mm — the VS authority
   analysis at S11 put the raw edge near 54 mm quasi-static, but the wall carries
   the transient; measure, don't trust my estimate). Pre-register the net's target
   band just above the null's knee. Recovery smoothness (z-excursion integral) is
   headroom even where both survive.
2. **Combined events**: kick DURING rampdown (moving equilibrium, drifting k_dest
   anchor — fixed gains are most wrong there); puff + kick; backtransition + ramp.
   Real machines die to coincidences, and the null has no cross-channel reflexes.
3. **The DEEP lethal-legal** (FRESH-3, the thesis scenario): 30–60 s, floor-legal
   Ip/shape trajectory that walks γ⁻¹ down toward the actuator edge with the l_i
   lever (τ_CR ≈ 13 s) fully expressed. The mechanism is live (−11.9% k_dest in a
   12 s ramp); depth needs duration. This is ALSO the ghost's showcase scenario
   (M3) — design it once for both consumers.
4. **Drawn-magnitude puffs** (the loguniform 0.002–0.02 band in dispersions.toml is
   pinned but easy.toml uses a fixed mag — the drawn ladder is the harder variant).
5. **Deeper/slower hl_backtransition** (target 0.6, recovery 3–5 s) — stresses the
   fixed feedforward.

## 3 · The objective contract needs a D-entry BEFORE any training

Re-read D-032h (objective-without-gate-terms drove pass to 0.00 — the misaligned-
objective receipt). Two specific traps here:
- The null runs a deliberate hot-hold (Ki≈0, offset-as-strategy; Q ≈ 2–3 vs
  q_report_set 1.7; n̄ settles 2.26e20 vs set 3.0). A "tracking RMS" objective
  scored against the setpoints lets the net "win" by tracking numbers the null
  deliberately ignores — a hollow win. Define per-scenario scoring: disruption/
  margin/effort where survival is the point; tracking only in scenarios DESIGNED
  for tracking.
- "Matched compute" needs a pinned definition (FLOPs/tick? wall µs on the tick
  path? EKF included?) before results exist, or it will be litigated after.

## 4 · Batched-env architecture (the fork to think through before building)

The cost problem: ramping scenarios pay ~1 ms/solve at 200 Hz per env — 2048 envs
of that is impossible. But the pipeline's stationary-reuse (D-041) makes FLAT-TOP
solves free. So:
- **CPU-first is viable**: ~2 µs/tick/env flat-top → 2048 envs ≈ 41 CPU-s per
  sim-s ≈ 2.6 s wall on 16 cores; 200M-tick PPO ≈ ~25 h wall. Heavy but real.
  CUDA (spec: optional) cuts 10–50× — port the tick loop, keep goldens CPU-bound
  (CTL-02 tolerancing).
- **Curriculum-stage the expensive tail**: train mostly on flat-top scenarios
  (free solves), introduce ramp/lethal-legal episodes late with fewer parallel
  envs. The ONE-dynamics-source law holds throughout (no surrogate plant).
- **Action cadence**: the net does NOT need 10 kHz everywhere. Consider the burn
  channel at 1 kHz and the vertical at 10 kHz — or, seriously consider:
- **Residual-on-null architecture**: the net outputs bounded corrections around
  the LQG null rather than raw actuators. Safest (bounded by construction),
  cleanest matched-compute story (count the residual's budget), and it converts
  "beat the null" into "improve the null" — which is winnable on the headroom
  scenarios without re-learning what classical control already knows. If the spec's
  wording ("the net beats the null") is read as replacement-only, D-entry the
  interpretation before building. My recommendation: residual first; raw-actuator
  as the stretch arm.

## 5 · Mechanical prep items (cheap now, painful later)

- **statecheck**: add the net-runtime TU(s) to FENCE_TUS the day the files exist.
  The obs builder for the net = control/obs.h frames + the innovation vector
  (CTL-21). Never let a training convenience import reach the true state.
- **Inference determinism**: flat-weight export with fixed-order dot products on
  the /fp:strict CPU path; GPU inference (if any) is TOLERANCED, never bit-claimed.
  Decide numerics before the first checkpoint exists.
- **Budget instrument**: extend the test_vertical budget mode to net inference on
  day one (< 50 µs p99.9 is the §7.4 row; the 4-state EKF+retune is 3.78 µs — a
  64×64 MLP is ~2–5 µs, so the budget is comfortable UNLESS someone reaches for a
  framework runtime; don't).
- **Renderer catch-up**: membrane/feed doesn't know EvKinds 9–11 (GsLate, Sawtooth,
  HLBack). 20 lines. Do it before any M3 soak or the feeds silently drop the new
  physics (the tokenizer's diet is the thesis — don't starve it).
- **Ghost proof-of-concept** (30 lines, an M2-lull task): fork the sim at tick T
  (SimInputs + state copy — FreeTrack is copyable by design), run both 1 s, memcmp.
  De-risks F-GHOST while everything is warm.
- **gs_late has never fired in anger post-fix** (0 across all healthy runs). One
  dev run with a tiny ΔZ bound to prove the late path before F-KEEPUP leans on it.
- **Snapshot discipline**: training checkpoints will bloat snapshots — pre-decide
  (receipts + final weights yes; checkpoint churn no). Small P-entry.

## 6 · Known soft spots in MY OWN M1 work (check before building on them)

1. The ECE frozen cross-cal drifts as peaking evolves (sawtooth cycles wobble
   T_obs vs truth). It's honest measurement physics, but a net trained on T_obs
   learns the wobble — document it in the obs contract rather than discovering it
   in a training curve.
2. l_i conventions: li_1d (cylindrical, j∝T^1.5 instantaneous) reads 1.45–1.51;
   the GS's flux-geometry l_i differs. If anything gain-schedules on l_i, pick ONE
   and receipt it.
3. The n-channel settles below setpoint by strategy; if the objective rewards
   n_set exactly, the net will pump gas the null doesn't — see §3.
4. The β_p split coupling is scaling-grade (one multiplicative knob per solve);
   fine for γ movement, not for detailed shape claims.
5. The oracle ruling (D-043) is honest but MINIMAL — the per-seed trajectory
   teacher is still owed, and distillation quality will depend on it more than on
   PPO polish. Budget it as a real slice, not an afterthought.

## 7 · Things to ponder (no action required, just carry them)

- **The null winning is a publishable outcome.** "A carefully built classical
  stack is hard to beat at matched compute on physics-honest scenarios" is itself
  a finding the estate can cite. Want the truth more than the win; the receipts
  make either valuable.
- The project's real moat so far isn't any single organ — it's the METHOD
  (pre-registration, receipts, nulls, falsifiers, determinism). M2 is the first
  milestone where the method gets tested against the temptation to tune the task
  toward the desired winner. The fence, the seed guard, and the pre-registered
  failure curve are the antibodies; use them.
- Session cadence shifts at M2: training runs are wall-clock hours, so slices
  become "design + launch + hand off; next session reads results." The machine's
  detach patterns (Start-Process, marker files — never bare PIDs) become
  load-bearing. Plan slice boundaries around runs, not around context exhaustion.
