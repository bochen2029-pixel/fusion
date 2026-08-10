# fusion — SESSION_LOG (the state of truth)

One entry per session, **appended at the end, never rewritten**. A fresh session reads the
tail and knows exactly where the project stands. Format: date · session id/handle · what
was done (with receipts/commits) · honest state · next. **Coding-session entries
additionally carry: receipt paths, commit SHA, measured numbers, and the null result**
(P-19 — this line is the template).

---

## S0 · 2026-08-09 · founding session (spec + staging; no code)

**Done:**
- `FUSION_ARCHITECTURE_v0.1.md` written — the full spec (four tiers, two seams, physics
  ladder, cerebellum doctrine, membrane, full-duplex voice with Nemotron-primary/sherpa-
  fallback ASR seam, telemetry/renderer, hard budgets, 6 falsifiers, M0–M5).
- **QC pass, same day — six catches applied** (multi-rate GS law; named EKF innovation
  source; barge-in on VAD; solicited-vs-unsolicited speech standing; honest worst-case
  desktop VRAM; ghost guarantee at the commit boundary) → logged as D-003…D-009.
- Repo staged for multi-session work: CLAUDE.md (charter + glossary + method),
  ROADMAP.md (M0–M5 with exit gates), DECISIONS.md (D-001…D-010), this log,
  DEFINITION_OF_DONE.md, KICKOFF_M0.md (the first coding session's bootstrap),
  `contracts/` seed tables (events/writs/floors v0-placeholders), directory skeleton,
  .gitignore, git initialized + first commit.

**Honest state:** zero code, zero measurements — every number in the spec §7 is a target.
The spec's physics is scaffolding-grade by design (§0, §12); the claim is the control +
presence architecture, not machine design.

**Next:** a brand-new session opens with `KICKOFF_M0.md` and builds M0 (tier-0 burn + PID
null + `fusor_mc` + memcmp oracle). Parent-repo patterns to steal are pointed in the
kickoff; auricle/booster/TinyVillage are read-only lineage.

---

## S1 · 2026-08-09 · the QC swarm (five Opus auditors on Intercom; no product code)

**Done:**
- Five-lane swarm QC over the whole repo — QC-PHYSICS / QC-CONTROL / QC-MEMBRANE /
  QC-VOICE-RT / QC-REPO-PROCESS — coordinated live in Intercom room `fusion-qc`
  (replayable; real cross-pollination: public self-refutations, cross-lane
  confirmations/escalations, converged joint fixes). **146 findings: 39 BLOCKER /
  60 MAJOR / 42 MINOR-NIT**, + 6 verified passes + 16 estate citations checked
  (13 verified / 1 wrong / 2 incomplete — no fabricated numbers anywhere).
- Reports: `docs/qc/QC_{PHYSICS,CONTROL,MEMBRANE,VOICE_RT,REPO_PROCESS}.md` + the
  orchestrator's `docs/qc/QC_CONSOLIDATED.md` (eight blocker clusters + dispositions).
- **Consensus corrections applied in-place** (D-011…D-017): γ⁻¹ wall-set 10–100 ms +
  passive conductor mandatory + §0 claim reframed onto F-NULL-C; LODESTAR honest-wall
  citation (M2 decides architecture against the parent's 0/16 null); policy-on-CPU fixed
  precision; Nemotron-via-sherpa unification (same backend; pin bump not export project);
  headset v1 + lane-stamped TTS; **commit = governor-accept, tick-stamped** + voice
  setpoints at H2 with objection window; events.toml budget made self-consistent
  (windowed cluster innovation + storm collapse + 300 s ticks + one budget table);
  fixed-2s-budget Deadline Law + formed_tick; 921/hr full-day rate correction; §7 VRAM
  honesty (worst case doesn't fit; shipping config ≈11.4 GB prescribed); DoD null-win
  path; CLAUDE.md ui/-exception + warpbus glossary; Greenwald floor single-sourced.
- ROADMAP gained the **V0.2 milestone (mandatory before M0)**; `KICKOFF_SPEC_V02.md`
  written; **M0 is suspended** and the spec carries a QC banner.

**Honest state:** still zero code; v0.1 is *not buildable cold* (all five lanes concur) —
but every flaw is located, filed, and dispositioned; the swarm found no fabricated
numbers, and the fastest finding was positive (sherpa already runs Nemotron 3.5).

**Next:** a fresh session opens with `KICKOFF_SPEC_V02.md`, produces
`FUSION_ARCHITECTURE_v0.2.md` + machine.toml + the redesign set, re-runs REPO's
cold-session simulation, rewrites KICKOFF_M0 — then M0 unsuspends.

---

## S2 · 2026-08-09 · fresh-eyes second instrument + cross-session adjudication (no product code)

**Done:**
- Zero-context outsider deep scan per `KICKOFF_FRESH_EYES.md` (three passes; Pass-1
  frozen to scratchpad **before** `docs/qc/` was unsealed). Deliverable:
  `docs/qc/FRESH_EYES.md` — 8 premise doubts · 22 comprehension failures · 27 flaws →
  **10 missed-by-all-five (2 gate-class: the three-arm twin, F-VERACITY), ~25
  missed-by-me, 4 disagreements**; 8 auricle receipts re-verified (921/hr re-derived
  exactly: 2,823 / 3h03m51s, run C); sherpa/Nemotron support web-verified (V-04 holds).
  No fabricated numbers — the third instrument concurs with the swarm.
- The founding session (low context) read the report and responded; the operator relayed
  both ways and ratified. Adjudication recorded in FRESH_EYES **Addendum §5**: the
  founding session conceded D1/D3/FRESH-5 and F-VERACITY outright; this session conceded
  the Q1 steelman overreach (the ghost is causal **via the sovereign** on the
  lethal-legal class — the writ doctrine working, not theater); tier-2's cut demoted to
  an operator ruling with a sanctioned compromise. One caution back on the record:
  auricle's existing F-PRESENCE null is a timer-twin, so the event-triggered arm is an
  addition there too, not a relabel.
- **Folded into the mandate (this commit):** `KICKOFF_SPEC_V02.md` de-trapped and
  expanded (synthetic FUSOR-1 replaces "SPARC/DIII-D-class"; 2–4-session honesty +
  machine-first ordering; deliverables 13–15: the eight orphaned dispositions with the
  M2 ladder first, scope rulings, ripple sweep + citation hygiene; three-arm
  F-PRESENCE-F + F-VERACITY + session input-tape + floor-legal-but-lethal class into
  deliverables 4/5/6; tie-break protocol). ROADMAP: V0.2 bullet updated; **M0.5 "the
  thesis slice" inserted before M1** (D-019); M2 null = PID+LQ (D-020). DECISIONS:
  **D-018 / D-019 / D-020** appended. `KICKOFF_FRESH_EYES.md` committed as the
  instrument's mandate.

**Honest state:** still zero code. The spec's numbers are now verified-honest three ways
(swarm, stranger, receipts); the machine-audit and the claim-audit are folded into one
v0.2 worklist. The claim as staked is falsifiable-on-paper; nothing is falsified until
rigs exist.

**Next:** a fresh session opens `KICKOFF_SPEC_V02.md` (now scoped as 2–4 sessions,
machine.toml first), produces v0.2 + `docs/qc/DISPOSITIONS.md` (dispositioning the five
lanes AND FRESH_EYES), re-runs the cold-session simulation, rewrites KICKOFF_M0 — then
M0, then the M0.5 thesis slice before M1.

---

## S3 · 2026-08-09 · the v0.2 revision (same session as S2, continued on operator's order; no product code)

**Done — the whole KICKOFF_SPEC_V02 list, walked in order:**
- **`FUSION_ARCHITECTURE_v0.2.md`** — the canon bump (v0.1 frozen beside): three tiers,
  narrowed anti-turn claim (D-018), §2.4 disruption model, M2 ladder, EKF information
  set, GS pipelined-by-construction, ghost mode declared + F-GHOST rescoped, nine-
  falsifier registry (F-BURN-0/F-SHAPE-1/F-VERACITY new; F-PRESENCE-F three-arm),
  §7 rebuilt (prescribed config only, GPU doctrine, CPU cores, tick budget, latency
  chains), §11 "enforced by" column, §12 honest limits updated.
- **Contracts:** `machine.toml` (FUSOR-1 — Q ≈ 2.3 @ H98=1 and γ⁻¹ ≈ 25–40 ms derived
  from ONE config, arithmetic in-file) · `objective.toml` · `dispersions.toml` ·
  `diagnostics.toml` · `spine_gates.toml` (SPINE_SHUTDOWN failure class) ·
  `scenarios/_TEMPLATE.toml` (incl. lethal_legal class) · `events.toml` v1 (per-cluster
  Mahalanobis, [terminal]/[organ]/[phase]/[cascade], health debounce, M3 vocab freeze) ·
  `floors.toml` v1 (dir/units-in-keys/activates_at/margins everywhere + low-density
  floor) · `writs.toml` v1 (grammar + autonomy dial A0–A3 + input-tape).
- **`docs/qc/DISPOSITIONS.md`** — all 146 swarm findings + FRESH_EYES items dispositioned
  line-by-line (2 open operator questions: tier-2 ruling, LICENSE).
- **`KICKOFF_M0.md` rewritten** (branch step, tomlplusplus + third_party plan, exact
  build block, goldens defined-and-created, F-BURN-0 exit) · **cold-session simulation
  re-run: 13/13 CLEAN** (receipt `runs/v02-coldsim-2026-08-09.md`; v0.1's two hard stops
  and two contradictions all cleared).
- Supporting: DoD clauses 1/3/6 patched (P-13/P-17) · CLAUDE.md canon pointer + full
  glossary (tokamak + estate halves) + branch/hot-path conventions · README → v0.2 ·
  ROADMAP V0.2 box **checked** + standing current-kickoff pointer (P-15) · .gitignore
  completed (P-14) · `tools/snapshot.ps1` (P-17) · SESSION_LOG format note (P-19).
- **DECISIONS D-021…D-030** appended (machine pin; objective/dispersions/seed law; EKF;
  offline-planner + pipelined GS; M2 ladder; ghost rescope; arbitration + budgets;
  membrane mechanics; falsifier registry v2; spine gates + dial + grammar).

**Honest state:** the spec is now buildable-cold by its own re-run simulation, every
audit finding is traceably dispositioned, and the claim is falsifiable on paper — and
still zero code, zero measurements: every §7 number remains a target until `runs/` says
otherwise. Two questions await the operator: tier-2 MHD (D-020) and LICENSE.

**Next:** a fresh session opens **`KICKOFF_M0.md`** (Step 0: branch `m0-burn`) and builds
the burn. Then M0.5 — the thesis slice — before M1 (D-019).

---

## S4 · 2026-08-09 · operator rulings + publication (same session, continued)

**Done:** the operator ruled both open questions — **tier-2 MHD cut for v1** (renderer =
tier-1-derived emissivity + artistic noise; pure-renderer revisit post-M5) and
**LICENSE = MIT** — logged as **D-031**; spec §2.1/§9, ROADMAP, DISPOSITIONS, README
synced (zero open operator questions remain). Repo published **public** at
`https://github.com/bochen2029-pixel/fusion` with description + topics; `runs/` stays
local-only by design (snapshot-preserved).

**Honest state / next:** unchanged from S3 — still zero code; M0 next via
`KICKOFF_M0.md`, then the M0.5 thesis slice before M1.

---

## S5 · 2026-08-09 · M0 — the burn (first product code; same session, continued)

**Done — M0 complete, F-BURN-0 GREEN on attempt 1:**
- **Code (branch `m0-burn`):** `core/` tier-0 burn per spec §2.1 on FUSOR-1 (Bosch-Hale
  range-asserted, alpha lag, Brems + Ar line radiation + Trubnikov-shaped synchrotron,
  Ohmic + volt-seconds, particle + He-ash balance + dilution, **implicit IPB98** with
  the conduction convention, RK4 dt=100 µs, burn sub-cycle 10, quench per-tick,
  Philox4x32-10 stream domains, §2.4 disruption triggers/TQ/CQ, spine quench_precursor
  + SPINE_SHUTDOWN verdict class, M0 floors) · `control/` PID+rad-ff null ·
  `fusor` + `fusor_mc` (Wilson CIs, JSON, golden dump, --verbose-fails) · ctests
  `replay_oracle` / `physics_sanity` / `budget_events` / `golden_check` / `seedcheck` ·
  `trainer/tune_pid.py` (CEM + playoff, seed guard) · tomlplusplus v3.4.0 vendored +
  pinned (sha256 in `third_party/README.md`).
- **Measured numbers (receipt `runs/m0-burn-2026-08-09.md`):** gate **985/1000 = 0.985,
  Wilson95 [0.9754, 0.9909]** (required ≥0.90 / LB ≥0.88), 0 disruptions / 0 spine
  shutdowns; every fail drew H98 < 0.91 (the capped tail); reference point Q = 2.213,
  τ_E = 0.561 s (physics_sanity, inside D-032 bands); ctest **5/5**; goldens ×3
  committed (fnv 6e537c65… / 145a1573… / 9d8d3e0d…); ~55 ms/run serial.
- **Null result (DoD 3):** the PID IS the null at M0 — stated; CEM-tuned with a
  receipted budget, ships as `control/gains_m0.toml`, becomes M2's in-binary arm.
- **Measured control lessons (D-032h, D-033):** objective-without-gate-terms drove pass
  to 0.00 (misaligned-objective, receipted); 48-seed batch selection is ±10% noisy →
  playoff protocol; pure-PID plateau 83%; naive rad-ff HURTS (Q's own denominator);
  the winning strategy is the hot pre-puff hold — the CEM zeroed Krad on its own.
- **Docs:** D-032/D-033 appended; `docs/WHY_FUSION.md` (the ignition thesis, distilled
  from the operator dialogue) + README pointer; KICKOFF_M05 written (DoD 5); ROADMAP M0
  checked + current-kickoff pointer → M0.5. Two invalid-TOML fixes (objective, writs —
  multi-line constructs; caught by the build — P-01's "which parser chokes" answered
  empirically).
- **Commit:** the `m0-burn` merge on `main` (this commit). Snapshot per DoD 6.

**Honest state:** the plant burns, holds Q ≈ 2 through radiative disturbances under a
tuned classical null, and replays bit-identically — **first measured milestone; no organ
beats a null yet** (the PID IS the null), no membrane, no voice, and control reads true
state until M1's statecheck fence. Every §7 number beyond M0's rows remains a target.

**Next:** a fresh session opens **`KICKOFF_M05.md`** (branch `m05-thesis-slice`) — the
thesis slice: tokenizer + floors + text-only triple + the three-arm harness +
F-VERACITY's tape-grader, dry-run before the M1 physics grind (D-019).

---

## S6 · 2026-08-09 · M0.5 — the thesis slice (the mind meets the machine; same session, continued)

**Done — M0.5 complete, dry-run only (no gate claims, per its charter):**
- **Code (branch `m05-thesis-slice`):** `core/` event tap (POD EventRecs riding the
  golden flag; goldens unaffected — 2/2 ctest green) · `membrane/feed` (POD→text
  renderer per events.toml, tick synthesis, composer, truth-table writer, `--null`
  ticks-only mode) · `membrane/tape.h` (hash-chained durable tape, verify green) ·
  **`membrane/shadow.cpp` — `fusor_shadow`**: the Qwen3.5-9B triple on one 32k
  `kv_unified` trunk (auricle soak machinery re-grown: fusion seats, structural
  plant-lane boundaries M-B6, word-grain segmenter on `opt`, fork-stub Sentinel M-M2,
  per-seat caps, 2 s budget drops, A0 log-only) with **three arms** (resident /
  event-twin / poll-twin), F-VERACITY code grader, tape verify · llama.cpp import set
  vendored + pinned (delay-load from C:/llama.cpp — auricle's proven pattern) ·
  scenarios `shadow_demo`/`shadow_demo2` + the operator script + veracity battery
  (`contracts/eval/`).
- **Measured (receipt `runs/m05-thesis-slice-2026-08-09.md`; D-034):** event-twin
  parity at **3.1× token cost** (D-018's narrowed claim, first data) · poll twin absent
  (5/6 questions unanswered) · dial-zero would-fire **85%** (cross-domain
  reproduction), guards contained to 36 emits · veracity: resident 5/6 code-graded ·
  Skeptic caught the planted Q=4 falsehood *and* the accidental T-falsehood, citing the
  stream · Sentinel fired on the floor-violating writ; its **gated would-fire (+1.47)
  on the floor-legal-but-lethal writ is FRESH-3's first datum** · the tokenizer null
  **confabulates** where events give citation (fast death invisible between ticks).
- **Findings → forward:** solicited answers need their own cap lane (M4); the readback
  format is what makes F-VERACITY mechanical (M4); the composed-session script wants
  death-anchored timing (M3 rescope).
- **Docs:** D-034 · KICKOFF_M1 written (DoD 5) · ROADMAP M0.5 checked, pointer → M1.
- **Commit:** the `m05-thesis-slice` merge on `main` (this commit). Snapshot per DoD 6.

**Honest state:** the reactor burns AND is watched — a resident triple that answers
with the tape's numbers, contests falsehoods, and flags dangerous writs, all at dial
zero behind guards, all log-only. Still: no EKF (innovation lane empty), no ghost (the
stub IS the null), no voice, no pacing, emissions non-deterministic. The three-arm
instrument is real and cheap to re-run; the claim's full test needs M1's physics and
M4's raw lanes.

**Next:** a fresh session opens **`KICKOFF_M1.md`** (branch `m1-shape`) — the shape and
the enemy: Grad-Shafranov + passive conductors + γ-from-eigenproblem + diagnostics +
statecheck fence + the EKF (NIS-gated) + §2.4 full + the lethal-legal class + the
offline oracle. Gate: **F-SHAPE-1**.

---

## S7 · 2026-08-09 · M1 slice 1 — the enemy exists (partial-commit; same session)

**Done (branch `m1-shape`; receipt `runs/m1s1-vertical-2026-08-09.md`; D-035):**
core/vertical.h — the linearized vertical instability over the REAL passive-conductor
circuit model (24 filaments, Maxwell mutuals via AGM elliptic integrals, 20-turn VS
pair); **γ⁻¹ = 48.9 ms wall-set REPORTED from the eigenproblem** (4.16 ms shell-removed;
class [10,100] pre-registered, PHY-03's report-not-configure law held); open-loop VDE
dies with the full token chain (gate → TQ(vde) → CQ → disrupt); observer-in-the-loop PD
null holds 20/20 through 60 mm kicks; the [innovation] lane is LIVE (two-tier
self-calibrated gate — a contract change, receipted); ctest 6/6, tier-0 goldens intact.
Three measured control lessons + the null's own limit cycle banked for F-NULL-C.

**Honest state:** M1 is OPEN — slice 1 only. No GS, no transport, no model-based EKF
(kinematic tracker stands in, maximal-handicap-documented), innovation gate v0 (11/20
above-dance kicks), no oracle, F-SHAPE-1 not attempted. The enemy exists and is heard;
the shape comes next.

**Next:** continue `KICKOFF_M1.md` slice 2 (GS + current profile + transport + the
model-based EKF + oracle → F-SHAPE-1), fresh session, branch stays `m1-shape`.

---

## S8 · 2026-08-09 · M1 slice 2a — the GS solver core (partial-commit; same session)

**Done (branch `m1-shape`; receipt `runs/m1s2a-gs-2026-08-09.md`; D-036):**
`core/gs.h` — the Δ* solver, **analytic-verified to 4.0e-6** (Solov'ev, 65×65, fixed
deterministic sweeps) + Picard equilibrium machinery (Ip renormalization → 8.50 MA
exact; axis interior/midplane; li 0.22 under the rectangle-shell BC — the BC's inboard
axis bias measured and stated). ctest 7/7. Slice-2b ownership pre-registered in D-036.

**Honest state:** M1 remains OPEN. The tier-1 stack now has: the enemy (slice 1,
γ-from-eigenproblem + VDE death chain + live innovation lane) and the shape's verified
numerical heart (slice 2a). Missing: shaped/free boundary, q95, transport, model-based
EKF, oracle, F-SHAPE-1 — slice 2b+, next session, same branch.

**Next:** SESSION PAUSE by operator's request — handoff to be written jointly.

---

## S9 · 2026-08-09 · M1 slice 2b first piece — the shape exists (same session; then HANDOFF)

**Done (branch `m1-shape`; receipt `runs/m1s2b-shaped-2026-08-09.md`; D-037):** shaped
fixed-boundary equilibrium on the real FUSOR-1 D-shape via masked SOR over the verified
core — Ip 8.50 MA exact, **Shafranov shift +27 mm outboard**, **q0 1.08 / q95 3.61**
(design est 3.0; floor 2.2 — measurable at last). ctest 7/7. Session handoff written:
`docs/HANDOFF_2026-08-09.md`; KICKOFF_M1 refreshed with slice status.

**Honest state:** M1 OPEN — done: the enemy (slice 1), the verified solver core (2a),
the shaped equilibrium + q95 (2b-1). Remaining: the vertical↔equilibrium MERGE (k_dest
from the decay index), free-boundary + von Hagenow + X-point, transport, model-based
EKF + χ² NIS, the oracle, F-SHAPE-1.

**Next:** a fresh session reads `docs/HANDOFF_2026-08-09.md` + `KICKOFF_M1.md`, branch
`m1-shape`, starting with the MERGE task.

---

## S10 · 2026-08-09 · M1 slice 3 — the MERGE: the enemy derives from the shape (partial-commit)

**Done (branch `m1-shape`; receipt `runs/m1s3-merge-2026-08-09.md` + `runs/m1s3/`
ledgers; D-038):** the handoff's named task. `core/gs.h` decomposes ψ_total into
ψ_self (Green-BC full-rectangle solve over J_φ — `core/rings.h` primitives extracted,
the same ones free-boundary coil tables will use) + ψ_ext (discrete-harmonic to
**5.5e-16**, measured); the decay index and k_dest are REPORTED at the axis:
**n = −1.047, Bz_ext = −0.586 T, k_dest = 3.28e7 N/m — 19× slice-1's calibration**.
The shell became real to hold it: **full 25×25 mutual matrix** (collective screening
43× the diagonal approximation; kwall 1.66e8, margin 5.05), ONE resistivity scalar
calibrated to the pinned τ_wall on the screening eigenmode, m_eff computed (103.8 kg,
180 Hz artifact pin). **γ⁻¹ = 47.1 ms wall-set / 5.26 ms shell-removed** — the
pre-registered class held by real physics where slice 1 held it by calibration.
**γ moves with the equilibrium** (κ 1.85→1.60 ⇒ γ⁻¹ 47→68 ms; k_dest ∝ Ip² exact
0.50/0.50) — the lethal-legal lever is constructible. Null kept (Kpz 5e4/Kdz 200:
20/20 through 60 mm, zmax 0.089; floor/cliff mapped, ledgered). Kicks now tokenize
**20/20** (was 11/20) with no gate change. machine.toml [vessel] kappa_shell pinned;
SimInputs.vd bridge; new replay guard (precomputed vs on-demand bit-identical).
ctest 7/7; tier-0 goldens untouched (vert-off paths bit-identical).

**Honest state:** M1 OPEN — done: the enemy (1), the verified core (2a), the shaped
equilibrium + q95 (2b-1), the MERGE (3). k_dest is rigid-ring-at-axis (stated);
profiles still the fixed linear family. Remaining: free-boundary + von Hagenow +
X-point, the D-024 pipeline schedule, transport (l_i movement), model-based EKF +
χ² NIS, diagnostics + statecheck fence, the oracle, F-SHAPE-1.

**Next:** continue `KICKOFF_M1.md` remaining-work order (free-boundary + von Hagenow
next), branch `m1-shape`, fresh session.

---

## S11 · 2026-08-09 · M1 slice 4 — the free boundary (partial-commit; same day, fresh session)

**Done (branch `m1-shape`; receipt `runs/m1s4-freeboundary-2026-08-09.md` + `runs/m1s4/`;
D-039):** `core/gs_free.h` — coil Green tables, the plasma boundary-response matrix
(the von Hagenow role as the exact discrete Green operator — technique ruling), the
DN-symmetric inverse isoflux fit, and the free-boundary Picard with X-point saddle
refinement, private-flux-safe boundary determination, and first-crossing LCFS rays.
**The coils hold the shape: X-LIMITED at (1.629, ±1.173), κ_ach 1.85 = the pin,
q95 3.68 (fixed-boundary 3.61 — 2% cross-validation), Ip exact, imax_frac 0.98,
k_dest-from-coils vs decomposition ratio 0.78.** The solver acted as fresh eyes on
the machine itself: [coils] units mislabeled (conductor kA, not kAt — corrected +
turns pinned), PF1 belonged at the divertor position, and **the κ_shell 1.5 vessel
could not contain the pinned separatrix — amended 1.9**, moving γ⁻¹ honestly from
47.1 to 19.9 ms (in class; separation bound 5→4 receipted). The amended physics then
exposed a curriculum bug: the vde kick TELEPORTED z, storing artificial screen energy
that rang the regularization artifact and killed ~50% of seeds gain-independently
(forensics ledgered, kick-size-invariant — the tell); replaced by the flux-conserving
slow-manifold `kick_state`. **20/20 at the original 60 mm kick and original gains —
no retune, no scenario change; the disturbance model was the bug.** Kicks tokenize
15/20 (the ring had inflated S10's number — restated); tracker NIS ~40 (the
model-based EKF slice owns χ²). ctest 7/7; tier-0 goldens untouched.

**Honest state:** M1 OPEN — done: the enemy (1), the verified core (2a), the shaped
equilibrium (2b-1), the MERGE (3), the free boundary standalone (4). The sim's
runtime k_dest still derives fixed-boundary (pipeline slice rewires — pre-registered
in D-039). Remaining: the D-024 pipeline schedule + coil L-R dynamics in-sim, the
DST/CR fast solver, transport (l_i movement), model-based EKF + χ² NIS, diagnostics
noise into the burn loop + statecheck fence, the oracle, F-SHAPE-1.

**Next:** the D-024 pipeline (gs_late events, solve@T applies@T+50) OR transport —
whichever the next session's read of KICKOFF_M1 prefers; branch `m1-shape`.

---

## S12 · 2026-08-09 · M1 slice 5 — the seam becomes honest (model-based EKF; same day, fresh session)

**Done (branch `m1-shape`; receipt `runs/m1s5-ekf-2026-08-09.md` + `runs/m1s5/`;
D-040):** handoff item 3. The 4-state model-based Kalman replaces the α-β stand-in —
moment-matched reduction (instantaneous stiffness exact, slow pole bisected onto the
reported γ_wall), D-023's information set consumed in full (plant scattered via the
M0-reserved draws, filter nominal, diagnostics-only inputs with the contract
latencies + the coil-current channel). **χ² NIS acceptance live: [0.5, 2.0]
pre-registered, [1.27, 1.30] measured; kicks tokenize 20/20.** The consistent filter
promptly indicted the plant: the undamped regularization artifact was being pumped
into a permanent 179 Hz ±60–90 mm standing limit cycle (traced — S11's "hold" hid
the smaller version in zmax). Fixed at the source: derivative-filtered rate feedback
+ the regularization's matching damping (ζ 0.7, artifact non-resonant by
construction) — **dance 61.8 → 2.20 mm RMS, kick recovery zmax 0.064, γ⁻¹ 23.2 ms
(damper's touch stated), the original 5× separation bound restored.** First §7.4
datum: vertical step + EKF p99.9 = 2.44 µs / 100 µs tick. Gains unchanged through a
third physics upgrade. ctest 7/7; tier-0 goldens untouched.

**Honest state:** M1 OPEN — done: enemy (1), Δ* core (2a), shaped equilibrium (2b-1),
MERGE (3), free boundary (4), model-EKF + χ² NIS (5). Remaining: the D-024 pipeline
+ DST/CR fast solver (+ covariance carry across re-linearizations), transport (l_i
movement), diagnostics into the burn loop + statecheck fence, the oracle, F-SHAPE-1.

**Next:** the DST/CR fast solver + the D-024 pipeline (the pair that makes the
equilibrium LIVE in the tick loop), branch `m1-shape`.

---

## S13 · 2026-08-09 · M1 slice 6 — the equilibrium goes live (DST + pipeline; same day, fresh session)

**Done (branch `m1-shape`; receipt `runs/m1s6-pipeline-2026-08-09.md` + `runs/m1s6/`;
D-041):** the named fast-solver + multi-rate pair. The DST-I direct solver behind the
same gs_solve interface (exact; print-identical derived numbers; test_gs 5.5×; ~1 ms
tracking iterations) enabled the D-024 pipeline: **the free-boundary equilibrium is
now the RUNTIME k_dest source** (D-039's pre-registration discharged), tracked by one
warm Picard iteration per 200 Hz slot at measured Ip, applied to the observer at +50
ticks with the x/P carry done through the q_s coordinate Jacobian (D-023's clause).
The plant retunes per tick from cached per-ampere geometry (linear-in-Ip — the
frozen-coil law). **rampdown.toml (schema [ramp]) is the first scenario where the
equilibrium moves: k_dest tracked −6.8% over 501 solves, 0 gs_late, MC 20/20 GOOD,
replay bit-stable including pipeline counters.** The ramp exposed and killed two
defects: pipeline same-tick sequencing (cadence silently halved) and — the real find —
**the estimate-fed loop pumping the reduction's off-design-point fast-mode error (136
vs 180 Hz at Ip 7.0; truth-fed clean, NIS 0.99): ZETA_SCREEN 0.7→1.2 overdamps the
artifact by construction — rampdown NIS 93→1.02**, reference suite [1.29, 1.41],
γ⁻¹ 40.8 ms, separation 6.3×. §7.4: retune+step+EKF p99.9 = 3.78 µs. ctest 7/7;
tier-0 goldens untouched (Greenwald floor now tracks live Ip — verified inert there).

**Honest state:** M1 OPEN — done: enemy (1), Δ* core (2a), shaped (2b-1), MERGE (3),
free boundary (4), model-EKF + χ² NIS (5), DST + pipeline (6). Remaining: transport
(l_i movement — the deep lethal-legal lever; profiles still the fixed family),
diagnostics noise into the burn loop + statecheck fence, the oracle (CEM teacher —
owns rampdown ≥95% and the curriculum), F-SHAPE-1.

**Next:** transport + diagnostics + statecheck fence, or the oracle — KICKOFF_M1's
remaining scope; branch `m1-shape`.

---

## S14 · 2026-08-09 · M1 slice 7 — the plasma gains an interior (same day, fresh session)

**Done (branch `m1-shape`; receipt `runs/m1s7-transport-2026-08-09.md` + `runs/m1s7/`;
D-042):** the tier-1 physics completion. 1-D transport (20 nodes, implicit, χ
calibrated in the operator's own discretization → τ_E(ref) = IPB98(ref) EXACTLY;
power degradation backed into χ; two calibration bugs found and receipted), the
evolving current-profile family (α → neoclassical peaking on τ_CR; β_p live into the
GS split; l_i/q0 reported), **state-triggered sawteeth (7 emergent crashes in easy —
the dispersions law's timing-from-state clause honored mechanically)**, burn PIDs on
ECE/interferometer diagnostics, hl_backtransition scenario + b5 draw block +
Sawtooth/HLBack event kinds. **Goldens regenerated — the pre-registered PHY-10 tier
switch.** MEASURED: easy 100/100 (tier-1's honest new baseline), hl 20/20, rampdown
20/20 with **k_dest tracked −11.9% through the ramp (the α/β_p lever adds −5.1% over
fixed profiles)**, vde 20/20, replay bit-stable, ctest 7/7.

**Honest state:** M1 OPEN — slices 1–7 done. The tier-1 stack (spec §2.1) is
physically complete: enemy, equilibrium (fixed+free), pipeline, EKF, transport,
profiles, disruption chain, scenario set (easy · vde_kick · vde_open_loop · rampdown
· hl_backtransition). Remaining for F-SHAPE-1: the statecheck fence (ctest), the
offline oracle (CEM teacher, decorrelated stream) proving the curriculum ≥95%, and
the gate attempt + receipts.

**Next (S15): the fence + the oracle + F-SHAPE-1.** Then KICKOFF_M2.

---

## S15 · 2026-08-09 · M1 slice 8 — the fence, the oracle, F-SHAPE-1 GREEN. **M1 DONE.**

**Done (branch `m1-shape`; receipts `runs/m1-fshape1-2026-08-09.md` + `runs/m1s8/`;
D-043):** the statecheck fence (CTL-14b) — the policy runtime (burn PIDs + rad-ff +
VS PD, the S10–S14 math verbatim) now lives in `control/policy.cpp` behind
`control/obs.h`; poison guards in the true-state headers; the `statecheck` ctest
(include-closure walk + token scan) registered as the hard gate. **The refactor is
bit-identical — the golden fnv is the proof.** The oracle ruling (D-043): the
CEM-tuned null is the constructive solvability proof at M1; the per-seed trajectory
teacher is M2's, pre-registered. **F-SHAPE-1 GREEN, attempt 1:** open-loop VDE death
chain on tape (0.186 s) · oracle 100/100 on easy · vde_kick · hl_backtransition ·
rampdown at gate seeds 900000:900100 (Wilson LB 0.963 each) · EKF NIS [1.29, 1.40]
in the pre-registered [0.5, 2.0], kicks tokenized 20/20 · determinism 8/8 including
the pipeline counters. ROADMAP flipped; **KICKOFF_M2 written** (the cerebellum:
solver-then-distill + PPO, F-NULL-C vs this null; the owed items pre-registered:
LQ null, deep lethal-legal, per-seed oracle, bolometer/rogowski channels).

**Honest state:** M1 DONE per DEFINITION_OF_DONE — gate receipt with reproduce
block; determinism green on committed goldens; the organ IS the null (P-13 stated);
docs synced this commit; KICKOFF_M2 exists; snapshot taken. The day's arc S7–S15:
nine M1 slices, D-035…D-043, all on `m1-shape`, merged to `main` at the gate.

**Next:** a fresh session opens **`KICKOFF_M2.md`** (branch `m2-cerebellum`) — the
cerebellum. The null to beat is receipted and waiting.

---

## S16 · 2026-08-10 · M2 slice 1 — the LQ vertical null (partial-commit; M2 open)

**Done (branch `m2-cerebellum`; receipt `runs/m2s1-lqnull-2026-08-10.md` + `runs/m2s1/`;
D-044, D-045):** the relay round with the M1 instance (25 questions, operator-relayed)
adjudicated first; its rulings pinned as **D-044** (rung ruling: LQG is the hand-built
rung (a) and IS the null, first trained arm = residual-on-LQG, claim phrased "net+null
beats null"; matched-compute = wall-µs p99.9 on the shared budget instrument; F-NULL-C
grid guard ≥98/100 + per-scenario-class decision rule + numeric smoothness metrics;
curriculum designed on EVAL seeds, gate seeds untouched; world-freeze ordering) and the
LQ design as **D-045**. Then the slice: **certainty-equivalent LQG on the EKF's own
4-state model** — test_vertical `lqdump` (per-grid-point (Φ,Bd) from
`VerticalEKF::model_from` on the frozen-coil co-move manifold) → `trainer/lq_design.py`
(pure-stdlib DARE, structured doubling, residual+spectral-radius+decay asserted per
point) → `gains_m2.toml [lq]`; a scheduling `policy_vs` branch behind the SAME fence
(u = −K(k_dest)·x̂, K interpolated); VertObs grew three fence-legal taps (q_s/I_vs
estimates + the observer's applied k_dest, rtEFIT-class stated simplification);
RunResult+fusor_mc completed the objective (z_rms priced under [tracking].z_position_m,
VS effort under [actuators]). **ctest 8/8 — PD path bit-identical (golden fnv unmoved).**

**Measured (eval seeds 800000:800100, N=100; the STRONGER-OF, published both arms):**
objective-derived Q/R at face value has **LQG-classic zero margins** (th=0 disrupts
20/20 pre-kick; design loop stable-by-construction, real loop pumps) — the stable
basin th≈(−3,−5) was CEM-recentered pre-run + receipted as design-stage bracketing.
CEM winner **th=(−3.123, −4.782)**, playoff cost 4.323, all 6 finalists tied 200/200
(flat basin = robustness). **The LQ dominates the PD on the vertical channel:** same
survival (100/100 on vde_kick·rampdown·hl_backtransition), **3.5–16× less VS effort**
(v_eff kick 7e-4→2e-4, ramp 3.58e-3→2.2e-4), **strictly cleaner NIS** (the PD's
off-design pumping — rampdown NIS upper 102.71 worst-seed — vanishes to LQ [0.98,1.04]),
z_rms tie-or-win (ramp 1.56→1.36 mm), equal compute (**both <5 µs p99.9** of the 50 µs
tick: PD 4804 ns / LQ 3895 ns on the shared budget instrument). Kv 1.55 vs the PD's
Kdz 200 — state feedback replaces derivative feedback, so no probe-noise→artifact
pumping (trace+spectrum clean, nis_mean 1.11, no 180/136 Hz line). The **remap**
(`runs/m2s1/remap_*.txt`) confirmed both relay Q8 predictions: EKF-fed Kd cliff stays
~1000 (stale S10 maps right about the in-loop landscape), Kp floor rises at the ramp
point; the M0-shipped (5e4, 200) PD is the single cell clean on all three health axes
at both operating points. easy identity 50/50 both arms bit-identical (tier-0 untouched).

**Null result (DoD 3):** the LQ ships as the composite null's vertical channel
(`gains_m2.toml` = new default; burn PIDs unchanged from M0; PD retained
runtime-selectable). F-NULL-C's baseline is now a near-minimal-effort, clean-lane
controller — the hollow-win-vs-a-limit-cycling-PD path is closed BY CONSTRUCTION.

**Honest state:** M2 OPEN — slice 1 only. The composite null (PID+LQ) exists and is
the strong baseline the cerebellum must beat. No batched envs yet, no trained arm, no
per-seed trajectory oracle, no hardened curriculum. The LQ schedule is k_dest-only
(the co-move manifold; a κ-drift-at-constant-Ip lethal-legal would need the (kd,Ip)
2-D revisit D-045 flagged).

**Next (S17a per the relay Q22 split): the world-freeze slice** — bolometer + rogowski
channels, gs_late anger test, renderer EvKinds 9–11 — then S17b's failure curves +
curriculum-freeze D-entry (the null's knee measured on eval seeds BEFORE any training),
then S18 (fusor_train_env + SimEnv bit-identity + ghost PoC). Branch `m2-cerebellum`.

---

## S17a · 2026-08-10 · M2 — the world-freeze (partial-commit; M2 open)

**Done (branch `m2-cerebellum`; receipt `runs/m2s1a-worldfreeze-2026-08-10.md` +
`runs/m2s1/refreceipt_worldfreeze.txt`; D-046):** the last observation-world changes
before any failure-curve measurement (S14's law: any obs-world change moves
trajectories — so land them all, THEN freeze, THEN measure curves). Four changes:
(a) **the bolometer channel** (`diagnostics.toml [bolometer]`; obs.h) — the tier-1
"true Prad, noise-free, stated" simplification removed; the burn rad-ff input is a
noisy (σ 5%), latched bolometric sum on its own Philox key. The null runs Krad=0 so
it is HONEST-BUT-INERT (proven live at Krad=0.5: easy cost 2.678→2.656); easy goldens
BIT-IDENTICAL. (b) **rogowski noise on the OBSERVER's Ip** (`[rogowski]`, σ 0.5%, EMA
τ 10 ms) — the relink reads it; the plant's Ip and the rtEFIT tracking solve stay
truth (relay Q20). ~0.035% at relink; NIS holds [1.29,1.40]. (c) **the gs_late anger
test** (`SimInputs.reval_bound_m`) — the 60 mm kick fires exactly 1 mid-flight
collision (fnv-stable), a 2 mm bound fires 5; the late path proven to fire+count
deterministically before F-KEEPUP leans on it. (d) **renderer EvKinds 9–11**
(`membrane/feed.cpp`) — GsLate/Sawtooth/HLBack were silently dropped; now rendered
(GsLate on `sys`, per spec §4), guarded by a new **`feed_render`** ctest. ctest 8/8 →
**9/9**; golden/replay/statecheck/NIS all green.

**Measured (eval seeds 800000:800100, N=100; re-receipt on the changed world):**
**100/100 both arms × all four scenarios**, matching S16 to <1% (the world change is
honest but small — bolometer inert, rogowski EMA-tiny); the live deltas confirm the
channel (rampdown PD worst-seed NIS 102.71→102.1; vde_kick LQ z_int 0.00273→0.002734).
The LQ's dominance over the PD is intact (3.5–16× less VS effort, clean NIS).

**Null result (DoD 3):** unchanged — the composite null (PID+LQ, `gains_m2.toml`) is
the shipping baseline; the world it lives in is now the honest one (noisy radiation +
current channels), and it still holds.

**Honest state:** M2 OPEN. The observation world is FROZEN (all channels honest, the
renderer complete, the late path proven, determinism intact). No failure curves yet,
no hardened curriculum, no trained arm. Deferred-stated: the diagnostics.toml→loaded
DiagnosticsCfg unification (channels are still hardcoded contract-mirrors), the
tracking-solve rogowski variant.

**Next (S17b): the failure curves + the curriculum-freeze D-entry** — the null's knee
measured on EVAL seeds (kick psychometric sweep, combined events, the deep
lethal-legal), pre-registered target bands, the hardened F-NULL-C grid frozen BEFORE
any training (relay §2/§3). Then S18 (fusor_train_env + SimEnv bit-identity + ghost
PoC). Branch `m2-cerebellum`.

---

## S17b · 2026-08-10 · M2 — the failure curves + the curriculum FREEZE (partial-commit; M2 open)

**Done (branch `m2-cerebellum`; receipt `runs/m2s1b-curriculum-2026-08-10.md` +
`runs/m2s1/{psycho_sweep,curriculum_measure}.txt`; D-047):** the credibility hinge — the
null's failure curves measured on EVAL seeds (800000:899999), the F-NULL-C curriculum
FROZEN, gate seeds untouched. New instruments: `RunResult.margin_min` (the lethal-legal
spine — min kwall/k_dest over a run) + fusor_mc `--kick-mm` sweep affordance; ctest 9/9
(observer-only). Five new scenarios authored + determinism-spot-checked +
committed: `lethal_legal_{shallow,deep}`, `kick_during_ramp`, `puff_kick`,
`sawtooth_storm`.

**THE HEADROOM MAP (measured, composite null gains_m2, N=100 eval):**
- **Isolated kick has ZERO headroom** — both nulls survive to 185 mm (2 mm from the wall)
  at 100/100. The M1 instance's 85-100 mm knee estimate was ~2x wrong (measured, as they
  asked). A load-bearing negative result: the psychometric band is a no-regression guard.
- **The DEEP lethal-legal is the money scenario** — a floor-legal 0.5 MA/s rampdown walks
  the margin to **1.04** (Ip 4.5) and a 30 mm kick lands at the bottom: **LQ survives
  36/100 vs the old PD's 11/100 (3.3x)**. The LQ's anticipatory schedule already captures
  much of the headroom the M1 instance predicted for the net — and 64% stays open (the
  schedule tracks the margin but can't PRE-position for the kick; that's the net's edge).
- **The LQ's dominance is starkest on the hard scenarios**: shallow lethal-legal both
  98/100 but PD thrashes (v_eff 0.553, NIS 333) vs LQ glides (v_eff 0.0023, NIS 1.01) —
  240x; kick_during_ramp both 100/100 but PD NIS 152 vs LQ 1.05. Survival ties are
  smoothness routs.
- puff_kick 96/100 (burn-radiative deaths; beta_p->k_dest is scaling-grade — a burn-head
  guard); sawtooth_storm 100/100 (weak coupling — honest no-headroom probe).

**THE FROZEN CURRICULUM (9 scenarios, pre-registered before any training):** guards (net
>= 98/100) = easy, vde_kick, rampdown, hl_backtransition, kick_during_ramp,
sawtooth_storm; separation = lethal_legal_deep (PRIMARY: net wins iff survival Wilson-LB
> 0.458, ~55/100), lethal_legal_shallow (smoothness), puff_kick (burn-head). The final
F-NULL-C attempt re-measures all nine on GATE seeds once.

**Null result (DoD 3):** the composite null (gains_m2) is the frozen baseline; its
measured rates ARE the bar. The isolated-kick negative result and the LQ-beats-PD-3.3x
lethal-legal result are both receipted, either way.

**Honest state:** M2 OPEN. The curriculum is frozen; the world is frozen (S17a); the null
is the strong baseline (S16). The headroom is narrower and more concentrated than the
relay's pre-measurement hypothesis (all in the deep lethal-legal's anticipatory regime) —
the honest, measured map. Nothing trained yet.

**Side survey (operator-requested):** `C:/nuclear` surveyed — a sibling estate project
(Trinity implosion Monte-Carlo sim, method-rich, NO controller); transferable M2 lessons
captured in memory `reference-nuclear-cousin` (matched-compute stats + variance-estimator
trap, sim-vs-wall-time queue foot-gun for S18's trainer, typed reward + synthetic null
test, generated verification oracle, no-op-path fixtures).

**Next (S18): fusor_train_env + the SimEnv bit-identity refactor + the ghost PoC** — the
batched-env runner (CPU-first), run_sim refactored to a stepwise SimEnv proven
bit-identical (the relay Q13 surgery), the parity ctest, and the ghost fork PoC (relay
Q23, promoted). Branch `m2-cerebellum`. Mind the sim-vs-wall-time trap (nuclear ADR-020).

---

## S18 · 2026-08-10 · M2 — the stepwise plant + batched envs + the ghost PoC (partial-commit; M2 open)

**Done (branch `m2-cerebellum`; receipt `runs/m2s18-simenv-2026-08-10.md`; D-048):** the
foundation the training stands on. run_sim's ~330-line one-shot loop body EXTRACTED into
`core/sim_env.h` `struct SimEnv` (state in members, `step()` = one tick, run_sim = a thin
`reset/while(step)/finish` wrapper). **The extraction is BIT-IDENTICAL — the golden fnv did
not move** (golden_check + replay_oracle + the new parity gate all green; the S15 fence
pattern: proven by the byte-stream). Fence held: sim_env.h is a true-state header (added to
statecheck FORBIDDEN_HEADERS), includes policy.h LAST so PolicyState is a value member
without arming the poison. Built on it:
- **fusor_train_env** — the batched runner (spec's FOURTH one-dynamics-source consumer,
  D-011): parallel SimEnv rollouts sharing ONE FreeContext (relay Q13 law), CPU-first.
  MEASURED (16 threads): easy flat-top **10.76 Mtick/s** (~1.5 µs/tick/thread, matching the
  relay §4 estimate); vde_kick vertical+pipeline 1.46 Mtick/s.
- **parity_train_env** ctest (relay Q18): stepwise==one-shot AND threaded==serial,
  bit-exact on vde_kick/rampdown/easy (thread-safety rides gs_solve's thread_local scratch
  + the const shared context — the relay Q13 landmine inventory, clean).
- **ghost_fork** ctest (relay Q23 promoted): the fork is `SimEnv g = plant;` — (i) two
  forks run bit-identically to the plant (F-GHOST criterion i, same fnv e6ddf198…); (ii) a
  fork with VS off from T takes the kick and DISRUPTS (z_max 0.190) while the plant
  survives — the M3 ghost's counterfactual-via-command mechanism, de-risked warm.
ctest 9/9 -> **11/11**.

**Null result (DoD 3):** N/A this slice — no organ shipped; the SimEnv IS the plant,
proven bit-identical to the gated one. The composite null still runs inside it unchanged.

**Honest state:** M2 OPEN. The plant is now stepwise + batched + forkable, bit-exact and
thread-safe, ~11 Mtick/s flat-top on 16 cores. Deferred to S19: the net's action-injection
hook (SimEnv is the seam; step() currently runs the in-binary null), the per-seed MPPI/CEM
teacher, the CUDA port of step(). Watch the sim-vs-wall-time trap in the trainer/queue
(nuclear ADR-020, memory reference-nuclear-cousin).

**Next (S19): the teacher + the trained arm** — the per-seed MPPI/CEM trajectory oracle
(decorrelated teacher stream, relay Q16), solver-then-distill onto the residual-on-LQG net
(relay Q1/Q15), the action-injection API, and the export byte-golden + KAT (relay Q18). The
frozen curriculum (S17b) is the target; the SimEnv (S18) is the substrate. Branch `m2-cerebellum`.

---

## S19 · 2026-08-10 · M2 — the net inference substrate (partial-commit; M2 open)

**Done (branch `m2-cerebellum`; receipt `runs/m2s19-net-2026-08-10.md`; D-049):** the
residual-on-LQG runtime BEFORE the training — the ship-runtime-before-you-train pattern a
third time (S16 the LQ, S18 the plant, now the net). `control/net.h` `NetMLP`: a flat-weight
MLP (trainer/export_net.py exports the byte format), CPU, fixed-order dot products
(/fp:strict, bit-reproducible), depth pinned at 2 hidden layers (spec §3), width in the file
header (v1=64). **Residual-on-LQG** (relay Q1/Q15): policy_vs runs u = -K(kd)(x_hat with a
net Z-REFERENCE OFFSET, +/-10 mm) — the offset, not a weak raw-voltage residual (the LQG
makes it the near-rail phase lead). Obs (relay Q19): z/v/q_s/i_vs estimates + observer k_dest
+ the RAW innovation nu/sqrt(S), 3-frame stacked = 18 in. Fence-legal (net.h std-only, added
to statecheck FENCE_TUS).

**Measured:** KAT — the exported weights load into net.h and reproduce the exporter's output
to **9e-9** (float32; Python<->C++ agreement + regression golden); zero weights -> 0; **a
zero-net sim run is BIT-IDENTICAL to the pure null** across 3 seeds (the residual path is
zero-cost at zero weights). Budget (§7.4, relay Q18): retune+step+EKF+policy_vs[LQ+net]
p99.9 = **11.2 us** of the 50 us tick (net adds ~7 us; 4.5x headroom — the FORWARD_NOTES §5
estimate confirmed). ctest 11/11 -> **12/12** (net_runtime).

**Null result (DoD 3):** N/A — no organ shipped; the net runtime is proven zero-cost at zero
weights (== the null). The composite null remains the baseline; the trained net is S20+.

**Honest state:** M2 OPEN. The net's inference path, export, KAT, budget, and fence are all
in place and verified — trained weights slot in with no plumbing risk. Deferred to S20: the
MPPI/CEM teacher (its decorrelated-stream / clairvoyance-trap subtlety, D-011, warrants
fresh context — the M1 instance's "budget it as a real slice"), then solver-then-distill,
then the burn residual head + the e2e stretch arm.

**Next (S20): the teacher** — the per-seed MPPI/CEM trajectory oracle on the SimEnv fork
machinery (relay Q16: vertical 80 ms/5 ms segments, CEM pop 64), DECORRELATED teacher stream
(D-011 — the fork shares the plant's stream, but the teacher must re-seed rollout draws from
stream 1; the clairvoyance-cannot-beat-chance ctest). Then S21 distill onto the net (S19
substrate) + PPO polish + the F-NULL-C attempt against the frozen curriculum (S17b). WATCH
the sim-vs-wall-time trap (nuclear ADR-020). Branch `m2-cerebellum`.
