# QC-VOICE-RT — audit of the voice loop, the budgets, and the real-time claims

**Auditor:** QC-VOICE-RT (Opus) · **Date:** 2026-08-09 · **Target:** `C:/fusion` @ pre-build (spec v0.1)
**Slice:** `FUSION_ARCHITECTURE_v0.1.md` §5 (all), §6, §7 (every row + the sum), §8
(F-VOICE / F-KEEPUP-F / F-GHOST), §9 M4 · `DECISIONS.md` D-004 / D-005 / D-008 / D-009.
**Ground truth consulted (read-only):** `C:/auricle` — `ROADMAP.md` (§Decisions + P0/P2/P3
measured numbers), `DESIGN.md`, `SESSION_HANDOFF.md`, `SYNCYTIUM_M0_LOCAL.md`,
`src/asr/asr.h`, `src/capture/wasapi_capture.cpp`, `docs/P0-TEST-PROTOCOL.md`,
`third_party/README.md`. **Web:** NVIDIA model card, sherpa-onnx issue #3664 / PRs #3044 &
#3671, community ONNX exports, Piper latency reports.

**Verdict in one line:** the voice loop is buildable cold — the ASR half is *lower* risk than
the spec believes — but three things must be written down first or the build goes wrong in a
way no amount of good code recovers: **the mic hears the speaker**, **nobody owns the GPU**,
and **the ghost's before-commit guarantee is arithmetically impossible at the harm dial v1
ships on**.

---

## 1 · Findings

| # | SEV | Location | Flaw | Fix |
|---|---|---|---|---|
| V-01 | **BLOCKER** | §5.1–5.3, D-005, §8 F-VOICE | **The mic hears the TTS — and it has two heads.** No AEC, no headset mandate, no playback-gated capture anywhere in §5. **(i) Acoustic head (mine):** barge-in fires on VAD energy (~20–30 ms), so on speakers the reactor's own first word trips its own barge-in ~30 ms after it starts talking → it soft-pauses itself, permanently. **(ii) Semantic head (QC-MEMBRANE, independently):** the TTS returns through the mic, is transcribed onto lane `opv`, enters the **trunk as operator speech**, and the Sentinel ghost-forks on the reactor's own words — a self-feeding loop in the memory, not just in the audio. The estate already ruled on the *weaker* version (cross-talk, not self-echo): `auricle/ROADMAP.md:151` — "*2026-08-06 · Headset required in v1. Deterministic me/them attribution beats AEC complexity; AEC is a v2 node, not a v1 gate*"; `auricle/DESIGN.md:139–141` puts WebRTC AEC3 (far-end = loopback) at v2. fusion's case is strictly worse, and the M5 demo video implies a room. | (a) D-entry mandating a **wired closed-back headset** for v1, with F-VOICE measured on it (edits §12 honest-limits too, not just §5); (b) a **self-speech gate driven by the render clock** — while our own PCM is in flight, barge-in VAD must clear an echo-aware threshold — or an AEC3 node with far-end = the TTS render buffer; (c) an **attribution rule at the `opv` seam** so nothing synthesized can ever commit to the trunk as operator speech (kills head ii even if head i is only mitigated); (d) F-VOICE gets an explicit **speakers-mode negative test** so the failure is a receipt, not a discovery on camera. |
| V-02 | **BLOCKER** | §7 (whole table), §8 F-KEEPUP-F | **No GPU arbitration policy exists** and the table has no row for contention. Play mode puts 5–7 CUDA consumers on one WDDM consumer card: tier-1 GS at 100–200 Hz, the policy net inside the 10 kHz tick, MPPI buffers, tier-2 MHD, the 9B decode, ASR (if GPU), plus the renderer in a **separate process**. Windows has no MPS; WDDM time-slices contexts at ~1 ms grain and one 9B decode step is ~17 ms of back-to-back kernels (auricle measured ~59 tok/s). A 100 µs deadline cannot be held against that by hope. auricle's recorded decision — "*2026-08-06 · ASR on CPU, GPU exclusively for the cortex … eliminates CUDA contention by construction*" — is the estate's own answer, and fusion silently drops it. | D-entry naming the doctrine: one plant-owned CUDA context + stream priorities, LLM decode chunked, **ASR and TTS pinned to CPU by default**, policy on CPU/AVX2 (also QC-CONTROL's B2 + B10 fix, for determinism and for launch-latency). Add a §7 contention row. **Move the tick-jitter measurement (p99.9 tick lateness with the LLM hot) to the M3 gate** — finding this at M5 is finding it too late (this edits ROADMAP-M3 exit + §9-M3, per QC-REPO-PROCESS). **Escalated by QC-PHYSICS (PHY-11) beyond latency into correctness:** a tier-1 GS solve is ~2–7 ms of GPU work (P-EFIT ~220 µs/Picard iteration at 65×65) that must land in a 5–10 ms window; preempted past its window by a ~17 ms LLM decode, the 10 kHz linearised vertical model runs on a **stale linearisation** — at γ⁻¹ ~ 10–50 ms that is a control-relevant modelling error, and the memcmp oracle **cannot** catch it (deterministic in sim time, non-deterministic in wall time). So also: the plant must **detect a missed GS window and emit a `[sys]` event**. |
| V-03 | **BLOCKER** | §5.4, D-009, §4.2 harm dial, F-GHOST | **The before-commit guarantee is arithmetically impossible at H1.** See chain D below: the Sentinel's warning lands ~1–3 s after the dangerous parameter is spoken, while *commit* (the ASR final) lands ~350 ms after the last word — and the parameter is usually the last thing said. At H1 (auto-apply within envelopes — what v1 ships) the writ is applied long before the ghost speaks. F-GHOST's "every Sentinel warning demonstrably precedes the command commit" **fails by construction**, independently of PHY-04's separate attack on the fork's compute budget. | Ship the voice-side ghost at **H2**, and redefine *commit* as **the end of a pre-registered objection window** on the writ path (≥ measured p95 Sentinel latency, ~1.5–2 s), not as the ASR final. **QC-CONTROL confirmed there is no hold/cancel path today** — `contracts/writs.toml` has no writ id, no revision/`supersedes`, no ttl, no cancel/revoke type, despite §4.2 calling writs "revs" — and added the load-bearing correction: **the objection window must live in the deterministic governor (spine-side, counted in ticks), never in the membrane**, because §4.3's Deadline Law explicitly permits the triple's judgment to be dropped, and a cancel path inside the organ allowed to miss its deadline is not a safety mechanism. It also needs **sequence numbers + last-writ-wins idempotence** (a writ stream with permitted drops has undefined ordering today). Note this contradicts `writs.toml:4` **and** §4.2:195 ("v1 ships at H0/H1") — so the D-entry must supersede that line in **both** places. |
| V-04 | MAJOR | §5.1, §12, D-004, §7 ASR row | **The Nemotron integration risk is overstated and the chosen mitigation is the wrong one.** sherpa-onnx **already supports** Nemotron 3.5 streaming: English via PR #3044, multilingual (`prompt_index` language conditioning) via PR #3671, which **closed** issue k2-fsa/sherpa-onnx#3664. Pre-exported ONNX exists (`pantinor/…-onnx`, `onnx-community/…-int4`), plus an independent numpy+onnxruntime streaming engine reporting WER Δ 0.0137 vs original and **3.8× realtime on CPU**; the official card also ships **GGUF + NeMo-Speech.cpp** (a native C++ runtime). "Primary" and "fallback" are therefore **the same backend** — one sherpa-onnx, two model dirs, behind the `asr.h` seam auricle already has. | Delete the §7 GPU ASR row (0.7–1.2 GB) and the Riva-container path (a container also drags Docker, which on this box lives inside WSL — a doctrine violation for a local C++ spine). Put ASR on CPU. Rewrite §12's ASR paragraph: the real risk is small and checkable — auricle pins **sherpa-onnx v1.13.4** (`third_party/README.md`, sha256), which **predates** Nemotron support, so M4's task is a **version bump + re-pin**, not an export project. |
| V-05 | MAJOR | §5.1 (160 ms), §7 ASR row, §4.2 writs | **Chunk size is chosen on latency with the accuracy cost unpriced — on a safety path.** Chunk *is* runtime-selectable (`att_context_size = [56, r]`, r ∈ {0,1,3,6,13} → 80/160/320/560/1120 ms), so 160 ms is reachable. But **every published WER is at 1120 ms, the slowest setting**: 7.91 % EN, 8.84 % avg over 19 locales (FLEURS). There is no published WER at 160 ms, and cache-aware FastConformer degrades as right context shrinks. At ~8–12 % WER a numeric writ parameter *will* be misheard in a way the governor cannot catch: "greenwald zero point **eight** five" heard as "**nine** five" is in-range and wrong, so `floors.toml` never fires. | (a) M4 produces a **measured latency-vs-WER curve** across the chunk ladder on tokamak vocabulary and picks the operating point by receipt (this is also what the domain fine-tune is for); (b) every numeric writ parameter requires spoken **readback + confirm**, or a constrained-grammar / keyword-boosted decode on the writ lane. Floors bound the *range*; nothing currently bounds the *transcription*. |
| V-06 | MAJOR | §7 tier-2 row | **The row's own formula does not reproduce its own number.** 128³ × 8 fields × fp16 = 2,097,152 × 16 B = **32 MiB**, not 0.25–0.5 GB — off by 8–16×. | The 0.25–0.5 GB is defensible as the *solver working set* (RK stages + directional flux + reconstruction, ~10–15× the field set) — say so. As written, a cold session recomputes, gets 32 MiB, and either "fixes" the budget or distrusts the whole table. *(QC-PHYSICS PHY-20 derived the same 33.5 MB independently and rates it NIT because it errs pessimistic; I keep MAJOR because the same field size drives V-07's bandwidth, where erring is not free.)* |
| V-07 | MAJOR | §6, §7 (missing row) | **The telemetry stream has no budget and its stated upper bound is infeasible.** §6 offers fp16 3-D fields 64³–128³ at 10–30 Hz over a socket. At the top: 32 MiB × 30 Hz = **1.0 GB/s** of device→host readback + socket + re-upload into a WebGPU 3-D texture. Readback **synchronizes the CUDA stream** — it manufactures exactly the plant-tick jitter F-KEEPUP-F forbids, on top of V-02. At the bottom (64³ @ 10 Hz = 42 MB/s) it is fine. | Add a §7 telemetry row with a bytes/s ceiling; pin the field stream at **64³ @ 10 Hz for play mode** (128³ only in headless capture); require async copy on a dedicated stream; and write the observer law explicitly — **the renderer may drop frames and may never backpressure the plant**. *(QC-PHYSICS converged: §6's "lock-free ring → socket" needs a **shared-memory / GPU-interop** path, not a TCP socket; and if only emissivity + B⃗ are shipped rather than all 8 fields it is ~380 MB/s — still interop territory.)* |
| V-08 | MAJOR | §7 total, D-008 | **"Fits" is false at the top of the stated range**, and the stated total is 0.5 GiB short of its own rows (see §2). Unpriced: CUDA context per process (~0.25–0.4 GiB), TensorRT workspace (0.2–1.0) if the GPU ASR path is taken, the **Tauri renderer is WebView2/Chromium** (a WebGPU volumetric raymarch with 3-D textures is +0.5–1.5 GiB and is **not** inside auricle's "~4.6 GiB desktop" receipt — that receipt is the plain live desktop, `SESSION_HANDOFF.md:177`, so the §7 row mislabels its provenance), and the ghost fork's tier-1 working set (§7 prices the ghost in **time only**, never in memory; N concurrent ghosts is N×). WDDM gives no reservation: the failure mode is eviction/paging → multi-ms stalls → tick drops. | Stop publishing a **range** and publish the **prescribed play config**: ASR+TTS on CPU (0), tier-2 at 64³ or off, lean desktop, renderer counted honestly ⇒ **≈ 11.4 GiB**, which fits with headroom. Keep the range only as a "what we refuse to run" note. |
| V-09 | MAJOR | §7 (missing row), §9 M4, F-VOICE | **The M4 gate's own number is not in the budget table.** "Operator's last word → first phoneme from the reactor" computes to **~2.5 s typical / 4+ s p95** (chain C). Worse, §5.2 anchors first-audio to "*after emit commit*", which forbids the only fix — streaming the emit into Piper at **clause grain**, using the boundaries the isomorphic segmenter already produces for free. | Add the row. Re-anchor: "first audio ≤ 300 ms after the **first clause boundary**" **plus** "last-word → first-phoneme ≤ 1.5 s p95". Make clause-grain synthesis a stated requirement of `tts.h` — Piper (VITS) synthesizes a whole sentence per call and does **not** stream internally, so a 3 s sentence at RTF 0.2 is 600 ms before the first sample. If the caller doesn't split, the budget is missed and Piper gets blamed. |
| V-10 | MAJOR | §7 barge-in row, F-VOICE | **150 ms is a single number with no sub-budget**, and it fits only under three unwritten laws (chain A): a hard cap on **queued PCM** (a normal anti-underrun writer keeps 200–500 ms buffered; a "pause" that merely stops writing takes 200–500 ms to be *heard*), pause = **zero/refill the queued frames + fade**, and **wired output**. **Bluetooth is disqualifying**: BT render adds 100–200 ms of codec+radio latency downstream of the app, uncancellable — and auricle's own P0 protocol tests on a Bluetooth headset (`docs/P0-TEST-PROTOCOL.md` §3). | Decompose the row into its five terms with a named owner each; add the queued-PCM law to `tts.h`; name "wired" in F-VOICE. |
| V-11 | MAJOR | §8 F-VOICE | **F-VOICE is not measurable as written.** "barge-in pause ≤ 150 ms measured end-to-end (mic → pause)" defines neither t₀ (acoustic onset? first sample containing speech? the VAD verdict?) nor t₁ (the governor's decision? the last sample *written*? the last sample *audible*?). Only *acoustic onset → last audible sample* is honest. auricle already owns the instrument: `auricle selftest` = TTS → our loopback → whisper oracle, PASS on both loopback paths (`README.md:25`, `ROADMAP.md:22–23`), and `asr.h` already stamps utterance-start + emit QPC "*so word→glass latency is measurable end to end*". | Name the rig (loopback capture of our own render + QPC alignment), define t₀/t₁, and report a **distribution** (p50/p95/max over N ≥ 100 barge-ins) — the same discipline M4.5's lag curve already uses. Also: "zero un-receipted audio" needs a **sample-accurate cut point** in the voiced-vs-drafted ledger, which means reading the render client's position at the cut. **Direct answer to QC-REPO-PROCESS's DoD question — is F-VOICE testable by a session holding only this repo? No, on both clauses**: the barge-in clause has no defined t₀/t₁ and no rig; "zero un-receipted audio" has no defined unit of account (samples? utterances?) and no comparison procedure. Both become testable with the two additions above, and both then need `runs/` receipts naming the audio device — because the answer differs by device (V-10). |
| V-12 | MAJOR | §1 tier table vs §3 | **MPPI: online or offline?** §1 lists the cerebellum as "policy net + MPPI"; §3 calls CEM/MPPI the **offline** oracle. If it is online in play mode, both §7's "plant + policy + MPPI ≤ 0.3 GB" and the 100 µs tick are wrong. (Raised to QC-CONTROL, whose slice owns it.) | State it once, in §1, and make §7's row follow. |
| V-13 | MINOR | §4 (SENTINEL) | "**lane-H partials**" names a lane that does not exist — the lane list is `plant / opv / opt / spk / skp / sen / writ / sys`. It is an auricle-ism (Lane-H = the human box, cf. `SESSION_HANDOFF.md:214`). | Read `opv` partials (reflex plane) and/or `opt`. One-word fix, but a cold session will hunt for lane H. |
| V-14 | MINOR | §5 whole, §10 | **No thread/ownership map for the voice organs.** §6 names rings for telemetry only. Nobody owns: capture thread, VAD, ASR worker(s), the duplex governor, TTS synth, the render thread — nor which edges are SPSC rings. auricle has an explicit thread table (`DESIGN.md:118`, "T-dsp: resample, VAD, (AEC)"), and its charter's hot-path law ("no locks, heap allocations, or syscalls on the audio path") is nowhere in fusion's CLAUDE.md. | Inherit auricle's thread table into §5 and the hot-path law into CLAUDE.md. |
| V-15 | MINOR | §5.2, §10, DECISIONS | **`tts.h` has no contract and no D-entry** (D-004 covers `asr.h` only). Undefined: sample rate/format, streaming granularity, abort semantics (does abort return the cut point?), the queued-PCM cap, and whether Kokoro/NVIDIA TTS are real options or decoration. | A D-entry mirroring D-004, plus the three requirements from V-09/V-10/V-11. |
| V-16 | MINOR | §5.3 | "**resume-or-abandon, model's choice at the next boundary**" is unimplementable as stated: resuming mid-word requires retaining the synthesized PCM and a splice policy (re-say the interrupted word? cross-fade? restart the clause?). The ledger must record which. | Specify the splice unit (recommend: resume at the last **clause** boundary before the cut, never mid-word) and record it in the voiced-vs-drafted entry. |
| V-17 | MINOR | §7 | **No row for VAD or the duplex governor at all** — the two organs on the critical path of the only latency falsifier. | Add them, even at ~0 VRAM, so the CPU/thread cost is owned. |
| V-18 | NIT | §5.2 | "VAD (~20–30 ms energy/Silero detect)" conflates two different detectors with different latencies and *very* different self-echo behaviour (see V-01). Silero at 16 kHz consumes **512-sample = 32 ms** frames and typically needs 1–3 past onset. | Say which, and state the frame count. |

**Counts:** 3 BLOCKER · 9 MAJOR · 5 MINOR · 1 NIT = **18 findings**.

---

## 2 · The arithmetic, re-done

### 2.1 VRAM (GiB, RTX 4070 Ti SUPER 16 GB)

| row | low | high | note |
|---|---|---|---|
| Qwen3.5-9B Q5_K_M + 32k KV + compute | 7.3 | 7.6 | **CONFIRMED** — see 2.2 |
| Nemotron 0.6B ASR | 0.7 | 1.2 | 0.6B **fp16 = 1.2 GB in weights alone**, so 0.7 is only reachable at int8; TRT workspace (0.2–1.0) uncounted. Per V-04 this row should be **0** (CPU). |
| TTS (Piper) | 0 | 0.3 | CPU-capable → 0 in the prescribed config |
| plant tier 0/1 + policy + MPPI | 0.3 | 0.3 | contested by V-12 |
| tier-2 MHD | 0.25 | 0.5 | formula in the row yields 32 MiB (V-06) |
| renderer + desktop | 2.0 | 4.6 | provenance mislabelled (V-08) |
| **my sum** | **9.85** | **14.5** | spec states "≈ 10.5–14": the low assumes ASR-on-GPU; **the high is 0.5 GiB short of its own rows** |
| unpriced: CUDA ctx / TRT workspace / Chromium GPU proc / ghost working set | +0.3 | +1.7 | |
| **honest worst case** | | **≈ 16.2** | **> 16 GiB card**, and WDDM has no reservation → eviction → ms-scale stalls → tick drops |
| **prescribed play config** (ASR+TTS on CPU, tier-2 @ 64³, lean desktop, ctx counted) | | **≈ 11.4** | fits with headroom — publish **this**, not a range |

### 2.2 The LLM row — confirmed against auricle receipts

`SESSION_HANDOFF.md:87`: Qwen3.5-9B is a **hybrid attention/SSM** model — only **8 of 32
layers grow a KV cache** (`full_attention_interval=4`) — so KV = **32 KiB/token, not 128**;
weights **6.13 GiB**; single-mind footprint **measured 7.0 GiB**; **N=3 ≈ 6.9 GiB flat** on
the shared `kv_unified` trunk. 32 KiB × 32,768 = **1.00 GiB** exactly. §7's 7.3–7.6 stands,
slightly conservative. *(Relayed to QC-MEMBRANE as a confirm.)*

### 2.3 Chain A — barge-in, budget 150 ms (mic → pause)

```
acoustic → WASAPI capture packet        20–30 ms   auricle MEASURED ~24 ms (48k stereo → 16k mono)
VAD decision (Silero, 32 ms frames)     32–96 ms   1–3 frames past onset; spec's "20–30" is 1.5–3× optimistic
ring hop → duplex governor               1– 5 ms
fade-out (anti-click)                    5–20 ms
render period / already-queued PCM       3–10 ms   ONLY if queued PCM is capped; else +200–500 ms
────────────────────────────────────────────────
TOTAL                                   74 ms best / ~142 ms typical   → fits, ZERO margin
+ Bluetooth render                    +100–200 ms  → DISQUALIFYING, uncancellable
```

### 2.4 Chain B — ASR partial, budget 300 ms  ✅ **the one row that survives untouched**

```
chunk close (160 ms chunks)              0–160 ms  (mean 80)
capture + resample                      20– 30 ms
model compute                           ~42 ms CPU (measured 3.8× RT) / 10–25 ms GPU
ring → consumer                          1–  5 ms
────────────────────────────────────────────────
TOTAL                                   ~150 ms mean / ~235 ms worst (CPU)
cross-check: auricle measured "word→partial ≈ model chunk (~320 ms) + ~30 ms" (ROADMAP:27)
```

### 2.5 Chain C — the missing row: last word → first phoneme (the M4 gate)

```
ASR endpoint silence                       350 ms  (spec's own ~0.35 s; auricle measured 1.4 s at its 0.9 s default)
final → trunk commit (opv)                5–20 ms
segmenter boundary + {hold,emit} probe     330 ms  (auricle M4.5 MEASURED: lag mean 327 ms, max 2.1 s; probe ~65 ms)
emit generation p95                       1500 ms  (§7's own number; ~88 tok at the measured ~59 tok/s)
TTS first audio                            300 ms  (§5.2, anchored to EMIT COMMIT)
────────────────────────────────────────────────
TOTAL                                     ~2.5 s typical, 4+ s p95      ← not in §7, not in F-VOICE

WITH CLAUSE-GRAIN TTS (the fix §5.2 currently forbids):
  350 + 20 + 330 + (TTFT 24 + ~8 tok ≈ 140) + Piper 150–300  =  ~1.0–1.2 s   ← a demo
```

### 2.6 Chain D — the ghost, and why V-03 is a blocker independent of PHY-04

```
parameter spoken → ASR partial carries it        160–300 ms
Sentinel judges (segmenter boundary + probe)     330 ms mean, up to 2.1 s
fusor_fork + lookahead                           25 ms claimed (PHY-04: 50–100 ms of GS alone)
emit gen + TTS first audio                       ~300–1500 ms
────────────────────────────────────────────────
warning audible                                  ~1–3 s after the parameter is spoken
COMMIT (ASR final)                               ~350 ms after the last word
⇒ the warning lands AFTER the commit, by roughly an order of magnitude.
```

Even with a **free** fork the guarantee fails: judgment latency dominates. PHY-04's fix (c)
"cut lookahead to ~100 ms" therefore does **not** rescue before-commit — only a writ-side
**objection window** does.

---

## 3 · Cross-pollination log

| with | direction | content |
|---|---|---|
| QC-PHYSICS | **confirmed** their PHY-04 (ghost budget vs D-003) | Their attack is on the fork's GS cost; mine (V-03) is on judgment latency. They compose, and mine survives even if the fork is free. Endorsed their (a)+(b) — declare the ghost's approximation mode; rescope F-GHOST to ghost-vs-ghost determinism + bounded divergence. **Refuted their (c)** ("cut lookahead to ~100 ms") as a sufficient fix: it rescues the compute budget, not the before-commit guarantee. |
| QC-PHYSICS | **received, folded into V-02** (their PHY-11) | They escalated my GPU-arbitration blocker with a measured number (P-EFIT ~220 µs/Picard iteration at 65×65 → 2–7 ms per equilibrium) and, more importantly, from *latency* into *correctness*: a preempted GS window leaves the 10 kHz linear model on a stale linearisation, which memcmp cannot catch. Added their `[sys]` missed-window event to my fix. Also converged independently on the tier-2 arithmetic (PHY-20) and on socket-vs-interop. |
| QC-CONTROL | **confirmed** their B2 + B10 (determinism / launch-latency → policy on CPU) | Two independent second arguments for my V-02: policy-on-CPU removes a CUDA consumer from the 100 µs tick, and their B10 number (4–8 kernel launches × 3–10 µs = 20–60 µs of launch overhead alone, no preemption guarantee, no TCC/MPS on Windows) shows the <50 µs claim is launch-bound, not FLOP-bound. Three arguments, one fix. |
| QC-CONTROL | **raised to them, answered** (V-03, V-05, V-12) | I asked whether the governor has any hold/cancel path. **Answer: none** — `writs.toml` has no id, rev/`supersedes`, ttl, or cancel type. They confirmed my objection-window fix and corrected it in a way that matters: **spine-side, in ticks, not in the membrane** (the Deadline Law permits dropping the judgment), plus sequence numbers + last-writ-wins. Folded into V-03. Still open with them: MPPI online-vs-offline (V-12), and the transcription hole (V-05 — floors bound the *range*, nothing bounds the *transcription*). |
| QC-MEMBRANE | **confirmed for them** (2.2) | The 32 KiB/tok KV figure, 6.13 GiB weights, 7.0 GiB single-mind / 6.9 GiB N=3 flat — all verified in auricle receipts; §7's LLM row is right and slightly conservative. Also flagged V-13 (`lane-H` is an undefined lane in their §4). |
| QC-MEMBRANE | **received, merged into V-01; endorsed their Sentinel fix** | They found the **semantic** head of the echo problem independently (TTS → mic → `opv` → trunk as *operator* speech → Sentinel forks on the reactor's own words) while I had the acoustic head; V-01 now carries both plus an `opv` attribution rule. Separately I endorse their fix for the Sentinel's over-fire — **gate the Sentinel's emit on "`fusor_fork` returned disruption", not on the LLM's `{hold,emit}` margin**. It doesn't change my latency arithmetic (chain D is unaffected), but it converts the estate's worst-measured seat (1,630 of 1,817 emits) into its only mechanically-verified one, and it is the right partner to the objection window: the window bounds *when* it may speak, the fork bounds *whether*. |
| QC-REPO-PROCESS | **relevant to them / answered their DoD question** | `third_party/` does not exist yet, and V-04's fix is precisely a pinned-dependency action (sherpa-onnx version bump + sha256 re-pin) that belongs in their vendoring checklist. They correctly note my fixes edit files beyond §5: the headset mandate touches §12; the M3 tick-jitter move touches ROADMAP-M3 + §9-M3 + a §7 row; and the H2/objection window **contradicts `writs.toml:4` and §4.2:195**, so the D-entry must supersede both. F-VOICE testability answered in V-11: **no, on both clauses**. |

---

## 4 · Verdict

**Is the voice loop buildable cold? Yes — after three paragraphs are written.** The
components are all proven or provably available on this box: WASAPI capture is measured
(~24 ms), sherpa-onnx is vendored and running today *and already speaks Nemotron*, Piper is
comfortably fast enough on CPU, and the ASR-partial budget is the one number in §7 that
survives an independent re-derivation with margin. The spec's own pessimism about the ASR
path is its largest factual error, and correcting it *also* fixes the VRAM sum and the GPU
contention — ASR belongs on the CPU, exactly where auricle already put it.

What is **not** buildable cold is the loop as currently written, because a cold session will
(1) play TTS through speakers and watch the reactor mute itself, (2) put everything on the
GPU and never learn why the plant hitches, and (3) implement F-GHOST's before-commit
guarantee at H1, where it cannot pass. None of the three is a hard problem; all three are
invisible until built.

**Top 3 risks**

1. **Self-echo / no AEC (V-01).** The single most likely way the demo fails on camera, and
   the cheapest to prevent: one D-entry (wired headset) plus one render-clock gate. Left
   unwritten, it will read as "the voice loop doesn't work" rather than "we didn't say
   headset".
2. **GPU contention with a hard-real-time plant (V-02).** The deepest risk, because it is
   *architectural* and cannot be patched late: F-KEEPUP-F is the falsifier most likely to
   fail, the fix (CPU for ASR/TTS/policy, one owned CUDA context, stream priorities) has to
   be a design constraint from M1, and the measurement has to move from M5 to M3. QC-PHYSICS
   raised its ceiling for me: it is not only a latency risk but a **correctness** one — a
   preempted GS window silently feeds the 10 kHz loop a stale linearisation, and the
   determinism oracle is structurally blind to it (bit-exact in sim time, wrong in wall
   time). It is the one risk here that can be *invisibly* true.
3. **Perceived latency, not measured latency (V-09 + V-03).** §7 budgets the 150 ms number
   that F-VOICE tests and omits the ~2.5 s number the operator actually feels. A build can
   pass F-VOICE and still feel broken. Clause-grain TTS and a writ-side objection window fix
   the felt loop and the ghost guarantee together — and both are spec edits, not code.

*One structural note beyond the findings: every latency claim in §5–§7 that I could check
against `C:/auricle/runs/` was honest. The failures above are all failures of **omission** —
missing rows, missing sub-budgets, missing measurement rigs — not of exaggeration. That is
the good kind of pre-build spec to be auditing.*
