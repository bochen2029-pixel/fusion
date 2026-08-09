# DISPOSITIONS — the v0.2 ledger (every finding, all six instruments)
**2026-08-09 · v0.2 revision session.** Codes: **FIXED** (in v0.2 spec §/contract),
**PRE-APPLIED** (S1's in-place patch, carried into v0.2), **SCHEDULED** (assigned a
milestone gate/deliverable), **RISK** (accepted-as-risk, reason given), **OPQ** (operator
question, awaiting ruling), **NOTED** (cosmetic/informational, folded). Spec § references
are to `FUSION_ARCHITECTURE_v0.2.md`; contracts by filename.

## QC-PHYSICS (29)

| ID | disposition |
|---|---|
| PHY-01 | FIXED — machine.toml (FUSOR-1, synthetic; every §-number derives from it) |
| PHY-02 | PRE-APPLIED (D-016) + FIXED — γ derived from vessel L-R, reported from eigenproblem; 10 kHz re-justified as samples-per-e-folding (§0, machine.toml [vessel]) |
| PHY-03 | FIXED — 24 passive filaments, full mutual-inductance matrix, built general (§2.1 tier-1) |
| PHY-04 | FIXED — ghost mode declared (frozen-eq tier-0 + linear vertical); F-GHOST rescoped to ghost-vs-ghost memcmp + bounded divergence + tape ordering (§5.4, §8; D-026) |
| PHY-05 | FIXED — dispersions.toml + scenario _TEMPLATE [pass]; seed→stream mapping in objective.toml [seeds] |
| PHY-06 | FIXED — §2.4 disruption model + events.toml [terminal] (ships together with the token, per MEM joint fix) |
| PHY-07 | FIXED — particle balance + He ash + dilution in tier-0 (§2.1) |
| PHY-08 | FIXED — impurity line radiation P_line with tabulated L_z (§2.1; machine.toml [radiation]) |
| PHY-09 | FIXED — implicit IPB98 convention stated; κ_a pinned in machine.toml; H98 exposed (§2.1) |
| PHY-10 | FIXED — tier-1 authoritative from M1, tier-0 demoted to source terms + reduction; tier switch invalidates goldens (§2.1) |
| PHY-11 | FIXED — solver named (DST/cyclic-reduction + von Hagenow); GS pipelined-by-construction, sim waits if wall-late, gs_late event; F-KEEPUP on aggregate pacing (§2.1, §7.2, §8) |
| PHY-12 | FIXED — vde_kick + rampdown + hl_backtransition in §2.3 + dispersions.toml |
| PHY-13 | FIXED — breakdown scoped OUT; runs start at machine.toml [initial_state]; §12 states it |
| PHY-14 | FIXED — parameterised p′/FF′ with l_i, β_p as 0-D states; sawtooth resets q₀ (§2.1) |
| PHY-15 | FIXED — Ohmic heating + volt-second budget (§2.1; machine.toml cs_flux_swing) |
| PHY-16 | FIXED — _TEMPLATE [pass] criteria; F-BURN-0 carries duration/disturbance/fraction/CI (§8) |
| PHY-17 | FIXED — toggle re-timed 150–300 ms, derived from machine γ (§6) |
| PHY-18 | FIXED — wall_reflectivity in machine.toml [radiation], cited §2.1 |
| PHY-19 | FIXED — E_α=3.52 pinned, one-pole slowing-down lag, confined fraction (machine.toml [alphas]) |
| PHY-20 | PRE-APPLIED (§7 row re-derivation) + FIXED — tier-2 pending D-020; telemetry at 64³ shared-memory (§6) |
| PHY-21 | FIXED — troyon_C=2.8 defined in machine.toml; floor at 0.90 frac with margin (floors.toml) |
| PHY-22 | FIXED — spine_gates.toml: predicates, dwell, hysteresis, responses, receipts, false-positive budgets |
| PHY-23 | PRE-APPLIED (D-016/S1 Greenwald single-sourcing) + FIXED — comment now "empirical disruptive boundary" (floors.toml) |
| PHY-24 | FIXED — RK4 named, compile-time sub-cycle ratio, quench terms on the 100 µs path (§2.1) |
| PHY-25 | FIXED — activates_at keys on every floor group (floors.toml) |
| PHY-26 | FIXED — n_ramp_max_e20_per_s, measurand = line-averaged n̄_e (floors.toml, machine.toml) |
| PHY-27 | FIXED — ramp floor derived from FUSOR-1 (0.85 MA/s under machine 1.0); MC wall-time noted in scenario durations |
| PHY-28 | FIXED — Bosch-Hale validity range asserted in code (§2.1) |
| PHY-29 | FIXED — fork snapshots mutable state only; static tables shared (§2.2 r6) |

## QC-CONTROL (39)

| ID | disposition |
|---|---|
| CTL-01 | FIXED — with PHY-03 (§2.1); γ reported per equilibrium, null gains schedule on it |
| CTL-02 | PRE-APPLIED (D-012) — policy on CPU fixed precision; carried §3 |
| CTL-03 | FIXED — per-cluster Mahalanobis + dwell + refractory + storm collapse + hard shaper (events.toml v1) |
| CTL-04 | FIXED — the M2 ladder, rung (a) gate-sufficient (§3; D-025) |
| CTL-05 | PRE-APPLIED (D-011) + FIXED — stream domains in dispersions.toml; clairvoyance ctest named (§2.2 r4) |
| CTL-06 | FIXED — MPPI/CEM is OFFLINE only; struck from §1 row and §7; D-006 rationale rewritten (§3; D-024) |
| CTL-07 | FIXED — objective.toml, one objective three consumers |
| CTL-08 | FIXED — F-NULL-C v2: matched wall-clock+VRAM, CRN, code-enforced seed partition, Wilson+bootstrap split, lexicographic rule, null-win branch (§8; objective.toml) |
| CTL-09 | FIXED — spine_gates.toml (moved out of floors; full predicates) |
| CTL-10 | FIXED — SPINE_SHUTDOWN failure class in MC taxonomy + objective terminal costs + third F-NULL-C metric (spine_gates.toml, objective.toml, §8) |
| CTL-11 | FIXED — EKF fully specified: model, info set (FRESH-4), covariance carry, NIS χ² acceptance at M1, plumbing to policy+tokenizer (§3; D-023) |
| CTL-12 | FIXED — dispersions.toml with observable/blind split + seed law before any tuning |
| CTL-13 | FIXED — writs.toml v1: id/rev/supersedes/ttl/cancel + spine-side objection window in ticks |
| CTL-14 | FIXED — §2.4 exists; named an M2 prerequisite via F-NULL-C metrics (§8) |
| CTL-14b | FIXED — statecheck ctest, hard M1 gate + grep gate (§3, §11) |
| CTL-15 | FIXED — §0(a) restated: necessity claim moved to multi-objective/fault-recovery territory; null acknowledged strong |
| CTL-16 | FIXED — CPU/AVX2 p99.9 budget (§3, §7.4) |
| CTL-17 | FIXED — existence proof scoped: size+rate yes, pipeline no; MPO named as rung (c) (§0, §3) |
| CTL-18 | FIXED — MLP + 3-frame stack decided, default pinned (§3) |
| CTL-19 | FIXED — DAgger named for any distill rung (§3) |
| CTL-20 | FIXED — fourth consumer in §2.2 r3 + parity_train_env ctest |
| CTL-21 | FIXED — innovation vector to the policy; hide-the-flags acceptance test (§3) |
| CTL-22 | FIXED — autonomy dial A0<A1<A2<A3 in risk order; effective = min(class, source, global) (writs.toml; §4.2) |
| CTL-23 | FIXED — ordering enumerated, envelope params enumerated, EMERGENCY_SHUTDOWN expressible (A1), schedule writ type added (writs.toml) |
| CTL-24 | FIXED — units in keys/typed units table; parser rejects unitless (writs.toml, floors.toml) |
| CTL-24b | FIXED — acceptance receipts on sys lane; commit = the accept receipt (writs.toml [receipts]; §4.2) |
| CTL-25 | FIXED — diagnostics.toml (σ, bias walk, drift, latency, dropout, quantization) |
| CTL-26 | FIXED — per-timescale teachers, piecewise-constant segments, info set stated (§3) |
| CTL-27 | FIXED — §7.4 tick budget table, p99.9 binding, measured at M3 |
| CTL-28 | FIXED — MPC downgraded to later comparison; null = PID+LQ (D-020; §3) |
| CTL-29 | FIXED — null CEM-tuned, declared budget, receipted at M0 (§3; F-BURN-0) |
| CTL-29b | FIXED — null ships in-binary as permanent arm (§3; §11) |
| CTL-30 | FIXED — sysid removed from §3 (parked, honest restatement required if revived; D-020) |
| CTL-31 | FIXED — MCTS+sysid parked with named-null requirement (§9 parked; D-020) |
| CTL-32 | FIXED — M2 gate gains p99.9 latency + byte-golden + KAT (§3, §9-M2) |
| CTL-33 | FIXED — PPO only with KL-to-teacher; MPO the proven alternative (§3) |
| CTL-34 | FIXED — F-BURN-0 requires holding Q through a scripted perturbation on disjoint seeds (§8) |
| CTL-35 | FIXED — "free-tail razor" expanded in glossary (CLAUDE.md) |
| CTL-36 | FIXED — one default architecture pinned; ranges live in the trainer (§3) |

## QC-MEMBRANE (34)

| ID | disposition |
|---|---|
| M-B1 | PRE-APPLIED (S1 events.toml rebuild) + FIXED — one budget table, 300 s ticks, budget_events ctest named (§10 targets) |
| M-B2 | PRE-APPLIED (D-015) + FIXED — 2 s hard budget + sampler abort + [sys] drop receipt (§4.3) |
| M-B3 | FIXED — rev_observed + formed_tick + governor staleness gate (writs.toml; §4.3) |
| M-B4 | PRE-APPLIED (D-013) + FIXED — commit = governor-accept; operator path = deterministic writ parse at the governor, never through a seat; ghost verdict a precondition inside the A2 window (§4.2, §5.4) |
| M-B5 | FIXED — templated deterministic interjection on fork-verdict + TTS pre-warm; ~0.4 s into a 3 s window (§5.4; chain D §7.5) |
| M-B6 | FIXED — plant-lane boundaries structural; segmenter governs human lanes only (§4) |
| M-B7 | FIXED — all three room guards law; [cascade] breaker in events.toml; caps pre-registered (§4) |
| M-B8 | PRE-APPLIED (D-014) + FIXED — headset v1 + render-clock self-speech gate + opv attribution rule (§5.1–5.2) |
| M-B9 | FIXED — M3 gate produces labeled ledger + quarantined 40-item holdout; --review valve ported (§8 F-INSTINCT, §9-M3) |
| M-M1 | PRE-APPLIED (D-015, 921/hr) + FIXED — cited with its arithmetic per D-018 |
| M-M2 | FIXED — Sentinel emit fork-gated (§4 seats) |
| M-M3 | FIXED — [terminal]/[organ]/[phase] complete the vocabulary; ships with §2.4 |
| M-M4 | FIXED — coalesce-with-receipt overflow; "band should dwell" replaced by dwell_windows mechanism (events.toml) |
| M-M5 | PRE-APPLIED (S1) — prose cites the contract; 0.95 stands (floors.toml) |
| M-M6 | FIXED — molt-on-calm + shed-oldest-plant-verbatim during crisis (§4) |
| M-M7 | FIXED — crisis boundary coalescing (300 ms min) + M3 fire-rate report is a load test with duty ceiling (§4.1, §9-M3) |
| M-M8 | FIXED — §11 row remapped to statecheck + enforced-by column added |
| M-M9 | FIXED — tokenizer null + Sentinel/ghost null declared, scored on lethal-legal class (§4.1, §8, contracts/objective note) |
| M-M10 | FIXED — "adapted" not "verbatim"; abandon-or-restart at clause v1; free resume → §12 v2; partial→pause is new budgeted work (§5.1–5.3) |
| M-M11 | FIXED — POD structs over SPSC ring; membrane renders; no sprintf on the tick (§4.1) |
| M-M12 | FIXED — L12 imported verbatim; operator is the calibrated grader; no model grades (§8 F-PRESENCE-F) |
| M-M13 | FIXED — vocabulary freeze at M3, append-only, human-lane emphasis (events.toml header; §4.1) |
| M-M14 | FIXED — acceptance receipts (writs.toml [receipts]; §4.2) |
| M-N1 | FIXED — "lane-H" removed; Sentinel watches opv partials (§4) |
| M-N2 | FIXED — Stage-0 dial prior (−0.9) carried into the M0.5/M3 rig config note (KICKOFF_M0 successor docs) |
| M-N3 | FIXED — rung-cap caveat carried (§4 engine para) |
| M-N4 | FIXED — one budget chain: grain 1–3 s ⊃ judgment 2 s hard ⊃ emit-gen 1.5 s p95 target (§4.3, §7.5) |
| M-N5 | FIXED — §1 table is multi-rate (v0.2 §1) |
| M-N6 | FIXED — probe cited as 65–70 ms (§5.4) |
| M-N7 | FIXED — §11 L-SILENCE row corrected |
| M-N8 | FIXED — surprisal tap labeled unvalidated-as-signal; no organ drinks unvalidated (§4, §12) |
| M-N9 | FIXED — Sentinel branch teardown at commit stated (§4 engine para) |
| M-T1 | FIXED — 921/hr single-sourced with arithmetic (§5.2) |
| M-T2 | FIXED — engine para cites receipts; remaining bare claims carry paths in v0.2 |

## QC-VOICE-RT (18)

| ID | disposition |
|---|---|
| V-01 | PRE-APPLIED (D-014) + FIXED — both heads: headset + render-clock gate + opv attribution + speakers-mode negative test in F-VOICE (§5, §8) |
| V-02 | FIXED — GPU arbitration doctrine §7.2; ASR/TTS/policy CPU; tick-jitter at M3; gs_late event |
| V-03 | PRE-APPLIED (D-013) + FIXED — A2 objection window spine-side in ticks; writs.toml v1 supersedes the H0/H1 line both places |
| V-04 | PRE-APPLIED (S1 unification) + FIXED — one backend, two model dirs, CPU; GPU ASR row deleted (§5.1, §7.1) |
| V-05 | FIXED — chunk-ladder WER receipt at M4 + readback/confirm + keyword-boosted writ-lane decode (§5.1) |
| V-06 | PRE-APPLIED (S1 §7 note) + FIXED — tier-2 row pending D-020; solver-working-set language retired with it |
| V-07 | FIXED — telemetry row 42 MB/s, 64³@10 Hz shared-memory, renderer never backpressures (§6, §7.5) |
| V-08 | FIXED — prescribed-config-only §7.1; renderer provenance honest (measured desktop vs estimated renderer) |
| V-09 | FIXED — chain C budgeted (≤1.5 s p95); first-audio re-anchored to first clause boundary; clause-grain in tts.h (§5.2, §7.5) |
| V-10 | FIXED — queued-PCM cap ≤60 ms in tts.h; five-term sub-budget in §7.5; wired named |
| V-11 | FIXED — F-VOICE rig, t₀/t₁, distributions, device-named receipts, unit of account (§8) |
| V-12 | FIXED — offline ruled (D-024); §1 and §7 follow |
| V-13 | FIXED — with M-N1 |
| V-14 | FIXED — thread/affinity table §7.3; hot-path law inherited into CLAUDE.md conventions |
| V-15 | FIXED — tts.h contract §5.2 (D-028) |
| V-16 | FIXED — splice unit = last clause boundary, recorded in ledger (§5.2) |
| V-17 | FIXED — VAD + duplex governor own rows in §7.3/§7.5 |
| V-18 | FIXED — Silero named, 32 ms frames, 1–3 frame latency (§5.2, §7.5) |

## QC-REPO-PROCESS (26)

| ID | disposition |
|---|---|
| P-01 | FIXED — tomlplusplus v3.4 named + third_party plan in §10 + KICKOFF_M0; third_party/dl/ gitignored |
| P-02 | FIXED — contracts/scenarios/_TEMPLATE.toml shipped |
| P-03 | FIXED — build block in KICKOFF_M0 + CLAUDE.md |
| P-04 | FIXED — units-in-keys floats (floors.toml v1) |
| P-05 | FIXED — dir keys on every limit (events.toml, floors.toml) |
| P-06 | PRE-APPLIED (S1) — prose cites contract (0.95) |
| P-07 | PRE-APPLIED (S1 events rebuild) — 300 s ticks, one budget table |
| P-08 | PRE-APPLIED (S1 CLAUDE.md carve-out) + FIXED — §10 states ui/-only JS/TS/Rust + npm/cargo allowance |
| P-09 | FIXED — golden format defined (binary state dump + hash, .gitattributes-protected); KICKOFF_M0 item creates them |
| P-10 | FIXED — branch convention m<N>-<slug> in CLAUDE.md; KICKOFF_M0 step 0 |
| P-11 | FIXED — glossary completed (SYNCYTIUM + tokamak + estate terms) in CLAUDE.md |
| P-12 | PRE-APPLIED (S1 .gitattributes) — verified present |
| P-13 | FIXED — DoD clauses reworded (gate criteria incl. named falsifier where one exists; null clause scoped to learned organs) + M0/M1 now HAVE falsifiers (F-BURN-0/F-SHAPE-1) |
| P-14 | FIXED — .gitignore gains node_modules/, _*_wt/, third_party/dl/, out/, Testing/, artifacts |
| P-15 | FIXED — standing "current kickoff" line in ROADMAP header |
| P-16 | FIXED — §10 lists docs/ and third_party/ |
| P-17 | FIXED — tools/snapshot.ps1 + <ts> format yyyyMMdd-HHmmss defined in DoD |
| P-18 | RISK (house style, deliberate) — runs/ stays gitignored; mitigation: snapshot script now exists + receipts summarized in SESSION_LOG; revisit if a receipt is ever lost |
| P-19 | FIXED — SESSION_LOG header names the coding-entry slots (receipts, SHA, numbers, null result) |
| P-20 | FIXED — dashboard consistently stretch (v0.2 §9-M0 omits it; ROADMAP/KICKOFF agree) |
| P-21 | FIXED — fusor_mc separate binary ruled; CLI sketch in KICKOFF_M0 |
| P-22 | FIXED — v0.2 §9-M1 matches ROADMAP scope (superset resolved into the spec) |
| P-23 | PRE-APPLIED (S2 noted) — v0.1 footer left frozen-wrong by design; v0.2 enumerates decisions, never ranges |
| P-24 | NOTED — falsifier suffix convention left as-is (names used consistently; renaming risks drift) |
| P-25 | FIXED — KICKOFF_M0 says "append your session entry" |
| P-26 | PRE-APPLIED (S1 DoD null-win) — carried; F-NULL-C names the losing branch |

## FRESH_EYES (the second instrument)

| ID | disposition |
|---|---|
| FRESH-1 | FIXED — F-PRESENCE-F three arms, information sets pre-registered (§8; D-018) |
| FRESH-2 | FIXED — F-VERACITY, code-graded, gates M4 (§8; D-018) |
| FRESH-3 | FIXED — lethal-legal scenario class (_TEMPLATE class key; §2.3; ghost null scored on it) |
| FRESH-4 | FIXED — EKF information set pre-registered (§3; D-023) |
| FRESH-5 | FIXED — health 2.5σ + 3-of-4 dwell (events.toml [health]) |
| FRESH-6 | FIXED — session input-tape (writs.toml [input_tape]; §2.2 r4) |
| FRESH-7 | FIXED — CPU core table §7.3 |
| FRESH-8 | FIXED — durable tape inherited (auricle durable.h pattern; §4 engine) |
| FE-17 | FIXED — depths tier cut (D-020; §1 three tiers) |
| FE-19 | FIXED — ghost overlay render path defined (§6) |
| FE-20 | FIXED — mind_died watchdog receipt (events.toml [organ]; §4 lanes) |
| FE-21 | RISK — no CI service on a single-box repo; mitigation: ctest list named in §10, DoD demands green suite per milestone |
| FE-25 | OPQ — LICENSE choice is the operator's (company repo); flagged, zero-cost to add |
| FE-26 | FIXED — consent floor reflected: single-operator headset mic; anything recording others inherits auricle doctrine (§12 implied by CLAUDE.md #7; noted in KICKOFF_M0 mistakes list) |
| D1 | FIXED — kickoff de-trapped (S2 commit); machine.toml executes the synthetic-machine rule |
| D2 | FIXED — post-M-M2 null statement (§4 seats; ghost null on lethal-legal class) |
| D3 | FIXED — §7.1 renderer provenance honest |
| D4 | FIXED — D-018 owns the narrowed claim in §0(b)/§4 boundary law |
| P-1..P-8 (premise) | NOTED/FIXED — P-1/P-2 → D-018+three arms; P-3 → F-VERACITY; P-4 → §0(c) + D-019 sequencing; P-5 → lethal-legal class; P-6 → EKF info set; P-7 → noted in D-019 rationale; P-8 → depths cut |
| C-1..C-22 (comprehension) | FIXED — glossary completions in CLAUDE.md (tokamak + estate halves); remaining parent-repo deep-dives are named read-gates, not assumed knowledge |

**Open operator questions: none.** Both were ruled by the operator on 2026-08-09
(D-031): tier-2 MHD **cut for v1** (renderer = tier-1-derived emissivity + artistic
noise; pure-renderer revisit post-M5) and LICENSE = **MIT**. Every finding from all six
instruments is dispositioned. — *v0.2 session, 2026-08-09*
