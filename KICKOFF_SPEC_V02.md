# KICKOFF — SPEC v0.2 · the QC-swarm revision (bootstrap for the next session)

**M0 is suspended until this session's work exists.** You are producing
`FUSION_ARCHITECTURE_v0.2.md` (v0.1 stays beside it, untouched from here on) plus the
design artifacts the QC swarm proved missing. No product code this session.

## Read first, in order

`CLAUDE.md` → `docs/qc/QC_CONSOLIDATED.md` (the eight blocker clusters + what was already
patched in-place — do NOT re-fix those) → the five lane reports
`docs/qc/QC_{PHYSICS,CONTROL,MEMBRANE,VOICE_RT,REPO_PROCESS}.md` **in full — they are the
line-item worklist** → `FUSION_ARCHITECTURE_v0.1.md` (as QC-patched) → `DECISIONS.md`
D-001…D-017 → `contracts/` → `SESSION_LOG.md` tail. Optionally replay the room:
`python C:/Intercom/intercom.py replay --room fusion-qc --limit 200` (forward slashes —
Git Bash eats backslashes). Then confirm: (a) the eight clusters, (b) what D-011…D-017
already settled, (c) your deliverable list below, (d) the multi-session discipline
(SESSION_LOG append + same-commit docs + rewrite KICKOFF_M0 + write KICKOFF_M0's successor
note before ending). Then work.

## Deliverables (each traceable to finding IDs; disposition every BLOCKER, then MAJORs)

1. **`contracts/machine.toml`** — pin the machine: R, a, B_T, I_p, κ, δ, plasma volume,
   coil set + limits, **vessel/passive-conductor L-R** (PHY-01/02: resolve the γ-vs-Q>1
   two-machine contradiction by *choosing one machine* and deriving both numbers from it;
   a SPARC/DIII-D-class compact device is the suggested scale — decide and justify).
2. **The reward/cost function** (CTL-B15) — in the spec §3 + a `contracts/reward.toml`:
   shape/tracking terms, disruption penalty, actuator costs, episode termination; plus
   the **held-out seed law** (train/eval seed domains) and the **dispersion envelope**.
3. **Disruption/quench physics + its event token** (PHY-06 + MEM joint fix: "the token
   and the physics ship together or neither ships") — VDE→wall-contact→current-quench
   model (scaling-law grade is fine, honesty-labeled), the `[plant] disruption ...`
   vocabulary, and the M1 gate re-derived on it.
4. **Writ grammar v1** (CTL-B17/D-013): id, supersedes, ttl, cancel, `formed_tick` +
   staleness windows per class; the H2 objection-window semantics; readback text spec.
5. **Scenario schema** — `contracts/scenarios/_TEMPLATE.toml` (REPO-B2 + PHY-05): what is
   deterministic, what is random (the stochastic model behind Wilson CIs), disturbance
   timing/magnitude distributions, per-scenario pass criteria (duration + disturbance for
   "sustains Q>1" — PHY-16).
6. **Falsifier completions** (unanimous): F-INSTINCT's held-out eval set — which
   milestone creates it, where it lives, the quarantine rule (auricle's `--review` valve
   is the parent pattern); F-PRESENCE-F grader protocol per TinyVillage L12 (disjoint
   rater, rubric, consistency floor); F-NULL-C "matched compute" defined mechanically;
   new F-M0/F-M1 gates if you promote them (PHY recommendation).
7. **The statecheck fence** (CTL/MEM) — the mechanical guard that synthetic diagnostics,
   not true state, reach the policy and the membrane; one paragraph + a ctest name.
8. **GPU arbitration + crisis scheduling** (VOICE-B2 + MEM): the priority law
   (plant > LLM), the tick-jitter measurement + its M3 gate row, probe batching under
   crisis rates, the molt deferral window (MEM: no 8.9 s blackout mid-crisis — defer to
   post-crisis or pre-emptive molt at crisis onset; decide), §7 rows for telemetry
   bandwidth (VOICE-V07) and last-word→first-phoneme (VOICE chain C).
9. **Nulls for the two new organs** (MEM): the innovation-gated tokenizer vs a naive
   threshold logger; the ghost vs a static limit-table warning. Cheap, pre-registered.
10. **§11 gains an "enforced by" column** (MEM's structural recommendation) and the three
    wrong rows fixed; glossary completions (REPO's list).
11. **KICKOFF_M0 rewritten** (REPO's three cold-session blockers): TOML parser choice +
    `third_party/` vendoring plan (no package managers), the exact build-command block
    (generator string, toolchain, ctest invocation — copy auricle's style), goldens
    defined + created in M0 scope, branch discipline named.
12. **Re-run REPO's cold-session simulation** against the rewritten kickoff; it must
    walk clean. Then: SESSION_LOG S2 entry, ROADMAP V0.2 box checked, D-entries for every
    design choice, commit (same-commit discipline), snapshot.

## Rules

Append-only DECISIONS (supersede, never edit). v0.1 is frozen — all changes land in
v0.2's file. The five QC reports are read-only records; disposition them in a
`docs/qc/DISPOSITIONS.md` ledger (finding ID → fixed-in-v0.2 §X / redesigned / accepted-
as-risk with reason). Forward slashes in all shell commands. No product code, no
third_party vendoring yet — that's M0's opening move once you've specced it.
