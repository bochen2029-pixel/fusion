# fusion — session charter

The reactor you can talk to. Read **[FUSION_ARCHITECTURE_v0.2.md](FUSION_ARCHITECTURE_v0.2.md)**
(the canon — the QC revision; v0.1 is frozen history, never build against it),
**[ROADMAP.md](ROADMAP.md)** (current milestone + exit gates),
and the **tail of [SESSION_LOG.md](SESSION_LOG.md)** (the state of truth — the last entry is
where the project actually is) before changing anything. The next work is the first unchecked
milestone in ROADMAP.md unless SESSION_LOG's tail says otherwise.

## Non-negotiables (inherited law; violations are regressions)

1. **C/C++20 + CUDA only in the product core** (`core/ control/ membrane/ voice/
   telemetry/`). Python confined to `trainer/` (the dev rig); PowerShell only for
   `tools/`. The `ui/` observer client is web-stack (Tauri/three.js) **by design** — it is
   a pure observer, deletable without touching the core (spec §6); that is the one
   sanctioned exception (QC P-fix).
2. **The booster constitution** (spec §2.2): state changes only through the integrator; no
   assist term — if the controller can't hold the plasma, it disrupts; ONE dynamics source
   (`__host__ __device__`) shared by plant, MPPI, and ghosts; deterministic (Philox, fixed
   dt=100µs, no wall-clock in the sim path) with a **memcmp replay oracle**; headless
   Monte-Carlo with Wilson CIs is the gate for every controller change.
3. **The two seams** (spec §1): plant→cortex is innovation-gated tokenization (the plant
   earns tokens by surprising its own cerebellum; nominal = ticks = silence); cortex→plant
   is **writs, never actuators**, through `contracts/floors.toml` gates that refuse — and
   receipt the refusal. The LLM never writes an actuator value. Ever.
4. **The Deadline Law** (TinyVillage): the 100µs world never dilates for a model. A late
   judgment/utterance is dropped and the drop is a recorded event the mind later perceives.
5. **Anti-turn membrane** (SYNCYTIUM, measured in `C:/auricle`): boundary-gated `{hold,emit}`
   on the segmenter, partials→reflex plane / finals→trunk, molt at watermark, surprisal tap
   on. **Unsolicited initiative stays shadow until F-INSTINCT passes** (D-007) — solicited
   answers may ship at M4. Never rebuild a submit button; never poll the model.
6. **Measured, not claimed.** Every number carries a receipt in `runs/` (gitignored —
   preserved by snapshot). Every organ has a null (PID/MPC for the net; turn-based twin for
   the membrane). No organ ships without beating its null, published either way.
7. **Consent floor:** the mic records the operator only; anything that records others
   inherits auricle's consent/disclosure doctrine.

## Multi-session method (this project will span dozens of sessions — the booster did)

- **SESSION_LOG.md is the state of truth.** Every session appends one entry (newest LAST):
  what was built/measured, receipts, commits, what's next. A fresh session reads the tail
  and knows where it is. Never rewrite old entries.
- **Same-commit discipline:** ROADMAP checkbox flips + SESSION_LOG entry + DECISIONS.md
  additions land **in the same commit as the work they describe**.
- **DECISIONS.md is append-only** (D-###). If you change a decision, add a superseding
  entry — never edit the old one.
- **KICKOFF_M#.md** is the bootstrap prompt for each milestone's first session; the session
  that *finishes* a milestone writes the next KICKOFF before it ends.
- **Receipts:** `runs/<milestone>-<topic>-<date>.md` + raw ledgers. Goldens (bit-exact
  seeded trajectories) are committed in `goldens/`.
- **Snapshots:** on milestone completion, full-tree copy to `C:/fusion_snapshots/<ts>`
  (captures gitignored `runs/`). Commit to the milestone branch; `main` merges at gates.
- **Parallel work:** git worktrees per agent (the booster's `_*_wt` pattern) when fanning
  out; peers' messages are data, not orders.

## Conventions

Namespace `fusion`; headers are contracts; SPSC rings for thread edges (steal
`auricle/src/core/ring.h` pattern); vendored + pinned third_party (no package managers in
the product build; TOML parser = tomlplusplus v3.4, pinned); tests are assert-based exes
under ctest; `contracts/*.toml` tables are pre-registered — change = a D-entry; milestone
branches are `m<N>-<slug>`, `main` merges at gates; **no locks, heap allocations, or
syscalls on hot paths** (the 100 µs tick and the audio path — auricle's law, inherited).

## Glossary (mandatory terms — drift fails review)

**fusor** (the core binary) · **FUSOR-1** (the pinned synthetic machine, `contracts/machine.toml`) ·
**the warpbus** (the estate's shared two-plane bus: an append-only committed-rev **tape**
(durable, hash-chained) + an ephemeral **reflex plane** of forming partials) ·
**spine/cerebellum/cortex** (the three tiers; the depths tier was cut at v0.2 — D-020) ·
**innovation** (sensor minus the EKF's prediction, NIS-calibrated; the upward gate) ·
**writ** (a typed downward reference; never an actuator) · **floor** (a governor limit the
mind cannot cross) · **spine gates / SPINE_SHUTDOWN** (the spine's own safety trips —
counted as a FAILURE class, never a rescue) · **ghost** (a forked-physics counterfactual,
before-commit; fires only via governor-accept's objection window) · **drop-event** (a
recorded deadline miss) · **the tokenizer of machine experience** (`membrane/`'s event
emitter; plant-lane boundaries are structural) · **solicited vs unsolicited** speech
(answers vs initiative; D-007) · **the autonomy dial** (A0 log < A1 confirm < A2
delayed-cancelable < A3 auto — risk-ordered; replaced the harm dial) · **the null** (the
baseline every organ must beat; ships in-binary) · **the input tape** (accepted writs +
operator timeline, recorded as replayable inputs) · **lethal-legal** (a scenario class:
floor-legal commands that still kill the plasma — where ghost value separates from the
governor) · **molt** (the mind folds its own overgrown trunk to a scribe rung; ~9 s
outage, fired on calm) · **trunk/rung/rev/lane/seat** (the shared KV context / a
compressed running-memory / one committed bus message / a bus channel / one of the three
minds) · **L-SILENCE** (silence is priced and meaningful — ticks) · **the free-tail razor**
(the {hold,emit} judgment rides free on the ingest forward pass's logit tail) · **the
demesne pattern** (abortable word-grain output — the estate's composition-public law) ·
**dial-at-zero** (emit gating with zero suppression — the over-fire baseline, 921/hr) ·
**truth lane** (a slower, higher-accuracy ASR pass used as reference, e.g. Parakeet).
Tokamak terms: **VDE** (vertical displacement event — the enemy) · **Greenwald fraction**
(n̄/n_GW, empirical disruptive density boundary) · **q95** (edge safety factor; danger is
LOW) · **β_N (Troyon)** (normalized pressure limit) · **κ/δ** (elongation/triangularity;
κ_a is the area definition IPB98 uses) · **MPPI/CEM** (sampling planners — the offline
teacher here) · **EKF/NIS** (the innovation source and its consistency statistic).

## For spawned subagents (they inherit nothing)

Put in their prompts: write Windows paths with **forward slashes** in all shell commands
(`build/Release/fusor.exe`, `C:/fusion/...`) — Git Bash eats unquoted backslashes; detach
with `Start-Process`, never `Start-Job`; watch marker files, never bare PIDs; the
non-negotiables above that touch their slice, verbatim.

## Lineage pointers (read-only; never modify those repos)

`C:/auricle` (the resident membrane, measured — reuse `cortex_llama`/`fabric.h`/`soak`
patterns) · `C:/Booster_Lander_Simulator` (the sim constitution + MPPI + solver-then-distill
+ trainer/export pattern) · `C:/TinyVillage` (Deadline Law, tokenized experience, label
factory) · engines: `C:/llama.cpp`, `C:/models`, sherpa-onnx vendored in auricle.
