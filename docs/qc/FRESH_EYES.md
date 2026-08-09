# FRESH_EYES — zero-context outsider deep scan of C:/fusion
**Instrument:** a brand-new session, no prior project context, run per `KICKOFF_FRESH_EYES.md` ·
**Date:** 2026-08-09 · **Repo state:** `0f23676` (post-QC-swarm, pre-v0.2) + untracked kickoff.
**Method integrity:** Pass-1 read in the prescribed order; all Pass-1 findings were frozen to a
scratchpad file **before** `docs/qc/` was opened (so the Pass-2 diff below cannot retro-fit).
Estate citations spot-checked against `C:/auricle` receipts; one external claim web-checked.

---

## 0 · Verification annex (what I independently checked, per the kickoff's license)

**Against `C:/auricle` (8 checks — all consistent with QC-MEMBRANE's §2 table):**

| claim | verdict | receipt |
|---|---|---|
| **921 emits/hr, "run C"** (D-015, spec §5.2) | **VERIFIED as a derivation** — the numeral appears *nowhere* in auricle; run lettering exists only in the M4.5b receipt's run table. 2,823 emits ÷ 3h03m51s = **921.2/hr**, exact. | `runs/tier1.5-m4.5b-molt-v1-2026-08-09.md:19` |
| 593/hr = run A, ~55% coverage | VERIFIED (run A died at 18,708 words ≈ 55%) | same file + `SYNCYTIUM_SOAK_M4.5.md` |
| probe ~65 ms | VERIFIED as **65–70 ms** (spec cites low end — M-N6 right) | `SESSION_HANDOFF.md:285` |
| lag mean 327/349 ms, max 2.1/5.5 s | VERIFIED | `SESSION_HANDOFF.md:285,287` |
| molt ~9 s outage, ~11× compaction | VERIFIED (8,856/8,982 ms; 10.5×/12.0×) | `SYNCYTIUM_SOAK_M4.5.md:50`, receipt |
| desktop ≈4.6 GB | VERIFIED — **and it is the *plain desktop*, no renderer** (V-08's provenance point independently confirmed; see §2.3-D3) | `SESSION_HANDOFF.md:86` |
| segmenter P(end) 0.97 vs 0.02 | VERIFIED ("berlin"/"in") | `SESSION_HANDOFF.md:268` |
| F-PRESENCE status | auricle's own words: *"the real, unproven bet"* — the parent agrees the twin comparison is the open experiment | `SESSION_HANDOFF.md:283` |

**Web (1 check):** sherpa-onnx has shipped multilingual Nemotron-3.5 streaming ASR support;
pre-exported ONNX exists (`pantinor/…-onnx`, `onnx-community/…-int4`); the 80/160/320/560/1120 ms
chunk menu matches §5.1 exactly. **V-04's "pin bump, not export project" holds.** Sources:
github.com/k2-fsa/sherpa-onnx/releases · issue #3664 (closed by the support PRs) ·
huggingface.co/nvidia/nemotron-3.5-asr-streaming-0.6b (discussion #1).

**Recommendation from the 921 check:** D-015's figure is honest but is an *inference from* a
receipt, not a quotation *of* one. In a repo whose law is "receipts non-mintable," derived
figures should carry their arithmetic: cite as **"2,823 emits / 3h03m51s = 921/hr (run C)"**.

---

## 1 · Pass 1 — the cold read (frozen before `docs/qc/` was opened)

### 1.1 Premise-level doubts

- **P-1 · The plant-side anti-turn case is weaker than the spec owns.** The architecture
  guarantees the cortex is never *necessary* (spine+cerebellum must hold the plasma alone; floors
  bound writs) — fine, that's the safety design. But then the *anti-turn necessity* argument rests
  entirely on the operator-voice side (ghost-on-forming-partials, barge-in). On the plant side,
  the innovation-gated tokenizer **is a threshold-waker** — structurally the "dumb waker" the
  regress dismisses. An event-triggered turn-based mind fed the same token stream would wake at
  the same moments. Residency's plant-side value is *memory and integration across events*, not
  wake timing — and no falsifier isolates that. (M-B6's fix — plant-lane boundaries are
  structural, one event = one boundary — quietly concedes this; see §2.3-D4.)
- **P-2 · F-PRESENCE-F's null is a strawman as written.** §8 hands the twin "the same telemetry
  summaries **at its best cadence**" — a *polling* twin. The honest null is an **event-triggered**
  turn-based twin (invoked once per tokenized event/boundary). If the resident triple only beats
  polling, the anti-turn claim is not proven in this domain. (→ FRESH-1, §2.1.)
- **P-3 · Overtrust is unexamined.** A first-person, articulate reactor voice will be believed
  beyond its competence (classic automation-trust result). Nothing measures whether what it says
  is *true*, and nothing calibrates its confidence. Nobody in the repo asks "what happens when the
  mind is confidently wrong out loud?" (→ FRESH-2.)
- **P-4 · Who is this for?** Paper, product prototype, art piece, estate-internal experiment? All
  falsifiers are internal; no external-validity question is asked anywhere. If the estate goal is
  "anti-turn generalizes beyond auricle," then F-PRESENCE-F is *the* experiment and M0–M2 are
  stage construction — that hierarchy is never stated, and it should drive sequencing (§3-Q1/Q3).
- **P-5 · Governor-vs-ghost redundancy.** Floors already refuse floor-crossing writs
  deterministically. The ghost's marginal value = commands that (a) pass every floor, (b) still
  kill the plasma, (c) within the lookahead horizon. No scenario class demonstrates that set is
  non-empty. (→ FRESH-3.)
- **P-6 · "Surprise" is degenerate in a simulator.** The one-dynamics-source law actively pulls
  the cerebellum's forward model toward model=truth, in which case innovation ≈ injected
  disturbance + sensor noise and the upward seam is really a *disturbance detector* whose
  statistics are set by the scenario stochastic model. What the EKF is *allowed to know* is the
  load-bearing unstated choice. (→ FRESH-4.)
- **P-7 · The cheapest de-risk path is never considered:** bolt the membrane onto the *already
  measured* booster sim. The good answer (a landing is 60 s of life; a tokamak campaign is hours
  of continuous life worth narrating) is nowhere written down.
- **P-8 · The depths tier is vestigial** — defined in §1, used by zero milestones, budgets, or
  falsifiers. Define a consumer or cut it.

### 1.2 Comprehension failures (the cold-boot test — this subsection grades the repo)

Terms/claims a session holding **only this repo** cannot resolve (my outside knowledge noted
where it rescued me; each is a finding regardless):

1. **"the demesne pattern"** (§5.2) — never defined in-repo.
2. **"A3-safe", "arm-B worked-example seed"** (§4) — auricle jargon, undefined here.
3. **"kv_unified … assert n_ctx_seq == n_ctx"** — llama.cpp internals assumed.
4. **"the free-tail razor"** (§4.1) — undefined; *decode-on-delta itself* is defined only in
   KICKOFF_FRESH_EYES, not in any document a build session reads.
5. **"L-SILENCE"** — a named law, never expanded (P-11 lists it too; still unfixed).
6. **"the label factory"** (§11) — undefined.
7. **TinyVillage "L2"/"L12"** — law numbers cited with content only gestured at.
8. **"Verifier Law"** — carries load ("licenses all of it") on a table-fragment definition.
9. **"composition-public law"** (§5.2) — undefined.
10. **"scribe rung ≤ 600 tok"** — "rung" undefined here.
11. **"θ-predictor over an analytic base law"** (§3) — M2's key architecture decision leans on a
    record that lives in another repo, untranslated.
12. **"lane-H partials"** — names a lane absent from the lane list (= M-N1/V-13; still in spec).
13. **VDE** — never expanded anywhere (vertical displacement event); used in §2.2, ROADMAP M1,
    floors.toml. The project's declared antagonist is an unexpanded acronym.
14. **MPPI, CEM, EKF** never expanded; **q95 / κ / δ / β_N / Greenwald / Troyon** assumed known.
15. **"dial at zero"** semantics inferable only from context; the emit runbook lives in auricle.
16. **The ~8.9 s molt outage cost** appears only in KICKOFF_SPEC_V02 — the spec's own molt
    paragraph never states the number a designer must schedule around.
17. **"M4.5/M4.5b", "run A/run C"** — parent-repo versioning assumed.
18. **"seq_cp"** (§2.2) — llama.cpp op as metaphor.
19. **"Parakeet-TDT truth lane"** — "truth lane" undefined.
20. **Intercom** — referenced for replay, never introduced.
21. **WDDM** (§7) — unexpanded, load-bearing for the renderer row.
22. **SYNCYTIUM vs auricle naming** — mapping stated only in the fresh-eyes kickoff.

**Meta-finding:** the cold-boot bet fails precisely at the **two parent vocabularies** —
membrane-ese and tokamak-ese — the two things being fused. The glossary covers this repo's
neologisms and misses the inherited jargon a build session actually needs (P-11 caught the
SYNCYTIUM half; the tokamak half — VDE, Greenwald, Troyon, q95-direction — was caught by no one;
the physics lane *used* the terms fluently, which is exactly why).

### 1.3 Independent flaws (as frozen; Pass-2 status added right)

| id | sev | location | flaw (compressed) | Pass-2 status |
|---|---|---|---|---|
| FE-1 | MAJOR | §8 F-PRESENCE-F | Twin null is polling-only; needs an event-triggered arm | **MISSED BY ALL FIVE → FRESH-1** |
| FE-2 | BLOCKER | §8, §9-M4, D-007 | No falsifier measures whether what the reactor *says* is true; solicited answers ship gated on plumbing only | **MISSED BY ALL FIVE → FRESH-2** |
| FE-3 | BLOCKER | §3, §4.1, events.toml | EKF's epistemic handicap unstated → innovation statistics undefined | partially caught (CTL-11's four undecideds; NIS gate); degeneracy/information-set framing missed → **FRESH-4** |
| FE-4 | MAJOR | §2.2-r6, §5.4, §7 | Ghost×tier-1 GS arithmetic breaks the ≤25 ms budget | **caught, better** — PHY-04 (2–7 ms/solve; F-GHOST rescope) |
| FE-5 | MAJOR | §2.3, §5.4, §8 | "Floor-legal but lethal" scenario class never pre-registered | organ-nulls caught (M-M9/V0.2-#9); the *scenario-class* requirement missed → **FRESH-3** |
| FE-6 | MAJOR | §2.1/§9-M0 | M0 gate vacuous (no named adversary) | **caught, better** — PHY-07/16, CTL-34 |
| FE-7 | MINOR | events.toml [health] | drift_sigma=1.0 ⇒ P(\|z\|≥1)≈0.32/organ/window ⇒ ~300–400 tok/hr of health noise vs the 150 cap | **MISSED BY ALL FIVE → FRESH-5** |
| FE-8 | MAJOR | §2.1/D-003, §7 | Async-GS apply semantics unspecified; WDDM preemption; memcmp blind to wall-time staleness | **caught, better** — PHY-11 + V-02 |
| FE-9 | MAJOR | §2.1 | Tier-0/1 confinement double-sourced | **caught** — PHY-10 |
| FE-10 | MINOR | writs.toml vs D-013 | setpoint harm=H1 contradicts voice-setpoints-at-H2 | caught (V-03 names both files to supersede) |
| FE-11 | NIT | events.toml | q95 warn/alarm direction inverted vs neighbours | **caught** — P-05 |
| FE-12 | MINOR | floors.toml | beta_n_max_frac=1.0 contradicts margin philosophy | **caught, better** — PHY-21 (limit configured nowhere) |
| FE-13 | MINOR | §4 | "lane-H" doesn't exist | **caught** — M-N1, V-13 |
| FE-14 | NIT | writs.toml [mode] | EMERGENCY_SHUTDOWN request inexpressible | **caught** — CTL-23(c) |
| FE-15 | NIT | spec footer | QC D-range wrong at both ends (D-003…D-008 vs D-009; D-004 not a catch) | **caught** — P-23 |
| FE-16 | NIT | KICKOFF_M0 | stale refs (D-010, "S1" already taken) | caught-ish (P-25; rewrite mandated) |
| FE-17 | MINOR | §1 | depths tier vestigial | **MISSED BY ALL FIVE** (minor) |
| FE-18 | MINOR | §3 | MCTS + sysid unhomed scope | **caught, better** — CTL-31 + CTL-30 (circularity) |
| FE-19 | MINOR | §5.4/§6 | Ghost *render* path undefined — tier-2 is not forked, so the "translucent ghost of the plasma dying" has no defined visual source | **MISSED BY ALL FIVE** (minor) |
| FE-20 | MINOR | §4/sys lane | No death story for the cortex host process (drop-events cover lateness, not a dead llama.cpp; no watchdog, no `[sys] mind died` receipt) | **MISSED BY ALL FIVE** (minor) |
| FE-21 | NIT | repo-wide | No CI/per-commit test automation across dozens of sessions | adjacent to P-03; listing as marginal |
| FE-22 | MAJOR | KICKOFF_SPEC_V02 #1 | "SPARC/DIII-D-class" suggestion embeds the γ-vs-Q trap it is asked to resolve | **disagreement with the S1 packaging → §2.3-D1** |
| FE-23 | MAJOR | §2.2-r4, §8, DoD-2 | Session-replay semantics with a live mind unstated: bit-identical replay of any session containing LLM writs/operator input requires the accepted-writ + operator-input **tape as recorded input** (the booster's recorded-pilot pattern); F-GHOST's fork-vs-reality memcmp inherits the same hole | **MISSED BY ALL FIVE → FRESH-6** |
| FE-24 | MINOR | §7 | No host-CPU/core budget despite policy+ASR+TTS+EKF+spine-spin+llama host threads all landing on CPU | partial (CTL-27 in-tick; V-14 voice threads); whole-app core map still unowned → **FRESH-7** |
| FE-25 | NIT | repo root | No LICENSE while an LLC is named | missed by all five (trivial) |
| FE-26 | NIT | §5 vs CLAUDE.md #7 | Consent floor never reflected in the voice sections | missed by all five (trivial) |
| FE-27 | MINOR | §4/glossary | Warpbus tape *persistence* unspecified (where the hash-chained tape lands on disk, rotation, cross-session) — auricle's `durable.h`/move-1 is the obvious inherit and is not inherited | **MISSED BY ALL FIVE → FRESH-8** |

---

## 2 · Pass 2 — the diff

### 2.1 Missed by all five lanes (the payload)

**FRESH-1 (MAJOR) · F-PRESENCE-F needs a third arm: the event-triggered turn-based twin.**
§8 specifies the null as a twin "fed the same telemetry summaries at its best cadence" — i.e.
polling. But the repo's own upward seam *manufactures* discrete, well-timed events; a turn-based
mind invoked once per event (or per M-B6 structural boundary) gets evidence-driven timing for
free, without residency. The three-arm design (resident triple / event-triggered twin / polling
twin) is what separates the *residency* claim (KV-hot memory, cross-event integration,
mid-partial perception) from mere *event-driven invocation*. As written, F-PRESENCE-F can pass
while proving only that events beat polling — which nobody doubts. The specialists all audited
*whether the gate is runnable* (grader, rubric, L12 — M-M12) and never *whether it tests the
thesis*. Fix: add the third arm to §8 + the V0.2 falsifier-completions deliverable; define what
each arm is allowed to see (identical token stream, identical floors) and what only the resident
arm structurally has (partials, cross-event KV memory).

**FRESH-2 (BLOCKER by house doctrine) · Nothing measures whether what the reactor says is true.**
"Every claim citing a bus rev" (§4 SPEAKER) is a mandate with no measurement anywhere: a Speaker
that fabricates rev citations or mis-reads the tape passes all six falsifiers. Solicited answers
— the thing M4 *ships out loud* — are gated by F-VOICE alone, which measures latency and ledger
completeness, never content. This is an organ shipping without its content gate, in the one
domain where the gate is nearly free: the tape and the deterministic plant state ARE ground
truth. Fix: an **F-VERACITY** falsifier at M4 — a pre-registered battery of planted factual
questions (current Q, last limit crossing, why did the governor refuse writ N) blind-graded
against the tape, plus a **citation-fidelity audit** (sampled utterances → does the cited rev
exist and support the claim), with a pre-registered pass fraction. Note the grader-protocol
machinery M-M12 imports for F-PRESENCE-F serves this too, and TinyVillage L5's "no model grades
anything" binds here as well — the tape-lookup is mechanical, so this grader can be code, not an
LLM. (This also answers P-3's overtrust doubt with a receipt instead of a vibe.)

**FRESH-3 (MAJOR) · The "floor-legal but lethal" scenario class must be pre-registered.**
The ghost's value over the governor is exactly the set of commands that pass every floor and
still kill the plasma within the lookahead horizon. No scenario requires that set to be
demonstrably non-empty; F-PRESENCE-F's "planted dangerous commands" could all be floor-refusable,
in which case the klaxon-grade governor catches them and the Sentinel's win is theater. The V0.2
ghost-null (deliverable 9: "ghost vs a static limit-table warning") makes this *worse* if the
scenario set is confounded — the comparison only discriminates on floor-legal-but-lethal cases.
Fix: add a named scenario class to §2.3/`_TEMPLATE.toml` (e.g. a floor-legal I_p ramp that drives
the vertical eigenvalue unstable given the *current* l_i/κ trajectory — dynamically lethal,
statically legal) and require F-PRESENCE-F's planted-error battery to draw ≥N from it.

**FRESH-4 (MAJOR, completing CTL-11) · Pre-register the EKF's information set — the epistemic
handicap.** CTL-11 asks *which model* the EKF runs from the build side; the premise-side question
is sharper: the one-dynamics-source law pulls toward model=truth, and a truth-model EKF in a
noise-free-dynamics sim yields innovation ≈ injected disturbances + sensor noise only — the
plant then "surprises its cerebellum" exactly when the scenario schema says so, and the entire
upward seam's statistics are an artifact of `dispersions.toml`. That may be acceptable — but it
must be *chosen*: state what the EKF may know (reduced model class, no disturbance-schedule
access, which parameters it may not see), because that choice — not the 3σ threshold — sets the
token stream. Belongs in the same D-entry as CTL-11's four decisions.

**FRESH-5 (MINOR, quantitative) · `events.toml [health] drift_sigma = 1.0` busts the budget by
itself.** P(|z| ≥ 1) ≈ 0.317 under the null, per organ per 5-min window ⇒ ~3.8 events/organ/hr ⇒
at ~10 organs, ~38 events ≈ 300–400 tok/hr against the 150/hr cap — health noise alone triples
the budget with nothing wrong. The swarm re-derived the innovation and tick lanes (CTL-03, P-07)
and never ran this row's number. Fix: threshold ≥ 2.5–3σ with dwell (N-of-M windows), or define
the statistic as a slope estimator with its own calibrated null.

**FRESH-6 (MAJOR) · Session-replay semantics with a live mind are unstated.** The memcmp oracle
replays seed + scenario. From M3 on, a real session's trajectory also depends on accepted writs
(from a nondeterministic cortex) and operator input. Unless the governor's accepted-writ schedule
(ids + ticks — the fields D-013 is adding anyway) and the operator-input timeline are recorded as
part of the run artifact and replayed as *inputs* — the booster's recorded-pilot pattern —
"replay bit-identical" is undefined for exactly the sessions the M5 ledger publishes, and
F-GHOST's fork-vs-reality memcmp is under-specified whenever any writ lands inside the lookahead
window. Cheap fix, natural home: the writ-grammar v1 deliverable (V0.2 #4) — an accepted-writ
tape is just the `sys`-lane acceptance receipts (CTL-24b/M-M14) in replayable form.

**FRESH-7 (MINOR) · No whole-app CPU budget.** D-012 + V-04 moved the policy and ASR to CPU; TTS
is CPU; the spine needs a spin core; the EKF, duplex governor, telemetry, and llama.cpp host
threads all want cores. CTL-27 budgets the 100 µs tick and V-14 asks for the voice thread map;
nobody owns the host-wide core/affinity budget on one consumer CPU. Add a §7 CPU table (cores ×
consumer, affinity policy, and what degrades first).

**FRESH-8 (MINOR) · The warpbus tape's persistence is unspecified** — where the hash-chained
committed-rev tape lands on disk, its rotation, and its cross-session succession. Every falsifier
says "on the tape"; nothing says the tape survives the process. auricle already built exactly
this (`src/fabric/durable.h` + `m0_tape.cpp`: durable append, hash-chain-intact re-fold on
restart, partials never persist) and fusion's §4 inherits `fabric.h` but not the durable tier.
One sentence + one steal.

**Also missed (small):** FE-17 (depths tier vestigial), FE-19 (ghost overlay's render source —
the M5 money shot has no defined pipeline from a tier-0/1 fork to a volumetric ghost), FE-20
(`[sys] mind died` watchdog receipt), FE-25/26 (LICENSE; consent-floor reflection in §5), and a
NIT the toggle demo deserves: §9-M5's "toggle OFF → beautiful catastrophe" interacts with
`[spine_autonomy]` (the spine will fire `vde_detected` shutdown "mind or no mind" — likely
unable to save an uncontrolled VDE, but the demo script should say what the spine does while the
operator watches the catastrophe it is programmed to prevent).

### 2.2 Missed by me (calibration — their findings I did not have)

Excluding the ~17 findings pre-disclosed to me by the patched repo (D-011…D-017 and the QC
banner made LODESTAR, γ, policy-on-CPU, headset, commit=governor-accept, the budget rebuild,
921, machine.toml, statecheck, GPU-arbitration, F-INSTINCT's home, DoD null-win, and the ui/
exception visible before my cold read), the swarm found **at least 25 material things I missed**.
The ones I rate highest, by lane:

- **PHY:** particle balance absent while gas/pellet actuators exist (PHY-07 — actuators with no
  state to act on; I flagged the vacuous gate, they named the missing physics); line radiation
  (PHY-08); the IPB98 implicit-P_loss trap + κ_a convention (PHY-09); **no VDE scenario in the
  curriculum** (PHY-12 — the antagonist missing from its own syllabus); breakdown unmodeled
  (PHY-13); no q-profile/p′FF′ source for GS (PHY-14); Ohmic heating absent (PHY-15); the §6
  "~50 ms" demo line inconsistent with every γ (PHY-17); alpha slowing-down lag (PHY-19);
  integration scheme unnamed / 1 ms can't resolve a quench (PHY-24).
- **CTL:** **spine-shutdown as an unaccounted assist term a trained policy can learn to farm**
  (CTL-10 — the single sharpest finding in the five reports, and I had nothing on
  `[spine_autonomy]`); the **harm dial is not monotone in risk** — H0<H3<H2<H1, so "ships at
  H0/H1" selects the loosest live mode while sounding conservative (CTL-22); MPPI online-vs-
  offline contradiction (CTL-06); Wilson-CI-cannot-apply-to-RMS + no CRN + no split-outcome rule
  (CTL-08); the TCV existence proof uses MPO and *no distillation* (CTL-17); DAgger is the
  parent's load-bearing result and is unnamed (CTL-19); feed the innovation vector to the policy
  (CTL-21); sensor model is three words (CTL-25); null tuning protocol + null-ships-in-binary
  (CTL-29/29b).
- **MEM:** **a "boundary" is undefined on non-speech lanes** (M-B6 — the segmenter is linguistic;
  I never noticed the judgment cadence itself was undefined on the plant lane); the room-cascade
  guards, 1 of 3 inherited (M-B7); the pre-commit window arithmetic 1.9 s vs 1.95 s (M-B5);
  Sentinel's vague mandate → gate its emit on the fork result (M-M2); molt-fires-during-crisis
  with the ~10.2-min derivation (M-M6); 115% GPU duty at measured disposition (M-M7);
  resume-or-abandon overreaches the reflex receipt (M-M10); the vocabulary freeze law L3
  (M-M13); the surprisal tap is measured to *exist*, not to *work* (M-N8).
- **VOICE:** WER is published only at 1120 ms chunks — the 160 ms operating point is unpriced on
  a safety path, and an in-range misheard numeral defeats floors (V-05); chain C — last word →
  first phoneme ≈ 2.5 s typical — is the felt number and is in no budget (V-09); F-VOICE's
  t₀/t₁ undefined (V-11); the queued-PCM law (V-10); the §7 renderer row's provenance mislabel
  (V-08 — I verified the 4.6 GB receipt myself and still failed to notice the spec bills it as
  "renderer + desktop").
- **REPO:** **`core.autocrlf=true` + no `.gitattributes` = CRLF rewriting of goldens silently
  killing the memcmp gate** (P-12 — since fixed on disk: `.gitattributes` now exists in the S1
  tree); the golden artifact is undefined and M0 creates none while DoD demands them (P-09); the
  branch-discipline gap (P-10 — I noticed S0/S1 landed on `main` and did not chase the mechanism);
  the KICKOFF-pointer fragility (P-15); `fusor mc` vs `fusor_mc` (P-21); the null-win DoD
  deadlock (P-26, pre-disclosed in the patched DoD, but I would not have derived its
  roadmap-deadlock consequence).

**Calibration statement:** of my 27 frozen flaws, 12 were already in the reports (usually
deeper), 4 partially, 10+1 survived as fresh; of the swarm's ~99 BLOCKER+MAJOR findings I
independently derived perhaps 8–10. The five-lane instrument is roughly an order of magnitude
more productive than one cold generalist on in-slice defects — and it still left a coherent
residue, which is the argument for running both.

### 2.3 Disagreements (with the reports or the applied dispositions)

**D1 · KICKOFF_SPEC_V02 deliverable 1 embeds the trap PHY spent its best self-refutation
killing.** "A SPARC/DIII-D-class compact device is the suggested scale" — PHY-01's own blocker
text says a DIII-D-scale device *cannot pass M0 with perfect code* (no D-T, Q≪1), and PHY-02's
recorded retraction establishes no real machine is both Q>1-capable and fast-VDE. Suggesting the
two names in one breath invites the v0.2 session to anchor on DIII-D parameters and re-create
PHY-01, or on SPARC and inherit a γ it must then not quote. The kickoff should instead
instruct: **pin a synthetic machine (FUSOR-1); derive γ from its vessel/passive L-R eigenproblem
and Q from IPB98(+H98) on the same config; publish both derivations in machine.toml comments.**
(The blame is the S1 packaging, not the auditors — but the packaging is what v0.2 will read
first.)

**D2 · M-M2's fix and M-M9's null quietly collapse into each other.** M-M2 (right, and I endorse
it) gates the Sentinel's emit on "`fusor_fork` returned disruption". But M-M9's Sentinel null is
"a deterministic threshold alarm — ghost says disruption, beep". After M-M2, the organ and its
null share the same trigger; the only difference left is the *delivery* (a generated, cited,
templated-or-not utterance vs a klaxon). That is still a real comparison — but it must then be
graded on communication value (does the operator understand/trust/act faster), which is
F-PRESENCE-F territory and needs FRESH-2's veracity instrument. State the post-M-M2 null
comparison explicitly, or the ghost-null deliverable (V0.2 #9) will measure a near-tautology.

**D3 · The applied §7 patch left V-08's provenance error in place.** The patched renderer row
still reads "renderer (separate process, WDDM) + desktop | measured up to ~4.6 GB on this box
(auricle receipts)" — but the receipt (`SESSION_HANDOFF.md:86`, verified) is the **plain live
desktop with no WebGPU renderer running**. The number is real; its label is wrong; the honest
row is "desktop ≈4.6 measured; renderer +0.5–1.5 estimated, unmeasured". The consolidated
report's "every §5–§7 figure checkable was honest" glosses this — V-08 said it, and the in-place
patch kept the mislabel while adopting the ≈11.4 total that *depends* on V-08's correction.

**D4 · M-B6's disposition is right and its premise cost is unowned.** Making plant-lane
boundaries structural (one event = one boundary) is the correct mechanical fix — and it concedes
that on the plant side the mind is event-clocked, not attention-clocked (my P-1). No lane
flagged that the fix narrows the anti-turn claim to the human lanes + the emit decision. The
spec should say this out loud in §4.1, and F-PRESENCE-F's three-arm design (FRESH-1) is what
keeps the narrowed claim testable rather than assumed.

**Confirmations for the record:** my 8 estate re-checks agree with QC-MEMBRANE's §2 table on
every overlapping row, including the two "verified-with-omission" verdicts; the 921/hr figure
is exactly derivable from the run-C raw counts; and V-04's sherpa/Nemotron claim survives an
independent web check. I found **no fabricated number anywhere**, consistent with the swarm.

---

## 3 · Pass 3 — the outsider questions, answered plainly

### Q1 · Steelman against the project

The strongest honest case: **this repo's only novel claim is F-PRESENCE-F, and the repo spends
~70% of its mass building a stage for it rather than running it.** The control half is proven
territory (TCV proved RL-flies-a-tokamak; the booster proved the sim constitution and
solver-then-distill's limits; PID/MPC hold real machines today). The membrane half is measured
in auricle. What is genuinely new is one comparison — a resident triple vs a turn-based twin in
a domain with a mechanical verifier — plus one genuinely new seam idea (innovation-gated
tokenization). Neither needs a free-boundary Grad-Shafranov solver: a tier-0 plant with a
scripted event stream and floors could host F-PRESENCE-F, F-VERACITY, and the tokenizer null at
a tenth the cost. Worse, the architecture *correctly* engineers the cortex out of the causal
loop (floors refuse, the cerebellum flies, and post-M-M2 the Sentinel is klaxon-gated) — so the
demo's implicit safety story ("the mind saved the plasma") is, by design, never true; what is
true is "the mind *explained* the save." **[Overreach — corrected in Addendum §5: causally
real *via the sovereign*; the deflation stands only against the autonomous reading.]** And the fusion setting maximizes hype risk for a
project whose §0 disclaims exactly the thing the demo video will appear to claim. Finally, the
emit QLoRA — the estate's actual known bottleneck, the thing gating the entire unsolicited bet
— is parked post-M5, behind months of physics. **The right thing built wrong** would be: months
of GS/transport/MHD before the one experiment that could invalidate the premise.

The counter (why build it anyway): the tokamak is the one domain where every interjection
carries a mechanical verifier — auricle can never grade its Sentinel against ground truth, and
this can; continuous kHz life is the substrate the thesis actually claims (a landing is 60 s);
and the estate's falsifier-first method *requires* staging the bet where it can lose cleanly.
Both are real. The resolution is sequencing, not cancellation — see Q3.

### Q2 · Scope survivability (one operator, AI sessions, one 16 GB GPU)

**As written: not survivable at the implied pace; survivable with two cuts and one inversion.**
Honest session arithmetic against the booster precedent (dozens of sessions for a *rigid-body*
sim): V0.2 as specced is 2–4 sessions, not the 1 the kickoff implies (12 deliverables + 99
line-item dispositions + a cold-sim re-run). M0 with the PHY fixes (particle balance, implicit
IPB98, scheme, stochastic model, machine.toml consumption): 2–4. M1 is the killer: free-boundary
GS on CUDA + passive-conductor eigenproblem + flux diffusion (PHY-14) + 1-D transport + synthetic
diagnostics + EKF with NIS acceptance + disruption model + scenario curriculum — this is more
model surface than the entire booster plant; 6–12 sessions. M2 carries the parent's measured
0/16 wall plus a bespoke training-env fleet: 6–12 (rescued from stall only by CTL-04's ladder).
M3 4–8, M4 3–5 (the ASR half is genuinely de-risked), M5 3–6. **Total ≈ 30–50 sessions**, with
the novel claim untested until roughly session 25+. Where it actually dies, in order of hazard:
(1) mid-M1, in GS/vertical-model determinism grind, with zero user-visible payoff since M0;
(2) M2 at the honest wall, if the ladder's rung (a) is not genuinely gate-sufficient;
(3) post-M5 QLoRA-never-happens (H200 rental is an external dependency), leaving the flagship
bet permanently in shadow — the project "succeeds" hollow. The 16 GB card is *not* a primary
death risk (headless training and play mode never overlap; the prescribed ≈11.4 GiB config
fits); the operator's attention across ~40 low-reward physics sessions is.

### Q3 · Cut first / protect at all costs

**Cut first, in order:** (1) **Tier-2 MHD entirely** — it is decorative by its own §2.1, feeds
only the renderer, costs a VRAM row, a GB/s telemetry problem (V-07), a GPU-arbitration
contender, and a chunk of M1; the demo can raymarch tier-1-derived emissivity with artistic
noise and nobody in the audience can tell. Park it with UE5. (2) The MPC null (keep PID+LQ,
per CTL-28 — the MPC is a second project hiding inside M2). (3) The MCTS/sysid bullets (CTL-31
agrees). (4) The depths tier (FE-17). (5) 129² grids, 12-coil sets, 40-locale ASR ambitions —
every "range" in the spec collapses to its cheap end for v1. **The inversion worth stating:**
pull a thin slice of M3 forward — the tokenizer + floors + a text-only triple against a tier-0
plant (no GS, no voice) — so F-PRESENCE-F's rig exists by ~session 10 and the thesis meets
evidence early.

**Protect at all costs: the two-seam contract discipline + the determinism law that makes it
auditable** — innovation-gated tokenization upward, writs-through-floors with receipted
refusals downward, on a bit-replayable plant. That composite is the one architectural idea this
repo adds to the estate (auricle owns residency; the booster owns honest sim; TinyVillage owns
deadline law), and it is the thing every falsifier stands on. If everything else burns, a
tier-0 plant + events.toml + floors.toml + a triple that answers with receipts is still *the*
demonstration of the thesis.

### Q4 · If I were the V0.2 session tomorrow, what's missing from KICKOFF_SPEC_V02?

1. **Session-count honesty + internal ordering.** 12 deliverables + 99 dispositions is 2–4
   sessions. The kickoff should say so and order the work: machine.toml **first** (≈⅓ of all
   findings derive from it), then objective/dispersions/scenario-schema, then the rest —
   with a partial-commit discipline if the session ends mid-list.
2. **Kill the "SPARC/DIII-D-class" phrasing** (§2.3-D1) and replace with the derive-both-
   numbers-from-one-synthetic-machine instruction.
3. **Eight QC dispositions with no deliverable line.** The 12-item list omits: the **M2 ladder
   D-entry** (CTL-04 — the highest-stakes control disposition in the audit!), the
   diagnostics/sensor-model contract (CTL-25), plant-lane boundary definition (M-B6), the room-
   cascade guards (M-B7), vocabulary freeze at M3 (M-M13), the harm-dial rename (CTL-22),
   molt-on-calm (M-M6, folded half-visibly into "crisis scheduling"), and `tts.h`'s contract
   (V-15). The kickoff's safety net ("disposition every BLOCKER") covers them *only if* the
   session walks all 146 line items — these are the ones most likely to be shallow-dispositioned.
   Add them to the deliverable list or mark the five reports' finding-ID checklist as the
   *primary* artifact and the 12 deliverables as headliners only (it half-says this; say it).
4. **A conflict-ruling protocol.** The swarm had PHYSICS rule on Greenwald; v0.2 is one session
   with no peers. Name the tie-breaker (physics-first? estate-law-first?) for the ~6 places two
   lanes' fixes tension (e.g. M-M2 vs M-M9, §2.3-D2).
5. **Fresh-eyes items** (this file): the F-PRESENCE-F third arm + F-VERACITY belong in
   deliverable 6 (falsifier completions); the EKF information-set clause in the CTL-11 D-entry;
   the floor-legal-but-lethal class in the scenario schema; the writ/input tape in the writ
   grammar; the [health] debounce in the events.toml disposition; tape persistence (steal
   `durable.h`) in the membrane paragraph.
6. **Propagation instructions.** REPO's ripple map (the §6:284 "~50 ms" demo line; §1's flat
   100 µs table row, M-N5) names in-place v0.1 text the *applied* patches did not touch; v0.2
   must sweep them, plus the V-08 provenance fix (§2.3-D3). And v0.2's own new numbers need the
   MEM treatment: every inherited figure cited with its receipt path, every derived figure with
   its arithmetic (§0 of this file).

---

## 4 · Verdict + top-5 actions

**Verdict.** This is the most honestly-instrumented pre-build repo I have ever cold-read: the
citations survive independent re-derivation to the decimal, the QC swarm caught the great
majority of what one careful outsider finds plus ~25 things he didn't, and the process
architecture (state-of-truth log, append-only decisions, falsifier-first gates) is genuinely
sound. Two structural residues survive all six instruments so far: **the falsifiers audit the
machine and not the claim** (the twin is a strawman, truth-of-speech is unmeasured, the ghost's
value is confounded with the governor's — FRESH-1/2/3), and **the sequencing tests the proven
half first and the novel half last** (Q1/Q2), which is backwards for a falsifier-first estate.
v0.1 + QC + v0.2-as-mandated will produce a buildable spec; whether it produces a *tested
thesis* by session 50 depends on the five actions below.

**Top-5 actions, in priority order:**

1. **Fix the experiment before the machine** (FRESH-1 + FRESH-2 + §2.3-D2): three-arm
   F-PRESENCE-F (resident / event-triggered twin / polling twin), F-VERACITY on solicited
   answers at M4, and the post-M-M2 Sentinel-null statement — all into v0.2's falsifier
   deliverable. Cost: a page of spec. It converts the flagship claims from demo-able to
   testable.
2. **Pull the thesis forward** (Q3 inversion): add a ROADMAP note that a text-only membrane
   slice (tier-0 plant + events.toml + floors + triple, no GS, no voice) is a sanctioned early
   rig for F-PRESENCE-F/F-VERACITY dry runs — before M1's physics grind, not after M3.
3. **De-trap the V0.2 kickoff** (Q4 items 1–3): session-count honesty + machine.toml-first
   ordering; delete "DIII-D-class"; add the eight orphaned dispositions (M2 ladder first among
   them) to the deliverable list.
4. **Close the replay hole** (FRESH-6): accepted-writ + operator-input tape as recorded replay
   input, specified inside the writ-grammar v1 deliverable; plus the EKF information-set clause
   (FRESH-4) in the CTL-11 D-entry and the floor-legal-but-lethal scenario class (FRESH-3) in
   the schema.
5. **Sweep the small stuff while the file is open**: [health] debounce (FRESH-5), tape
   persistence via `durable.h` (FRESH-8), §7 CPU table (FRESH-7), V-08 provenance label
   (§2.3-D3), ghost render-path sentence (FE-19), `[sys] mind died` watchdog receipt (FE-20),
   depths-tier decision (FE-17), and the comprehension list (§1.2) into the glossary — the
   tokamak half as well as the SYNCYTIUM half.

---

*FRESH_EYES · one cold session, three passes · Pass-1 frozen before the seal came off ·
8 premise doubts · 22 comprehension failures · 27 flaws → 10 missed-by-all-five (2 gate-class),
~25 missed-by-me, 4 disagreements · 8 estate receipts re-verified (all consistent, incl. 921/hr
re-derived exactly) · 1 web check (sherpa/Nemotron: holds) · no fabricated numbers found.*

---

## 5 · Addendum (2026-08-09, same day) — the founding session's response, adjudicated

The founding/orchestrator session (S0/S1's author, low on context) read this file; the
operator relayed its response and ratified the adjudication below. Recorded in the estate's
own pattern — corrections on the record, original text above preserved.

**Conceded by the founding session (its errors, owned):** D1 (the "SPARC/DIII-D-class"
kickoff trap — fixed: synthetic FUSOR-1, both numbers derived from one config), D3 (the §7
renderer-row provenance — the ≈11.4 GiB total was adopted *from* V-08 while the mislabel
V-08 corrected was kept), FRESH-5 (its own `[health]` row leaking 300–400 tok/hr against
its own cap — "the swarm re-derived the two neighboring rows and skipped mine"), and
FRESH-2 outright ("no defense": a Speaker that fabricates rev citations passes all six
falsifiers as written). It unified FRESH-1 + P-1 + D4 as one discovery and sharpened it
better than I had: **the anti-turn advantage lives exactly where the world has not already
been compressed for you** — on pre-tokenized lanes the tokenizer is a threshold-waker and
residency's value is hot-memory integration, not wake timing. That narrowing is now
doctrine (D-018), and the three-arm twin is its instrument. The Q3 inversion was endorsed
without reservation ("fusion's shadow soak") → ROADMAP M0.5, D-019.

**Corrected by the founding session — I concede:** §3-Q1's sentence *"the demo's implicit
safety story ('the mind saved the plasma') is, by design, never true; what is true is 'the
mind explained the save'"* overreaches. On a floor-legal-but-lethal command, when the
operator heeds the ghost and belays, the mind saved the plasma **via the sovereign** — the
writ doctrine working exactly as designed, not theater. My own FRESH-3 is what makes that
story demonstrable rather than assumed: pre-register the class where the governor alone
would *not* have saved it, and the demo's honest line becomes "causal via the human, and
here is the receipt" — a better story than the one I deflated. The deflation stands only
against the *autonomous* reading ("the mind grabbed the stick"), which the architecture
rightly forbids. Retracted as stated; the requirement that the causal chain be receipted
stands.

**Adjusted on its reservation:** the tier-2 MHD cut (my Q3) is demoted from
recommendation-to-apply to **operator ruling** — an aesthetic call that belongs to the
operator's pen — with the sanctioned compromise (renderer raymarches tier-1-derived
emissivity + artistic noise) keeping the beauty either way. D-020 reserves it; the other
Q3 cuts (MPC null downgraded, MCTS/sysid parked, depths tier, ranges to cheap ends) were
endorsed and are ruled in D-020.

**One caution back (so this addendum is adjudication, not deference):** the founding
session's aside that auricle's F-PRESENCE twin "already includes a second-pass responder
(the event-triggered arm, roughly)" is generous — auricle's recorded null is a
**timer-twin** (`C:/auricle/SESSION_HANDOFF.md`: "beat a timer-twin over a live soak"),
i.e. the polling arm. The event-triggered arm appears to be new there too; the back-port
to the whitepaper's F-PRESENCE is an *addition*, not a relabel.

**Folded into the repo (same commit as this addendum):** `KICKOFF_SPEC_V02.md` de-trapped
and expanded (2–4-session honesty + machine-first ordering; deliverables 13–15; three-arm
twin + F-VERACITY + session input-tape + lethal-legal class into deliverables 4/5/6;
tie-break protocol) · ROADMAP M0.5 "the thesis slice" inserted before M1, M2's null
renamed, tier-2 parked-pending-ruling · `DECISIONS.md` D-018/D-019/D-020 appended ·
SESSION_LOG S2. The estate-portable pieces (three-arm claim-separation → auricle's
F-PRESENCE; citation-fidelity → the soak briefs) are the founding session's to carry home
on the next auricle touch — not this repo's edit.
