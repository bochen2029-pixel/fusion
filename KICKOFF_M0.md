# KICKOFF — M0 · the burn (first coding session bootstrap; rewritten at v0.2)

**Read first, in order, before any code:** `CLAUDE.md` (charter + non-negotiables +
glossary) → `FUSION_ARCHITECTURE_v0.2.md` (the canon; §2 and §9-M0 are your sections;
v0.1 is history, do not build against it) → `contracts/machine.toml` (FUSOR-1 — every
number you implement derives from it) → `contracts/` (objective, dispersions, diagnostics,
events, floors, spine_gates, writs, scenarios/_TEMPLATE) → `ROADMAP.md` (M0 exit) →
`SESSION_LOG.md` tail → `DECISIONS.md` D-001…D-030. Then confirm, in your own words:
(a) the three tiers + two seams, (b) the booster constitution's six rules **including the
SPINE_SHUTDOWN failure class and the input-tape replay law**, (c) M0's exact scope and
F-BURN-0's numbers, (d) the multi-session discipline (append your session entry;
same-commit docs; write/refresh KICKOFF_M0's successor note before you finish). Then build.

## Step 0 — branch

```bash
git -C C:/fusion checkout -b m0-burn
```
Milestone branches are `m<N>-<slug>`; `main` merges at gates (DoD). Work never lands on
`main` directly again.

## Step 1 — third_party (the opening move; the only vendoring M0 does)

Vendor **tomlplusplus v3.4.0** (header-only, MIT) at `third_party/tomlplusplus/`:
download the single-header release `toml.hpp`, record its sha256 + source URL + version in
`third_party/README.md` (create it — auricle's `third_party/README.md` is the style).
Downloads staged via `third_party/dl/` (gitignored). No package managers, ever, in the
product build.

## The build block (copy exactly; forward slashes always)

```bash
cmake -S C:/fusion -B C:/fusion/build -G "Visual Studio 17 2022" -A x64
cmake --build C:/fusion/build --config Release
ctest --test-dir C:/fusion/build -C Release --output-on-failure
```
CMake ≥ 3.26, C++20 (`/std:c++20 /fp:strict` on the sim targets), CUDA **not required at
M0** (tier-0 is CPU). If `cmake` is not on PATH, locate VS via
`"C:/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe"`.
Every receipt quotes this block verbatim as its reproduce line.

## M0 scope (and nothing more)

1. **CMake skeleton:** targets `fusor`, `fusor_mc`, ctests (`replay_oracle`, `seedcheck`,
   `budget_events` stub-ok). `fusor_mc` is a **separate binary** (P-21 ruled):
   `fusor_mc --scenario contracts/scenarios/easy.toml --seeds 900000:901000 --out runs/...`.
2. **`core/` tier-0 burn physics, per spec §2.1 exactly:** Bosch-Hale (range-asserted),
   alpha heating with slowing-down lag + pinned constants, Bremsstrahlung, **line
   radiation L_z(Ar) table**, synchrotron (reflectivity from machine.toml), **Ohmic +
   volt-seconds**, **particle balance + He ash + dilution**, IPB98(y,2) **solved
   implicitly** (κ_a from machine.toml, H98 the knob) → live Q. **RK4, dt = 100 µs,
   burn sub-cycle ratio 10 (compile-time), quench windows per-tick.** Philox4x32-10 with
   the `dispersions.toml [streams]` domain law; no wall-clock in the sim path; state only
   through the integrator. Floors with `activates_at = "M0"` enforced; spine gates
   `quench_precursor` active (tier-0 signals only); SPINE_SHUTDOWN is a verdict class.
3. **`control/` PID null:** heating/fueling PID holding the Q setpoint on
   `contracts/scenarios/easy.toml` (create it FROM `_TEMPLATE.toml` — every table, class
   "gate", the impurity-puff disturbance included; pre-register before tuning). **The PID
   is CEM-tuned by `trainer/` on train-domain seeds with a declared budget, and the tuning
   is receipted** (CTL-29 — this PID becomes M2's shipped null).
4. **`fusor_mc`:** N seeded runs headless → GOOD/DISRUPT/SPINE_SHUTDOWN/TIMEOUT rates +
   tracking RMS, Wilson 95% CIs on rates, per `objective.toml [stats]`. The **seedcheck**
   ctest proves the trainer refuses gate-domain seeds.
5. **memcmp replay oracle** as ctest: same seed + scenario (+ empty input tape) twice →
   bit-identical state trajectory.
6. **Goldens (DoD #2 — defined here, created here):** `goldens/m0-easy-<seed>.golden` =
   binary state-vector dump every 10 ms sim time + a final FNV-1a hash file, for 3 named
   gate seeds. Binary per `.gitattributes` (`goldens/** -text`). Committed.
7. *(stretch, non-blocking)* minimal three.js Q-meter on the telemetry ring — `ui/` only.

**Exit gate = F-BURN-0 (spec §8) + DEFINITION_OF_DONE in full:** Q ≥ 1.0 held ≥ 10 s
through the pre-registered impurity puff in ≥ 90% of N = 1000 gate seeds (Wilson lower
bound ≥ 0.88) · memcmp green · goldens committed · PID tuning receipted ·
`runs/m0-burn-<date>.md` states config, seeds, numbers, CIs, the reproduce block, and the
null result · ROADMAP box + SESSION_LOG entry + any D-entries in the same commit ·
KICKOFF_M0.5 written (the thesis-slice session is next — D-019) · snapshot via
`tools/snapshot.ps1` · merge `m0-burn` → `main`.

## Patterns to steal (read-only lineage — never modify those repos)

- `C:/Booster_Lander_Simulator/core/` — integrator/determinism/telemetry shape;
  `trainer/export_weights.py` (M2, note only).
- `C:/auricle/src/core/ring.h` — the SPSC ring (copy the pattern, `fusion` namespace).
- `C:/auricle/CMakeLists.txt` + `third_party/README.md` — vendoring + ctest wiring style.

## Mistakes that cost prior projects real time — do not repeat

Backslash paths in shell commands (forward slashes always) · an assist term nudging
physics toward success (spine gates are a FAILURE class, not a rescue) · wall-clock or
unordered FP reductions in the sim path · Python anywhere in the product · tuning on gate
seeds (seedcheck exists to catch you) · claiming a number without a receipt · finishing
without SESSION_LOG/KICKOFF (the handoff IS part of the milestone) · recording the
operator's mic anywhere but the headset lane (consent floor, CLAUDE.md #7).
