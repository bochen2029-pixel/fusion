# fusion — session charter

The reactor you can talk to. Read **[FUSION_ARCHITECTURE_v0.1.md](FUSION_ARCHITECTURE_v0.1.md)**
(the canon — spec + QC pass), **[ROADMAP.md](ROADMAP.md)** (current milestone + exit gates),
and the **tail of [SESSION_LOG.md](SESSION_LOG.md)** (the state of truth — the last entry is
where the project actually is) before changing anything. The next work is the first unchecked
milestone in ROADMAP.md unless SESSION_LOG's tail says otherwise.

## Non-negotiables (inherited law; violations are regressions)

1. **C/C++20 + CUDA only in the product.** Python confined to `trainer/` (the dev rig).
   PowerShell only for dev scripts in `tools/`.
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
the product build); tests are assert-based exes under ctest; `contracts/*.toml` tables are
pre-registered — change = a D-entry.

## Glossary (mandatory terms — drift fails review)

**fusor** (the core binary) · **spine/cerebellum/cortex/depths** (the four tiers) ·
**innovation** (sensor minus cerebellum prediction; the upward gate) · **writ** (a typed
downward reference; never an actuator) · **floor** (a governor limit the mind cannot cross) ·
**ghost** (a forked-physics counterfactual, before-commit) · **drop-event** (a recorded
deadline miss) · **the tokenizer of machine experience** (`membrane/`'s event emitter) ·
**solicited vs unsolicited** speech (answers vs initiative; D-007) · **the null** (the
baseline every organ must beat).

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
