# KICKOFF — M1 · the shape and the enemy (bootstrap for the next session)

> **STATUS 2026-08-09 (S7–S12): slices 1, 2a, 2b-1, 3, 4, 5 are DONE and merged** —
> the enemy (D-035), the Δ* core (D-036), the shaped equilibrium (D-037), the MERGE
> (D-038), the FREE BOUNDARY (D-039: DN separatrix held by the 12-circuit set;
> machine pins corrected by the solver-as-fresh-eyes; slow-manifold kick), and **the
> MODEL-BASED EKF (D-040): 4-state moment-matched Kalman, D-023's information set
> consumed in full (scatter + contract latencies + coil channel), χ² NIS live —
> pre-registered [0.5, 2.0], measured [1.27, 1.30]; kicks tokenize 20/20; the
> resonant-artifact defect found and damped at the source (dance 62→2.2 mm RMS);
> γ⁻¹ 23.2 ms; first §7.4 datum: vertical+EKF p99.9 = 2.44 µs.** Handoff items 1–3
> are done, and **slice 6 (D-041) landed the DST direct solver + the D-024 pipeline:
> the free-boundary equilibrium is the RUNTIME k_dest source, tracked at 200 Hz
> (solve@T applies@T+50, gs_late, ΔZ revalidation), plant retuned per tick, observer
> re-linearized with the Jacobian x/P carry; rampdown.toml is the first moving-
> equilibrium scenario (k_dest −6.8% tracked, 0 late, 20/20 GOOD, replay bit-stable);
> the artifact is now OVERDAMPED (ζ 1.2 — the off-design-point pumping fix);
> §7.4 vertical+EKF p99.9 = 3.78 µs.** SESSION_LOG S13 is the tail of truth.
> **Next: transport (l_i movement — the deep lethal-legal lever) + diagnostics into
> the burn loop + the statecheck fence, or the oracle (owns rampdown ≥95% + the
> curriculum) — then F-SHAPE-1.**
> The scope list below remains the milestone's full charter.

**Read first, in order:** `CLAUDE.md` → `FUSION_ARCHITECTURE_v0.2.md` **§2.1 tier-1,
§2.4, §3 (EKF + statecheck), §8 F-SHAPE-1, §9-M1** → `contracts/machine.toml` ([vessel],
[coils] — the passive-conductor inputs and the 12 circuits) → `contracts/diagnostics.toml`
(the sensor model the fence guards) → `contracts/dispersions.toml` → `DECISIONS.md`
D-021…D-034 → `SESSION_LOG.md` tail → `runs/m0-burn-2026-08-09.md` +
`runs/m05-thesis-slice-2026-08-09.md` (what already works). Branch: **`m1-shape`**.

## Scope (spec §2.1 tier-1 — the plant grows its enemy)

1. **Free-boundary Grad-Shafranov** on 65×65: Picard + fast Poisson (DST/cyclic
   reduction) + von Hagenow free-boundary coupling; p′(ψ)/FF′(ψ) from the parameterised
   current-profile family (l_i, β_p as 0-D states; sawtooth resets q₀ — PHY-14).
   CUDA optional at first (CPU reference solver FIRST, golden-able; CUDA port after
   parity — the memcmp law binds the CPU path, toleranced per-arch on GPU).
2. **The passive conductor model** (PHY-03/CTL-01): 24 axisymmetric filaments + the 12
   actuated circuits, full mutual-inductance matrix (build GENERAL — it carries quench
   eddy/halo later); **γ REPORTED from the eigenproblem at each equilibrium** (never
   configured; design-point expectation 25–40 ms — machine.toml [vessel] comment).
3. **The multi-rate law as scheduled pipeline** (D-024): GS solve scheduled at tick T
   applies at T+50 (200 Hz); the linearized rigid-displacement + circuit model carries
   10 kHz between; sim WAITS if wall-late (`gs_late` event, aggregate-pacing F-KEEPUP);
   ΔZ validity bound forces an early re-solve.
4. **Synthetic diagnostics** per `contracts/diagnostics.toml` (noise, bias walk,
   integrator drift, latency, dropout) + **the statecheck fence** (CTL-14b): the
   observation builder and policy runtime link against a header that cannot expose true
   state; shipping target fails to compile if it references it; `statecheck` ctest =
   hard M1 gate.
5. **The EKF** (D-023): linearized vertical/circuit model, diagnostics-only inputs,
   pre-registered information set (never the disturbance schedule, never blind truths,
   parameters carry plant_scatter mismatch), covariance carry across re-linearizations,
   **rolling NIS χ² acceptance gate** — events.toml σ-thresholds unlock only when it
   passes. Innovation vector plumbed to (future) policy AND the tokenizer's
   [innovation] cluster events (the M0.5 feed finally gains its missing lane).
6. **§2.4 disruption model, full:** VDE trigger (|Z| + dZ/dt, spine_gates), wall-contact
   Z excursion, TQ/CQ as at M0 + vertical-driven paths; `vde_kick`, `rampdown`,
   `hl_backtransition` scenarios + the **lethal-legal class** (FRESH-3: a floor-legal
   I_p ramp that destabilizes the vertical eigenvalue — now constructible, since γ
   moves with l_i/κ).
7. **The offline oracle** (CEM/MPPI teacher, D-024/CTL-26): per-timescale horizons,
   piecewise-constant action segments 1–10 ms, decorrelated Philox stream (D-011);
   proves every curriculum scenario solvable.
8. **Gate = F-SHAPE-1** (spec §8): open-loop (VS off) demonstrably disrupts via
   VDE→quench with tokens on tape; oracle lands flat-top through the full disturbance
   set incl. vde_kick + hl_backtransition ≥95% seeds; EKF NIS green; determinism suite
   green at tier-1 (incl. the pipeline schedule). Receipts `runs/m1-*`; goldens
   regenerate (tier switch invalidates them — PHY-10; a D-entry records the swap).

## Build notes

The M0 build block stands (CUDA arrives via `enable_language(CUDA OPTIONAL)` when the
GPU port lands — not before CPU parity). Steal: `C:/Booster_Lander_Simulator/core/`
integrator/telemetry shape. The 100 µs tick budget table (§7.4) starts BINDING at M1 —
measure the linear step + EKF early, receipt p99.9.

## Mistakes that cost prior projects real time — do not repeat

Forward slashes always · no unordered FP reductions (the GS reduction must be
deterministic on CPU; the CUDA port is TOLERANCED, never bit-claimed — CTL-02's scoping)
· γ is reported, never configured · the EKF never sees scenario flags (hide-the-flags —
CTL-21) · pre-register scenario files before tuning anything · goldens invalidated =
D-entry + regenerate, never hand-edit · SESSION_LOG + KICKOFF_M2 before you finish ·
**NOMINMAX before windows.h** (M0.5's compile lesson) · TOMLs are single-line strings
and single-line inline tables (P-01's empirical answer).
