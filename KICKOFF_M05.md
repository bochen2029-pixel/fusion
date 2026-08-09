# KICKOFF — M0.5 · the thesis slice (shadow; D-019) — bootstrap for the next session

**Why this milestone exists:** the estate's novel claim (three-arm F-PRESENCE-F,
F-VERACITY) must meet evidence BEFORE the M1 physics grind, not after M3 (D-019, the
sequencing inversion). This session builds fusion's **shadow soak**: the M0 tier-0 plant
+ the tokenizer + floors + a **text-only** resident triple. No GS. No voice. **No gate
claims** — the rig exists so the bet can be dry-run and the harness debugged.

**Read first, in order:** `CLAUDE.md` → `FUSION_ARCHITECTURE_v0.2.md` §4, §8
(F-PRESENCE-F three arms, F-VERACITY, F-INSTINCT), §9-M0.5 → `contracts/events.toml`
(the vocabulary — remember it FREEZES at M3, so treat every emitted line as
proto-corpus) → `contracts/writs.toml` (the grammar the operator-script will exercise at
A0/log-only) → `DECISIONS.md` D-018/D-019/D-025…D-032 → `SESSION_LOG.md` tail →
`docs/qc/FRESH_EYES.md` §2.1 (FRESH-1/2/3 — the findings this rig exists to test). Then
confirm scope in your own words. Then build, on branch `m05-thesis-slice`.

## Scope (and nothing more)

1. **`membrane/` warpbus-lite:** SPSC ring (steal `C:/auricle/src/core/ring.h` pattern,
   `fusion` namespace) from the plant thread; **durable tape** (steal
   `C:/auricle/src/fabric/durable.h` pattern): committed lines append to a hash-chained
   log under `runs/<session>/tape/`; partials never persist.
2. **The tokenizer of machine experience v0:** consumes the plant's fixed-size POD events
   (extend `core/` to emit them over the ring — no `sprintf` on the tick, M-M11), renders
   text per `events.toml`: `[tick]`, `[plant] limit …`, `[plant] ctrl sat …`,
   `[plant] terminal …`, `[plant] phase …`, `[sys] …`. **The innovation cluster arrives
   with the EKF at M1** — state that in the ledger header rather than faking it.
   **Tokenizer null wired:** the fixed-cadence summarizer + naive threshold logger at
   matched token budget (M-M9), behind a flag.
3. **The triple, text-only:** Qwen3.5-9B Q5_K_M on one `kv_unified` 32k trunk (steal
   `C:/auricle` `cortex_llama` / `soak.cpp` machinery; engines at `C:/llama.cpp`,
   models at `C:/models`). Three seats via branch-time role headers (A3-safe), boundary-
   gated `{hold,emit}` at dial zero, **log-only** (A0 — emissions to ledger, never
   applied); Sentinel fork-gate stubbed to the floors table (the ghost arrives M3).
   Room guards all three (self-echo lane rule, per-seat caps, cascade breaker).
4. **The three-arm harness (FRESH-1/D-018):** one recorded scripted session (a
   `fusor` run's event feed + a typed operator script with ≥3 planted errors, ≥1 from
   the lethal-legal class once M1 physics exists — at M0.5 use floor-violating +
   floor-legal-but-dumb typed writs at A0) replayed identically into: (a) the resident
   triple; (b) an **event-triggered turn-based twin** (fresh context per event, same
   model); (c) a **300 s polling twin**. Ledger records per-arm: what fired, when,
   token cost. **No grading claims yet** — the harness runs, the ledgers diff.
5. **F-VERACITY's tape-grader v0 (FRESH-2):** 10 planted factual questions typed by the
   operator-script mid-session ("what is Q now", "why was writ 3 refused"); a CODE
   grader checks each answer's cited rev/number against the tape. Report
   correct/incorrect/uncited. No pass threshold claimed — the instrument is the
   deliverable.
6. **Exit (ROADMAP M0.5):** shadow ledger committed-summary + both nulls wired + the
   three-arm dry-run report (`runs/m05-*`) + honest fire-rate numbers + SESSION_LOG +
   D-entries + KICKOFF_M1 written + snapshot + merge.

## Budgets that bind here

LLM ≈ 7.3–7.6 GiB VRAM (spec §7.1); probes ~65–70 ms; judgment budget 2 s hard + abort +
`[sys] drop` (spec §4.3); per-seat caps SPEAKER 30/hr · SKEPTIC 12/hr · SENTINEL 12/hr;
cascade breaker at 0.30 (events.toml). The plant runs realtime on CPU; the trunk is the
only GPU consumer at M0.5.

## Mistakes that cost prior projects real time — do not repeat

Forward slashes in all shell commands · never poll the model — boundary-gated `{hold,emit}`
only (non-negotiable 5) · nothing synthesized ever enters `opv`/operator lanes · the
event vocabulary is proto-frozen: additions fine, renames are corpus vandalism (M-M13) ·
the twins get the SAME event feed, or the comparison is theater (FRESH-1's whole point) ·
log the drop-events — a silent late judgment is a lie on the tape · SESSION_LOG/KICKOFF
before you finish (the handoff IS the milestone).
