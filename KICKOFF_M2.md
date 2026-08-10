# KICKOFF — M2 · the cerebellum (bootstrap for the milestone's first session)

**Read first, in order:** `CLAUDE.md` → `FUSION_ARCHITECTURE_v0.2.md` **§3 (the
cerebellum — obs/policy/reward doctrine, CTL-14b/21/26), §2.3 (writs/floors — the
gates the net lives under), §7 (budgets: in-loop inference < 50 µs), §9-M2** →
`DECISIONS.md` D-035…D-043 → `SESSION_LOG.md` tail (S15 = M1 DONE, F-SHAPE-1 green
attempt 1) → `runs/m1-fshape1-2026-08-09.md` (the gate receipt — the curriculum and
the null you must beat) → `control/obs.h`/`control/policy.cpp` (the FENCE — the net
lives in this TU's world; statecheck is the law). Branch: **`m2-cerebellum`**.

## Scope (spec §9-M2)

Solver-then-distill → PPO polish on batched envs (2048+; CUDA arrives here via
`enable_language(CUDA OPTIONAL)` — CPU batched envs FIRST, goldens/replay still
CPU-bound law); flat-weight export → in-loop inference < 50 µs (§7.4 row, measured
p99.9); the net eats the OBSERVATION FRAME + the innovation vector (CTL-21), never
true state (statecheck extends to the net's TU).
**Exit gate: F-NULL-C** — the net beats the tuned classical null (the M0/M1 CEM
product: `control/gains_m0.toml` behind `control/policy.cpp` — receipted 100/100
across the M1 curriculum at gate seeds) on disruption rate + tracking RMS at matched
compute, Wilson CI, published either way. The MPC comparison is separately receipted
later (D-020). If the null wins, receipt it and ship the null (DoD P-26).

## What M1 hands you (use it, don't rebuild it)

- **The plant**: tier-1 complete — free-boundary GS tracked at 200 Hz (D-024
  pipeline, gs_late), 27-state vertical with full mutuals, 1-D transport with
  evolving profiles + sawteeth, diagnostics-fed burn PIDs, disruption chain.
  `run_sim` is the ONE dynamics source; `SimInputs.fctx` shared per process.
- **The curriculum** (all 100/100 at the null): easy · vde_kick(60mm) ·
  hl_backtransition · rampdown (+ vde_open_loop as the death demo). Seeds law:
  train < 800000 (seedcheck), gate ≥ 900000.
- **The seams**: VerticalEKF (χ² NIS green, [0.5,2.0]) feeds the [innovation] gate —
  the net gets ν/S per CTL-21. The fence: `control/obs.h` frames; the policy TU
  cannot compile against true state (poison + statecheck).
- **Budgets banked**: vertical retune+step+EKF p99.9 = 3.78 µs/tick. Your net gets
  < 50 µs in-loop (§7.4) — measure early with the m1s5/m1s6 budget-mode pattern.

## Deferred/owed items that land IN M2 (pre-registered)

1. **The LQ vertical null** (spec §3 names PID+LQ as the composite null) — build it
   before the net; F-NULL-C's baseline is the STRONGER of PD/LQ per channel. The
   S10 sweep landscapes are stale (damped artifact) — remap around the shipped gains.
2. **The deep lethal-legal scenario** (FRESH-3): long-horizon floor-legal ramp
   walking γ past the VS edge (the mechanism is live: k_dest moved −11.9% in 12 s;
   go longer/deeper). The ghost's value separates from the governor HERE.
3. **Per-seed trajectory oracle** (CTL-26 load-bearing): MPPI/CEM over action
   segments on the real sim (decorrelated teacher stream = 1) — the distillation
   teacher. The M1 gate used the parameter-CEM product (D-043's ruling).
4. **Bolometer channel** in diagnostics.toml (the rad-ff currently reads the true
   Prad as a stated tier-1 simplification) + rogowski noise on the observer's Ip.
5. **NIS off-reference band** (rampdown measured 1.02 — formalize the acceptance
   beyond the reference suite when the net trains across operating points).
6. **β_p split refinement** in the GS family + q0 axis-resolution work if sawtooth
   fidelity starts to matter to control.

## Build block (unchanged + additions)

VS 17 2022, `/fp:strict`, tomlplusplus vendored, llama import libs delay-loaded.
Trainer stays Python (`trainer/`); the batched-env runner is C++ (`fusor_train_env`
target named in spec §6 — build it this milestone). CUDA optional-first. NOMINMAX
before windows.h. TOMLs single-line. Forward slashes in every shell command.

## Mistakes that cost M1 real time — do not repeat

Stale binaries lie (rebuild before believing a probe) · silent python replaces need
asserts · the harness EATS backslash-escapes in heredocs (write patch scripts to
files) · Q-inflation cannot fix a systematic model error (find the mode) · a
regularization artifact must be non-resonant BY CONSTRUCTION (ζ 1.2 — D-041) ·
calibrate transport in the operator's own discretization, and subtract BC parts
from source-part calibrations (D-042's two receipted bugs) · goldens invalidated =
D-entry + regenerate, never hand-edit · SESSION_LOG + KICKOFF_M3 before you finish.
