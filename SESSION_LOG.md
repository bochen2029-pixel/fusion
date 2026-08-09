# fusion — SESSION_LOG (the state of truth)

One entry per session, **appended at the end, never rewritten**. A fresh session reads the
tail and knows exactly where the project stands. Format: date · session id/handle · what
was done (with receipts/commits) · honest state · next.

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
