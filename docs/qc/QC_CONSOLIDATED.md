# QC-CONSOLIDATED — the orchestrator's synthesis of the five-auditor swarm pass
**2026-08-09 · room `fusion-qc` on Intercom (replayable) · five Opus lanes, ~45 min, full cross-pollination**

**Totals: 146 findings — 39 BLOCKER · 60 MAJOR · 42 MINOR/NIT — plus 6 verified passes and
16 estate citations individually checked (13 verified, 1 wrong, 2 incomplete).** The five
reports are the worklist; this file is the ranked index and the disposition record. Read
them in this order: `QC_PHYSICS.md` · `QC_CONTROL.md` · `QC_MEMBRANE.md` · `QC_VOICE_RT.md`
· `QC_REPO_PROCESS.md`.

## Verdict

**v0.1 is not buildable cold — and the swarm proved it the right way:** not one auditor
found a fabricated number (every estate citation traced to a real receipt; every §5–§7
figure checkable against `C:/auricle/runs/` was honest). What they found is that the spec
inherits its parents' *names* without their *enforcement*, states discipline in prose
without mechanism, and — in the deepest catch — inherits one parent's *hope* instead of its
*measured null* (CTL-1: "LODESTAR doctrine, verbatim" cites a document whose own §11 is
titled *the honest wall*). The scaffold's process architecture is sound (REPO verdict);
what is missing everywhere is executable specificity. **Therefore: consensus corrections
are applied in-place today (D-011…D-017); the remaining blockers are design work, mandated
as a dedicated v0.2 revision session (`KICKOFF_SPEC_V02.md`) before any M0 code.**

## The eight blocker clusters (each merges findings from ≥2 lanes)

1. **No machine is pinned** (PHY-01 + PHY-02 + REPO): no R/a/B/I_p/κ anywhere, so M0's
   "Q>1" gate floats — and the two numbers that *are* stated (γ⁻¹ ~1–5 ms, Q>1) belong to
   machines that cannot be the same machine; γ is **wall-set (10–100 ms class), not κ-set**.
   *Disposition: γ corrected + §0 claim reframed today (D-016); `contracts/machine.toml`
   is v0.2's first deliverable; the necessity-of-a-net claim now rests on F-NULL-C, where
   it belonged.*
2. **The training doctrine mis-inherits its parent** (CTL-1/B4): the booster's end-to-end
   distillation *plateaued at 0/16* (measured null D-041); what shipped was a θ-predictor
   over an analytic base. *Disposition: spec corrected today to cite the wall; M2's
   pipeline becomes "oracle-guided, architecture decided at M2 against the parent's null,
   with a losing branch defined."*
3. **Determinism vs the policy** (CTL-B2 + VOICE GPU-arbitration + PHY escalation): a
   fp16/int8 CUDA policy feeding the integrator breaks memcmp; 5–7 CUDA consumers on one
   WDDM card break the 100 µs tick — and a preempted GS window leaves the loop on a stale
   linearization *the memcmp oracle is blind to*. *Disposition: D-012 today — policy on
   CPU at fixed precision (the parent's proven ~10 µs C net); ASR on CPU (auricle's
   recorded rule); GPU priority plant > LLM; the arbitration policy + tick-jitter gate is
   v0.2/M3 design work.*
4. **The event/token budget is self-contradictory by orders of magnitude** (CTL-B3 +
   MEM + PHY + REPO, independently): per-channel per-sample 3σ ⇒ ~1080 events/s vs a
   ≤4 tok/min budget; the tick cadence alone was 12–14× the prose law. *Disposition:
   events.toml redefined today — innovation on windowed cluster residuals (250 ms), storm
   aggregation (the flood→rate-cap immune row), tick interval 300 s, one budget source of
   truth. The full vocabulary redesign (incl. the missing DISRUPTION event) is v0.2.*
5. **"Commit" is undefined and the ghost's H1 guarantee is arithmetically impossible**
   (MEM + VOICE-B3 + CTL converged): warning lands ~1.9–3 s; an ASR-final commit lands
   ~350 ms after the last word. *Disposition: D-013 today — **commit = governor-accept,
   tick-stamped** (the room's single converged definition); numeric writs are H2 with a
   spine-side objection window + readback; the writ grammar gains id/supersedes/ttl/cancel
   in v0.2.*
6. **The mic hears the TTS** (VOICE-B1 + MEM independently): self-pausing barge-in AND the
   reactor's own words entering the trunk as operator speech. *Disposition: D-014 today —
   headset required in v1 (auricle's recorded precedent), AEC is v2, TTS lane-stamped and
   never enters `opv`.*
7. **The Deadline Law is mis-inherited and the membrane is oversubscribed in crisis**
   (MEM, with TinyVillage L2 verbatim): a p95 is not a fixed budget + abort; the
   `effective_tick` staleness half is missing; at measured fire-rates the membrane wants
   ~115% GPU duty mid-crisis and the molt fires ~10 min into one (an 8.9 s blackout at the
   worst moment). *Disposition: D-015 today — fixed 2 s budget + drop semantics restated;
   the crisis-scheduling design (probe batching, molt deferral window, dial) is v0.2.*
8. **The gates aren't testable as written** (REPO + CTL-B15/B16 + MEM + PHY, unanimous):
   no reward function anywhere; no disruption model behind F-NULL-C's primary metric; no
   held-out seed law; F-INSTINCT's eval set never created by any milestone; F-PRESENCE-F
   has no grader/rubric (TinyVillage L12 has the protocol); M0's "Q>1" has no duration or
   disturbance; a milestone whose null *wins* deadlocks the DoD (P-26 — the auditor found
   the defect in its own lane's files). *Disposition: DoD null-win clause patched today;
   everything else is the core of v0.2.*

## What was verified clean (so v0.2 doesn't re-litigate it)

Falsifier naming consistent across §8/ROADMAP/KICKOFF; all lineage paths exist; all three
contracts files parse as valid TOML 1.0; markdown links resolve; §10 layout matches disk;
founding commit clean; the LLM VRAM row conservative-and-correct against auricle receipts
(hybrid-SSM KV math confirmed); ASR-partial latency chain survives with margin; **and the
best positive finding of the day (VOICE-V04): sherpa-onnx already supports Nemotron 3.5
streaming (PRs #3044/#3671) — "primary" and "fallback" are the same backend; M4's ASR task
is a version bump + re-pin, not an export project.**

## Corrections applied in-place today (the consensus set)

γ⁻¹ → wall-set 10–100 ms + §0 reframe · LODESTAR honest-wall citation · policy-on-CPU
fixed-precision · Nemotron-via-sherpa unification · headset/AEC/self-echo · commit
definition + ghost→H2 objection window · events.toml budget coherence + storm aggregation ·
emit-rate citation 593→921/hr (full-day run C; and no estate p95 exists — maxima 2.1/5.5 s)
· §7 honesty ("fits" false at worst case; prescribed config ≈11.4 GiB; tier-2 row = solver
working set, fields are 32 MiB) · DoD null-win path · CLAUDE.md glossary + ui/-exception ·
two fork semantics (D-011). Everything else: **v0.2, with the five reports as the
line-item worklist.**

*Every disposition above is traceable: finding IDs in the five reports, discussion in the
`fusion-qc` room replay, decisions in DECISIONS.md D-011…D-017.*
