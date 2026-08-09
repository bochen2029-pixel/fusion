# KICKOFF — SPEC v0.2 · the QC-swarm revision (bootstrap for the next session)

**M0 is suspended until this work exists.** You are producing
`FUSION_ARCHITECTURE_v0.2.md` (v0.1 stays beside it, untouched from here on) plus the
design artifacts the QC swarm and the fresh-eyes pass proved missing. No product code in
this phase.

> **Scope honesty (FRESH_EYES Q4-1, ratified):** this worklist is **2–4 sessions, not
> one**. Work in this order — `contracts/machine.toml` FIRST (≈⅓ of all findings derive
> from it), then objective/dispersions/scenario schema, then the rest. If a session ends
> mid-list: commit the completed deliverables + a SESSION_LOG entry + the DISPOSITIONS
> ledger showing how far the walk got (partial-commit discipline), and hand off.

## Read first, in order

`CLAUDE.md` → `docs/qc/QC_CONSOLIDATED.md` (the eight blocker clusters + what was already
patched in-place — do NOT re-fix those) → the five lane reports
`docs/qc/QC_{PHYSICS,CONTROL,MEMBRANE,VOICE_RT,REPO_PROCESS}.md` **in full — they are the
line-item worklist** → **`docs/qc/FRESH_EYES.md` in full, including its Addendum §5 — the
second instrument; its §4 top-5 actions are mandate, not suggestion (D-018/D-019/D-020)**
→ `FUSION_ARCHITECTURE_v0.1.md` (as QC-patched) → `DECISIONS.md` D-001…D-020 →
`contracts/` → `SESSION_LOG.md` tail. Optionally replay the room:
`python C:/Intercom/intercom.py replay --room fusion-qc --limit 200` (forward slashes —
Git Bash eats backslashes). Then confirm: (a) the eight clusters, (b) what D-011…D-020
already settled, (c) your deliverable list below, (d) the multi-session discipline
(SESSION_LOG append + same-commit docs + rewrite KICKOFF_M0 + write KICKOFF_M0's successor
note before ending). Then work.

## Deliverables (each traceable to finding IDs; disposition every BLOCKER, then MAJORs)

1. **`contracts/machine.toml`** — pin the machine: R, a, B_T, I_p, κ, δ, plasma volume,
   coil set + limits, **vessel/passive-conductor L-R** (PHY-01/02). Resolve the γ-vs-Q
   two-machine contradiction by pinning a **synthetic machine — FUSOR-1** (compact,
   deliberately thick-walled) and **deriving both numbers from that one config**: γ from
   the vessel/passive L-R eigenproblem, Q>1 capability from IPB98(y,2)+H98 on the same
   geometry; publish both derivations as comments in the file. Do **not** anchor on a
   named real device (FRESH_EYES D1): PHY-02's recorded self-refutation stands — no
   machine is both Q>1-capable and fast-VDE, and a DIII-D-class device cannot reach Q>1
   at all (PHY-01).
2. **The reward/cost function** (CTL-B15) — in the spec §3 + a `contracts/objective.toml`:
   shape/tracking terms, disruption penalty, actuator costs, episode termination; **one
   objective, every consumer** (the trainer's reward, the teacher's cost, the null's cost
   all read this file — CTL-07); plus the **held-out seed law** (train/eval seed domains)
   and the **dispersion envelope**.
3. **Disruption/quench physics + its event token** (PHY-06 + MEM joint fix: "the token
   and the physics ship together or neither ships") — VDE→wall-contact→current-quench
   model (scaling-law grade is fine, honesty-labeled), the `[plant] disruption ...`
   vocabulary, and the M1 gate re-derived on it.
4. **Writ grammar v1** (CTL-B17/D-013): id, supersedes, ttl, cancel, `formed_tick` +
   staleness windows per class; the H2 objection-window semantics; readback text spec;
   **and the session input-tape** (FRESH-6): accepted-writ receipts (id + tick) and the
   operator-input timeline recorded as replayable inputs — the booster's recorded-pilot
   pattern — so a live-mind session replays bit-identical and F-GHOST's fork-vs-reality
   memcmp stays defined when writs land inside the lookahead window.
5. **Scenario schema** — `contracts/scenarios/_TEMPLATE.toml` (REPO-B2 + PHY-05): what is
   deterministic, what is random (the stochastic model behind Wilson CIs), disturbance
   timing/magnitude distributions, per-scenario pass criteria (duration + disturbance for
   "sustains Q>1" — PHY-16); **plus the pre-registered floor-legal-but-lethal class**
   (FRESH-3): commands that pass every floor and still kill the plasma inside the ghost
   horizon — the only class on which ghost-vs-governor value separates; F-PRESENCE-F's
   planted-error battery draws ≥N cases from it.
6. **Falsifier completions** (unanimous + D-018): F-INSTINCT's held-out eval-set home +
   quarantine rule (auricle's `--review` valve is the parent pattern); **F-PRESENCE-F
   gains its grader protocol (TinyVillage L12) AND its third arm** — resident triple /
   **event-triggered turn-based twin** / polling twin, each arm's information set
   pre-registered (identical token stream + floors for all three; partials + cross-event
   KV memory are the resident arm's only structural extras — that difference IS the claim
   under test); **F-VERACITY, new falsifier** (FRESH-2): planted factual questions + a
   citation-fidelity audit blind-graded **by code against the tape** (mechanical grader —
   L5-clean), pass fraction pre-registered, **gating M4's solicited ship**; F-NULL-C
   "matched compute" defined mechanically (estate law: matched wall-clock AND VRAM, the
   null ships in-binary — CTL-08/29b); **the promoted F-BURN-0 / F-SHAPE-1 gates**
   (PHY + REPO P-13: DoD clause 1 is undefined for the first two milestones without
   them); and the **post-M-M2 Sentinel-null statement** (FRESH_EYES D2): once the
   Sentinel's emit is fork-gated, organ-vs-klaxon separates only on communication value
   — say so, and grade that under the F-PRESENCE-F/F-VERACITY instruments.
7. **The statecheck fence** (CTL/MEM) — the mechanical guard that synthetic diagnostics,
   not true state, reach the policy and the membrane; one paragraph + a ctest name.
8. **GPU arbitration + crisis scheduling** (VOICE-B2 + MEM): the priority law
   (plant > LLM), the tick-jitter measurement + its M3 gate row, probe batching under
   crisis rates, the molt deferral window; §7 rows for telemetry bandwidth (VOICE-V07)
   and last-word→first-phoneme (VOICE chain C); **plus a whole-app CPU/core budget
   table** (FRESH-7: spine spin, policy, EKF, ASR, TTS, llama host threads — affinity
   and what degrades first).
9. **Nulls for the two new organs** (MEM): the innovation-gated tokenizer vs a naive
   threshold logger; the ghost vs a static limit-table warning — **scored on the
   lethal-legal class of deliverable 5** (FRESH_EYES D2, else the comparison is
   confounded with the governor). Cheap, pre-registered.
10. **§11 gains an "enforced by" column** (MEM's structural recommendation) and the three
    wrong rows fixed; glossary completions — REPO's SYNCYTIUM list **plus the tokamak
    half** (FRESH_EYES §1.2: VDE, Greenwald, Troyon/β_N, q95 + its direction, κ/δ,
    MPPI/CEM/EKF) **and the estate stragglers** (demesne pattern, free-tail razor,
    L-SILENCE, rung, truth lane, dial-at-zero).
11. **KICKOFF_M0 rewritten** (REPO's three cold-session blockers): TOML parser choice +
    `third_party/` vendoring plan (no package managers), the exact build-command block
    (generator string, toolchain, ctest invocation — copy auricle's style), goldens
    defined + created in M0 scope, branch discipline named.
12. **Re-run REPO's cold-session simulation** against the rewritten kickoff; it must
    walk clean.
13. **The eight orphaned dispositions** (FRESH_EYES Q4-3 — swarm MAJOR/BLOCKER fixes
    with no deliverable line; disposition each explicitly, **the M2 ladder FIRST**):
    the **M2 ladder D-entry** (CTL-04: θ/gain-schedule → residual → end-to-end, rung (a)
    gate-sufficient); `contracts/diagnostics.toml` (CTL-25); plant-lane boundary
    definition (M-B6: structural — one event = one boundary); the three room-cascade
    guards (M-B7); vocabulary freeze at M3 (M-M13); harm-dial → autonomy-dial rename in
    risk order (CTL-22); molt-on-calm + watermark-in-crisis behaviour (M-M6); `tts.h`
    contract (V-15). **Plus the EKF D-entry** (CTL-11's four decisions + FRESH-4's
    information-set clause — state what the EKF may know; the epistemic handicap, not the
    σ threshold, sets the entire percept stream — + the NIS/χ² acceptance gate at M1).
14. **Scope rulings to implement** (D-020): MPC null → PID+LQ is the null, the MPC a
    later separately-receipted comparison (CTL-28); MCTS + sysid-by-backprop out of §3 →
    parked with named nulls or deleted (CTL-30/31); depths tier cut from the v1 stack;
    every range collapses to its cheap end for v1. **Tier-2 MHD: the operator rules**
    (recommended v1 cut; sanctioned compromise: renderer raymarches tier-1-derived
    emissivity + artistic noise) — leave the bracketed question in DISPOSITIONS if
    unruled, and record the ruling as a D-entry either way.
15. **Ripple sweep + citation hygiene** (FRESH_EYES Q4-6): the in-place patches missed
    v0.1 text that v0.2 must sweep — §6's "controller OFF → VDE in ~50 ms" (PHY-17:
    honest figure 100–300 ms at corrected γ, and better television), §1's flat-100 µs
    spine row (M-N5), §7's renderer-row provenance (V-08 + FRESH_EYES D3: 4.6 GB is the
    **plain desktop**, renderer +0.5–1.5 GiB estimated-unmeasured). Every number v0.2
    inherits carries its receipt path; every number it derives shows its arithmetic
    (D-018: the emit rate cites "2,823 / 3h03m51s = 921/hr, run C"). Small fixes ride
    along: `events.toml [health]` debounce (FRESH-5: 1.0σ/5-min ≈ 300–400 tok/hr of pure
    noise — ≥2.5σ + N-of-M dwell), warpbus tape persistence (FRESH-8: steal auricle
    `src/fabric/durable.h`), the ghost's render path (FE-19: one sentence — what the
    overlay draws from a tier-0/1 fork), the `[sys] mind died` watchdog receipt (FE-20),
    LICENSE (FE-25).

Then: SESSION_LOG entry, ROADMAP V0.2 box checked (only when the whole list is walked),
D-entries for every design choice, commit (same-commit discipline), snapshot.

## Rules

Append-only DECISIONS (supersede, never edit). v0.1 is frozen — all changes land in
v0.2's file. The five QC reports and FRESH_EYES are read-only records; disposition them
in a `docs/qc/DISPOSITIONS.md` ledger (finding ID → fixed-in-v0.2 §X / redesigned /
accepted-as-risk with reason) — FRESH-1…8, D1…D4 and the FE/P items disposition alongside
the lanes' IDs. **Tie-breaks (you are one session, no peer lanes):** physics → QC-PHYSICS
rules; estate law → the parent's receipt rules; scope/aesthetics → the operator rules
(leave a bracketed question in DISPOSITIONS rather than guessing — tier-2 is already one).
Forward slashes in all shell commands. No product code, no third_party vendoring yet —
that's M0's opening move once you've specced it.
