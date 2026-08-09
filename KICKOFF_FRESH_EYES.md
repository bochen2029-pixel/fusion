# FRESH-EYES DEEP SCAN — C:/fusion (zero-context outsider review)

You are a brand-new session with zero prior context — **on purpose**. `C:/fusion` is a
pre-build spec-and-scaffold repository (no code yet) that has already been reviewed once by
a five-agent specialist swarm. You are the *second* instrument, and your value is exactly
what the specialists structurally lacked: you were not steeped in the project's framing.
Your job is a deep scan with fresh eyes — including permission to question the premises
themselves. You are NOT here to build anything.

## Part 1 · The background you need (read this first, or the repo will confuse you)

**The anti-turn thesis (the strange part — absorb this before opening any file).**
Ordinary LLM products are *turn-based*: the model is idle until someone invokes it, it
answers, it goes idle again. Something *else* always decides when it thinks — a user
pressing send, a cron job, an orchestrator. The estate this repo belongs to is built on
the claim that for one class of mind — the watching, interface, "presence" mind — this is
structurally wrong, and provably so via a regress: any idle-until-invoked mind needs a
waker, and the waker is either dumb (a clock or rule — blind to exactly the evidence-driven
moments that matter) or another turn-based mind (and then who wakes the waker?). The
regress terminates only at a mind that owns its own clock: a **resident** LLM whose KV-cache
(its working attention state) stays hot, into which the world's text streams continuously,
and which decides *for itself, at every completed thought, whether to speak* — with no send
key, no poll, no scheduler anywhere. This is not speculation in this codebase's lineage: it
was built and measured in `C:/auricle` (read-only parent repo). The measured mechanics you
will see cited: **decode-on-delta** (the speak/hold judgment rides free on the tail of the
same forward pass that ingests each word); the **isomorphic segmenter** (the model's own
next-token probabilities reveal where a thought ends — P(clause-end) ≈ 0.97 at a completed
thought vs 0.02 mid-phrase — so judgment fires only at real boundaries, no second model
needed); a **triple** of three minds (Speaker/Skeptic/Sentinel) sharing ONE attention trunk
at ~1.2× the cost of one mind; the **warpbus** (a two-plane shared bus: an append-only
hash-chained tape of committed messages + an ephemeral "reflex plane" where *forming*
partial thoughts are visible before commitment); and the **molt** (the mind reads its own
overgrown context and authors a compressed running-memory, ~11× on real content). Receipts
for all of this live in `C:/auricle/runs/*.md` and are cited by number throughout the spec
you're about to read. Known measured *problems*, also cited honestly: at zero calibration
the base model over-fires (~900 would-be interjections/hour on a real workday — unlivable),
and it confuses *watched* content with *addressed* content; a fine-tune ("the emit QLoRA")
is the planned fix, gated by a pre-registered falsifier.

**Why fuse a classical neural network with that (the project's actual move).** FUSION
composes two different neural tissues at two timescales, the way a body does:
- a **cerebellum** — a small, fast, *non-linguistic* control network (kHz-class, trained on
  error against a simulator) that actually flies the plant. Its parent is
  `C:/Booster_Lander_Simulator` (read-only): a 6-DOF rocket-landing sim whose constitution
  is "the physics cannot be faked" — state changes only through the integrator, no hidden
  assist toward success, deterministic bit-identical replay, headless Monte-Carlo success
  rates as the only gate, and if the controller can't solve the descent, the vehicle
  crashes. FUSION's plant is a tokamak: an elongated fusion plasma is *vertically unstable*
  (wall-set growth times, 10–100 ms class), so continuous kHz feedback is mandatory — a
  controller that "takes turns" loses the plasma. (Precedent that this is real: DeepMind ×
  EPFL, *Nature* 2022 — deep-RL drove a real tokamak's 19 coils at 10 kHz.)
- a **cortex** — the resident anti-turn LLM triple above, which cannot fly anything (far
  too slow) but can *watch, remember, judge, and talk*.
The two are joined by two seams, and the seams are most of the design: **upward**, the
plant does NOT stream raw telemetry into the LLM (wrong grain by ~4 orders of magnitude);
instead the plant "earns tokens by surprising its own cerebellum" — only prediction-error
(innovation) events and sparse silence-ticks become text on the bus. **Downward**, the LLM
NEVER writes an actuator value; it writes *writs* (typed setpoint/mode references) through
a deterministic governor with hard floors it cannot cross. On top: **full-duplex speech**
(mic → streaming ASR → the trunk; the Speaker → abortable TTS with sub-150 ms barge-in),
and **the ghost** — because the sim is deterministic and forkable, the Sentinel can run
your *forming* dangerous command 500 ms into the future and interrupt you with a rendered
counterfactual of the plasma dying, pre-verified by the same physics, before the command
commits. The endgame demo is a reactor you talk to, that talks back, and that talks you
out of killing it — with receipts.

**The house method (so the paperwork makes sense).** Everything is falsifier-first:
claims are staked so they can lose (pre-registered F-* tests), every organ must beat a
dumb baseline ("no organ outlives its null"), every number needs a dated receipt, replay
must be bit-identical (memcmp oracles), decisions are append-only (D-### entries,
superseded never edited), and the repo is built for dozens of sessions: SESSION_LOG.md is
the state of truth, each milestone has a KICKOFF prompt, docs sync in the same commit as
the work they describe.

**Where the repo stands right now.** Two commits: `c7d8ad0` (founding: spec v0.1 + the
multi-session scaffold) and `0f23676` (a five-lane specialist swarm QC — 146 findings, 39
blockers — whose *consensus* fixes were applied in-place as D-011…D-017, and whose
remaining blockers were mandated as a "V0.2 revision" milestone; M0, the first code, is
**suspended** until V0.2 exists). The five specialist reports + a consolidated synthesis
live in `docs/qc/`.

## Part 2 · Your task — three passes, in this exact order (the order IS the instrument)

**Pass 1 — the cold read. Do NOT open `docs/qc/` yet.** Read, in order: `README.md` →
`CLAUDE.md` → `FUSION_ARCHITECTURE_v0.1.md` (in full — it is the blueprint) →
`ROADMAP.md` → `DECISIONS.md` → `DEFINITION_OF_DONE.md` → `SESSION_LOG.md` →
`KICKOFF_SPEC_V02.md` → `contracts/events.toml`, `writs.toml`, `floors.toml`. While
reading, log three streams independently:
(a) **premise-level doubts** — question the project itself: is the four-tier composition
coherent? would you build it this way? what question is nobody in this repo asking?
(b) **comprehension failures** — every term, claim, or leap you could not understand from
the repo alone. You are the live test of this repo's central bet (that a cold session can
boot from these files); every place you needed outside knowledge is a finding, even if you
happened to have that knowledge.
(c) **independent flaws** — errors, contradictions, infeasibilities, gaps, with severity
(BLOCKER / MAJOR / MINOR / NIT) and location.

**Pass 2 — the diff.** Now read `docs/qc/QC_CONSOLIDATED.md`, then the five lane reports
(`QC_PHYSICS`, `QC_CONTROL`, `QC_MEMBRANE`, `QC_VOICE_RT`, `QC_REPO_PROCESS`). Produce
three lists: **missed-by-them** (your Pass-1 findings absent from all five reports — the
payload of this whole exercise), **missed-by-you** (their findings you didn't catch —
calibrate yourself honestly), and **disagreements** (where you think a finding, a
disposition, or one of the applied D-011…D-017 fixes is *wrong*, with reasons — the
specialists converged with each other on the bus, which is powerful and also exactly how
shared blind spots survive).

**Pass 3 — the outsider questions, answered explicitly, no politeness:**
1. **Steelman the case against the project.** The strongest honest argument that a talking
   fusion-reactor sim is the wrong thing to build — or the right thing built wrong.
2. **Scope survivability:** one operator + AI sessions + one 16 GB consumer GPU; the
   booster parent took dozens of sessions. Is V0.2 + M0–M5 as written realistically sized,
   and where will it actually die if not?
3. **What would you cut first, and what is the ONE thing you would protect at all costs?**
4. **If you were the V0.2 session tomorrow, what's missing from `KICKOFF_SPEC_V02.md`?**

## Part 3 · Deliverable + rules

Write **`C:/fusion/docs/qc/FRESH_EYES.md`** with sections: (1) Pass-1 cold-read findings
(the comprehension-failure list as its own subsection — it grades the repo, not you),
(2) the Pass-2 diff (missed-by-them / missed-by-you / disagreements, with reasons),
(3) the four outsider answers, (4) verdict + your top-5 actions in priority order. Then
reply in chat with a compact summary: counts, the headline findings, and your verdict in
three sentences.

**Rules:** the repo is READ-ONLY except your one deliverable file — no edits elsewhere, no
commits, no builds, no code. Parent repos (`C:/auricle`, `C:/Booster_Lander_Simulator`,
`C:/TinyVillage`) are read-only ground truth; you MAY verify the spec's citations against
`C:/auricle/SESSION_HANDOFF.md` and `C:/auricle/runs/*.md`. Web search is allowed for
physics/ML fact-checks; cite what you check. A token-aware chunker exists at `C:/chunker`
(`python C:/chunker/chunker.py --help`) but every file in this repo reads directly.
**Machine rule, non-negotiable: write Windows paths with FORWARD SLASHES in every shell
command** (`C:/fusion/...`, `python C:/chunker/...`) — Git Bash eats unquoted backslashes.
