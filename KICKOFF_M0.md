# KICKOFF — M0 · the burn (first coding session bootstrap)

**Read first, in order, before any code:** `CLAUDE.md` (charter + non-negotiables +
glossary) → `FUSION_ARCHITECTURE_v0.1.md` (the canon; §2 and §9-M0 are your sections) →
`ROADMAP.md` (M0 exit gates) → `SESSION_LOG.md` tail (state of truth) → `DECISIONS.md`
(D-001…D-010) → `contracts/` (pre-registered tables). Then confirm, in your own words:
(a) the four tiers + two seams, (b) the booster constitution's five rules, (c) M0's exact
scope and exit gate, (d) the multi-session discipline (SESSION_LOG append + same-commit
docs + write KICKOFF_M1 before you finish). Then build.

## M0 scope (and nothing more)

1. **CMake skeleton** (`fusor` + `fusor_mc` + ctest targets; C++20; CUDA optional at M0 —
   tier-0 is CPU-fine; forward-slash build commands in all docs).
2. **`core/` tier-0 burn physics:** Bosch-Hale ⟨σv⟩(T) D-T, alpha heating, Bremsstrahlung,
   synchrotron (Trubnikov), IPB98(y,2) τ_E, 0-D power balance → live Q. dt=100 µs
   integrator with 1 ms burn cadence (sub-cycled). Philox4x32-10 seeded; no wall-clock in
   the sim path; state only through the integrator.
3. **`control/` PID null:** heating/fueling PID holding a Q setpoint on the easy scenario;
   config in `contracts/scenarios/easy.toml` (create it; pre-register before tuning).
4. **`fusor_mc`:** N seeded runs headless → sustained-Q rate + Wilson 95% CI printed.
5. **memcmp replay oracle** as a ctest: same seed+scenario twice → bit-identical state
   trajectory hash.
6. *(stretch)* minimal three.js dashboard on the telemetry ring (Q meter + temps on a
   glowing shader ball) — pure observer; skipping it does not block the gate.

**Exit gate (DEFINITION_OF_DONE.md applies in full):** memcmp green · PID sustains Q > 1
on `easy.toml` · MC prints rates+CIs · receipts `runs/m0-burn-<date>.md` · ROADMAP box +
SESSION_LOG S1 + any D-entries in the same commit · `KICKOFF_M1.md` written · snapshot.

## Patterns to steal (read-only lineage — never modify those repos)

- `C:/Booster_Lander_Simulator/core/` — integrator/determinism/telemetry shape;
  `trainer/export_weights.py` (for M2, note only).
- `C:/auricle/src/core/ring.h` — the SPSC ring (copy the pattern, `fusion` namespace).
- `C:/auricle/CMakeLists.txt` — the vendored-libs + ctest wiring style.

## Mistakes that cost prior projects real time — do not repeat

Backslash paths in shell commands (Git Bash eats them — forward slashes always) ·
an "assist term" that nudges physics toward success (forbidden; a failed control run
crashes honestly) · wall-clock or unordered FP reductions in the sim path (kills
determinism) · Python anywhere in the product (trainer/ only) · tuning against the gate
scenario then claiming generality (pre-register scenarios first) · finishing without
SESSION_LOG/KICKOFF (the handoff IS part of the milestone).
