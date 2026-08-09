# QC — REPO & PROCESS (the scaffold as a system)

Auditor lane: **QC-REPO-PROCESS** (`tesrynry`) · 2026-08-09 · repo state audited: founding
commit `c7d8ad0`, branch `main`, working tree **clean**.

Scope: README · CLAUDE.md · ROADMAP.md · DECISIONS.md · SESSION_LOG.md ·
DEFINITION_OF_DONE.md · KICKOFF_M0.md · `contracts/{events,writs,floors}.toml` ·
`.gitignore` · directory skeleton · git state · spec §9–§12 + cross-checks into §0–§8.

Question answered: **can a cold session boot from this repo and not go wrong?**

---

## 1 · Findings table

Severity: **BLOCKER** = cold session blocked or misled immediately · **MAJOR** =
contradiction/missing piece that bites within a milestone · **MINOR** = gap · **NIT** = wording.

| # | SEV | File : loc | Flaw | Fix |
|---|---|---|---|---|
| P-01 | BLOCKER | `KICKOFF_M0.md:20` + `CLAUDE.md:55` + spec §10:372 | **No TOML parser is specced and `third_party/` does not exist.** KICKOFF item 3 orders `contracts/scenarios/easy.toml` (create it) → M0 must read TOML on day one. Charter mandates "vendored + pinned third_party (no package managers in the product build)" but there is no dir, no README plan, no named library. Cold session hand-rolls a parser (contract semantics silently diverge), reaches for vcpkg (violates non-negotiable #1), or stalls. Parent `C:/auricle/third_party/` has `{README.md, dl, llama.cpp, sherpa-onnx}` + a `third_party/dl/` ignore rule; fusion inherited the words, not the structure. | Name and pin the parser (toml++ v3.x, header-only, is the obvious choice); create `third_party/README.md` with the vendoring plan; add `third_party/dl/` to `.gitignore`. |
| P-02 | BLOCKER | `KICKOFF_M0.md:20` + spec §2.3:109 | **Scenario file format is undefined.** §2.3 says only "Each scenario = seed + config TOML". No key schema, no units convention, no required-field list, no example. KICKOFF says "pre-register before tuning" — you cannot pre-register a schema that does not exist, and the whole M1 curriculum inherits whatever M0 improvises. Merges with QC-PHYSICS PHY-05: the schema is also where the *stochastic* model must live (see §4). | Ship a commented `contracts/scenarios/_TEMPLATE.toml`: required keys + units per key, the seed→Philox-stream mapping, and the jitter/diagnostic-noise/disturbance distributions. |
| P-03 | BLOCKER | repo-wide (`CLAUDE.md` has no Build/test section) | **Zero build commands exist in the entire repo.** Grep of every `.md` yields only `KICKOFF_M0.md:13` ("CMake skeleton … C++20; CUDA optional at M0") and spec §10:372 ("CMake, targets …"). No generator string, no `cmake_minimum_required`, no MSVC/CUDA toolkit version, no ctest invocation, no run command. Parent `C:/auricle/CLAUDE.md` carries an explicit Build/test block (`cmake -S . -B build -G "Visual Studio 17 2022" -A x64` → `--build --config Release` → `ctest --test-dir build -C Release`, plus the `vswhere` fallback). Not "blocked" but worse: **every session invents its own toolchain**, so the reproduce-line on each milestone receipt differs. Also `KICKOFF_M0.md:13` orders "forward-slash build commands in all docs" — there are no build commands in any doc. | Copy auricle's Build/test block into `CLAUDE.md` verbatim; pin generator/arch/std now and the CUDA toolkit when it lands at M1. |
| P-04 | MAJOR | `contracts/floors.toml:11-13` | **Two floors are unit-bearing strings** — `ip_ramp_max = "0.15MA/s"`, `n_ramp_max = "0.05e20/s"` (`tomllib` confirms type `str`); every other floor in the file is a float. The governor is declared "deterministic, spine-enforced" yet now needs a runtime unit-parser with no specced grammar, and the two strings do not even share a unit style (`MA/s` vs bare `e20/s`). | Units in the **key**, float in the value: `ip_ramp_max_MA_per_s = 0.15`, `n_ramp_max_e20_per_s = 0.05`. |
| P-05 | MAJOR | `contracts/events.toml:17` | **q95 limit direction is unencoded and inverted vs its neighbours.** `greenwald {warn 0.80 → alarm 0.92}` and `beta_n {0.85 → 0.95}` ascend; `q95 {warn 2.6 → alarm 2.35}` descends (a lower-limit approached from above). A cold implementer writes one comparator (`fire when v >= warn`) and gets q95 exactly backwards — alarming through healthy operation, silent all the way into the floor at 2.2. | Per-limit `dir = "above" \| "below"`. |
| P-06 | MAJOR | spec §4.2:191 **vs** `contracts/floors.toml:7` | **Governor number contradiction on a safety limit.** §4.2 states the governor as "q95 ≥ 2.2, Greenwald f ≤ 1.0"; `floors.toml` says `greenwald_max = 0.95`. q95 agrees (2.2 both); Greenwald does not. Canon and the pre-registered table disagree about where the floor is. QC-PHYSICS ruled the table correct (and separately flagged the table's *comment* — the Greenwald limit is an empirical disruptive boundary, not a physical wall at 1.0). | §4.2 → "Greenwald f ≤ 0.95 (deliberate margin below the empirical boundary)"; prose cites the contract rather than restating a second number. |
| P-07 | MAJOR | spec §4.1:180 **vs** `contracts/events.toml:4,12,28-29` | **The budget law does not survive its own arithmetic.** §4.1 and the events.toml header both state "nominal hour ≤ ~100 tokens". But `[ticks] nominal_interval_s = 30` → 120 tick-lines/hour, and §4.1's own specimen line `[tick +30s nominal Q=8.1]` is ~10–12 tokens → **~1,200–1,400 tokens/hour from ticks alone**, before a single event. Independently `[budget] max_tokens_per_min_nominal = 4` → 240/hour, contradicting both the ≤100/hour prose *and* its own tick cadence (~20 tok/min). Three numbers, no two agree; both readings of "token" (LLM-token or event) break. Crisis side is consistent (40/s matches §4.1). Independently derived by QC-MEMBRANE, who rates it BLOCKER for M3. | Pick one: lengthen the tick interval and shorten the tick form, or restate the law in the units actually meant. |
| P-08 | MAJOR | `CLAUDE.md:11` **vs** spec §10:367, §6:279, §9-M0:334, `ROADMAP.md:10`, `KICKOFF_M0.md:24` | **The charter's hard law forbids the UI it ships in M0.** Non-negotiable #1 is an exhaustive enumeration — "C/C++20 + CUDA only in the product. Python confined to `trainer/`. PowerShell only for dev scripts in `tools/`." JS/TS and Rust appear nowhere. Yet §10 specifies `ui/ Tauri + three.js/WebGPU`, §6 details it, and a three.js dashboard is in **M0 scope** per §9, ROADMAP and KICKOFF. §6:276 ("Delete `ui/` and `fusor` still runs") is the *rationale* for an exemption that is never stated. Either the session refuses M0 item 6, or it learns on day one that the non-negotiables are soft — the expensive outcome for a doctrine-heavy repo. Compounding: Tauri implies cargo + npm vs `CLAUDE.md:55` "no package managers in the product build". | Amend non-negotiable #1 with the carve-out in the same breath as the law: renderers/clients are out-of-process pure observers, JS/TS/Rust permitted **in `ui/` only**, the product *loop* is C/C++20+CUDA — and state explicitly whether `ui/` may use npm/cargo. |
| P-09 | MAJOR | `DEFINITION_OF_DONE.md:7-9` **vs** `KICKOFF_M0.md:12,22` | **M0 cannot satisfy its own DoD.** DoD #2 requires "goldens updated and committed in `goldens/`" for every milestone. KICKOFF's scope list is explicitly closed ("M0 scope (and nothing more)") and none of its six items creates a golden — item 5 is an in-run self-comparison (same seed twice → identical hash), not a committed artifact. `goldens/` ships with only a `.gitkeep`. The session must violate "nothing more" or fail DoD #2. Additionally **"golden" is nowhere defined** (state-vector dump? trajectory hash? at what cadence? what tolerance?). | Add a KICKOFF item 5b (commit `goldens/m0-easy-<seed>.golden`) and define the golden artifact format once, in the spec or DoD. |
| P-10 | MAJOR | `CLAUDE.md:48` + `DEFINITION_OF_DONE.md:17` **vs** `KICKOFF_M0.md:27-29` + git state | **Branch discipline is unnamed and unreachable.** CLAUDE.md: "Commit to the milestone branch; `main` merges at gates." DoD #6: "the milestone committed on its branch." **No naming convention exists anywhere** (auricle's own style is `tier1.5-syncytium`), and KICKOFF's exit-gate checklist — the only checklist the M0 session actually follows — omits the branch step entirely. Repo has one branch, `main`. M0 will therefore be done on `main`, silently breaking DoD #6 in session 1, while D-010 declares the method "binding from session 1". | Name the convention in CLAUDE.md (e.g. `m<N>-<slug>`) and add `git checkout -b <branch>` as KICKOFF step 0. |
| P-11 | MAJOR | `CLAUDE.md:59-67` (Glossary) | **The glossary omits the repo's most-used term.** "warpbus" appears 8× across README, CLAUDE.md, ROADMAP, DECISIONS and the spec and is **defined nowhere**. Same for `molt`, `L-SILENCE` (used in §1:52 and the §11 doctrine table), `trunk`, `rung`, `seat`, `lane`, `rev` ("every claim citing a bus rev"), `two-plane`/`reflex plane`, `surprisal tap`, and the harm dial H0–H3. CLAUDE.md calls the glossary "mandatory terms — drift fails review", so the list is load-bearing by its own claim. A session that has not read `C:/auricle` cannot define these from this repo — which defeats the self-bootstrapping premise. | Add the inherited SYNCYTIUM terms with one-line definitions (or an explicit "read `C:/auricle/DESIGN.md` §X before M3" gate in ROADMAP-M3). |
| P-12 | MAJOR | missing `.gitattributes` + `core.autocrlf=true` | **The determinism gate's own evidence is exposed to line-ending rewriting.** Measured: `git config core.autocrlf` → `true`; `git ls-files --eol` → every tracked file `i/lf w/lf **attr/**` (no attributes at all). With autocrlf on and no `.gitattributes`, a fresh clone/checkout materialises text files CRLF, and any golden that is textual (hex dump, CSV of floats, trajectory hash file) is byte-different across clones — silently failing the memcmp oracle that is this project's central gate, for a reason no one will suspect. Compounds with QC-CONTROL B2 (arch-pinned binary goldens). Note auricle also lacks `.gitattributes`, but auricle does not stake a bit-exactness gate on committed artifacts. | `.gitattributes`: `* text=auto eol=lf`, plus explicit `goldens/** -text` (or `binary`) and the same for any committed ledger. |
| P-13 | MAJOR | `DEFINITION_OF_DONE.md:5,10` | **DoD clauses 1 and 3 are undefined for the first two milestones.** Clause 1 says "the **gate falsifier** is green" — but §8 registers six falsifiers and **M0 and M1 have none**; their gates are plain criteria. Clause 3 says "the null is reported … no organ ships without its null run" — at M0 the PID *is* the null, so the clause is vacuous and a session cannot tell whether it is failing to comply. These are precisely the two milestones a cold session runs first. | Clause 1 → "the gate criteria (ROADMAP exit) are green, including the named falsifier where one exists"; clause 3 → scope it to learned organs, explicitly N/A at M0/M1. |
| P-14 | MINOR | `.gitignore` (whole file, 9 lines) | Gaps vs this repo's own prescriptions: no `node_modules/` (M0 ships three.js — one `npm install` in `ui/` stages ~40k files); no `_*_wt/` though `CLAUDE.md:49` prescribes "git worktrees per agent (the booster `_*_wt` pattern)"; no `third_party/dl/` (P-01); no `out/` — which auricle's own `.gitignore` has and fusion dropped (it is the Visual Studio CMake default dir); no `.vscode/`, `compile_commands.json`, `CMakeUserPresets.json`, `Testing/`, `*.obj/*.pdb/*.exe`. Conversely `*.log` is broad enough to silently swallow a ledger someone intends to commit. |
| P-15 | MINOR | `README.md` (all) + `CLAUDE.md:3-7` | **The KICKOFF chain's only pointer is prose in the last log entry.** No document *links* to `KICKOFF_M0.md`; README never mentions KICKOFF at all; CLAUDE.md's read-order names spec/ROADMAP/SESSION_LOG-tail. The chain works today only because `SESSION_LOG.md:28` happens to say "a brand-new session opens with `KICKOFF_M0.md`". The moment a session appends a vaguer "Next", the bootstrap silently breaks. | Put "current kickoff: `KICKOFF_M<n>.md`" as a standing line in ROADMAP (or in CLAUDE.md's read-order), so the pointer is structural rather than anecdotal. |
| P-16 | MINOR | spec §10:358-371 | Layout omits **`docs/`** (exists on disk, tracked) and omits **`third_party/`** even though the same paragraph mandates "vendored third_party, pinned". Otherwise §10 matches disk exactly: `core control membrane voice trainer telemetry ui contracts goldens runs tools` all present. |
| P-17 | MINOR | `CLAUDE.md:47` + `DEFINITION_OF_DONE.md:16` | Snapshot procedure underspecified: `<ts>` format unstated (ISO? `YYYYMMDD-HHMM`?), `C:/fusion_snapshots` does not exist, and `tools/` is empty — no snapshot script exists for a step that is 1 of 6 DoD clauses and the sole durability mechanism for `runs/` (P-18). |
| P-18 | MINOR | `.gitignore:3` + `DEFINITION_OF_DONE.md:5` | **All gate evidence is untracked.** `runs/` is gitignored, so the receipts that DoD #1 makes the proof of every milestone live only on this box plus the manual snapshot. Skip step 6 once and the proof is one `git clean -xdf` from gone. (auricle sets the same precedent — `git ls-files runs` → 0 — so this is deliberate house style, not an oversight; but auricle's receipts are not the sole evidence for a falsifier-gated roadmap.) Suggested: commit the small `runs/*.md` receipt summaries, gitignore only the raw ledgers. |
| P-19 | MINOR | `SESSION_LOG.md:4-5` | Entry format is stated ("date · session id/handle · what was done · honest state · next") and S0 demonstrates it — but S0 is a *spec* session. A coding-session entry needs slots the format lacks: receipt paths, commit SHA, measured numbers, and **the null result** (DoD #3). Nothing shows a coding entry's shape, so S1 will invent one and S2 will copy it. |
| P-20 | MINOR | spec §9:334 **vs** `ROADMAP.md:10` + `KICKOFF_M0.md:24-26` | The three.js dashboard is plain scope in §9 ("Tier-0 + PID + headless MC + minimal three.js dashboard") but **stretch** in ROADMAP ("(stretch)") and KICKOFF ("*(stretch)* … skipping it does not block the gate"). CLAUDE.md declares the spec "the canon", so a canon-first session reads it as required. Gates agree in all three, so the risk is scope not gate. |
| P-21 | MINOR | spec §2.2:97 **vs** §10:372 + `ROADMAP.md:10` + `KICKOFF_M0.md:13,21` | **`fusor mc` (subcommand) vs `fusor_mc` (separate binary).** The booster constitution states rule 5 as `fusor mc --n 1000`; §10, ROADMAP and KICKOFF all specify a separate `fusor_mc` target. Neither form's CLI is specced (`--n`? `--seed`? `--scenario`?) though DoD #1 requires the receipt to state "config, seeds, numbers". |
| P-22 | MINOR | `ROADMAP.md:13-18` **vs** spec §9:337-340 | ROADMAP-M1 is a superset of §9-M1: it adds 1-D transport, the scenario set, PF coil circuits and the D-006 EKF innovation source. Not contradictory, but the canon/roadmap scope drift is unmanaged. Note also that ROADMAP places the EKF at M1 while §3:128 defines it as running "beside the policy at all times" — the policy does not exist until M2. |
| P-23 | MINOR | spec footer :404-408 **vs** `DECISIONS.md` + `SESSION_LOG.md:17` | Citation error: the footer names **six** QC catches and cites them as "Logged as D-003…D-008", but the sixth (the ghost guarantee at the commit boundary) is **D-009**, and D-004 (ASR is a seam) is not a QC catch at all — so the range is wrong at both ends. SESSION_LOG says "D-003…D-009". Enumerate rather than range. |
| P-24 | NIT | spec §8 | Falsifier suffix convention is inconsistent — `F-KEEPUP-F`/`F-PRESENCE-F` vs `F-NULL-C` vs bare `F-GHOST`/`F-VOICE`/`F-INSTINCT`. Names are used *consistently* everywhere (verified), so this is cosmetic. |
| P-25 | NIT | `KICKOFF_M0.md:29` | "SESSION_LOG **S1**" implies one session per milestone; the method (DoD #5, CLAUDE.md:43) correctly assumes N sessions per milestone. Phrase as "append your session entry". |
| P-26 | MAJOR | `DEFINITION_OF_DONE.md:3,5,10` **vs** `CLAUDE.md:29-30` + `ROADMAP.md:21-22` | **A milestone whose null wins can never be DONE — the roadmap deadlocks on an honest result.** DoD's preamble says "partial credit does not exist" and clause 1 requires "the gate falsifier is **green**". M2's gate *is* F-NULL-C = "the net **beats** the null". But `CLAUDE.md:29-30` ("No organ ships without beating its null, **published either way**") and DoD clause 3 ("beat **or honestly lost to** its baseline") both explicitly contemplate losing. So a measured loss simultaneously satisfies clause 3 and fails clause 1, and M2 can never be marked done — the roadmap has no branch for the outcome its own doctrine says is legitimate and publishable. This is not hypothetical: QC-CONTROL B4 shows the parent project's equivalent end-to-end distillation is a **recorded measured null** (booster D-041), i.e. the losing branch is the one with prior evidence. Surfaced by QC-CONTROL B16(7); the defect is in my files, not theirs. | State the losing branch explicitly: a falsifier that fires is a **completed** milestone with a published negative receipt plus a D-entry choosing the fallback rung (per B4's proposed M2 ladder) — not a stall. Reword clause 1 to "the gate falsifier has been **run and adjudicated**, receipt published either way". |

**Verified clean (explicit passes):**

- **All six falsifier names match exactly** across §8 / ROADMAP / KICKOFF — no name drift.
- **Every lineage path KICKOFF sends the cold session to exists on disk**: `C:/Booster_Lander_Simulator/core`, `.../trainer/export_weights.py`, `C:/auricle/src/core/ring.h`, `C:/auricle/CMakeLists.txt`, `C:/TinyVillage`, `C:/llama.cpp`, `C:/models`. This was the single likeliest silent killer and it is clean.
- **All three contracts files are valid TOML 1.0** — parsed with `python -c "import tomllib"`; zero syntax errors. Every flaw found is semantic (P-04/05/07), not syntactic. *(No C++ TOML parser is specced, so "which parser chokes" is unanswerable — see P-01.)*
- **Every markdown link target resolves** (5 distinct targets, 0 broken).
- **§10 repo layout matches disk** for all 11 named directories (2 omissions only, P-16).
- **Git state is clean**: one founding commit, no stray files, no uncommitted work, `.gitkeep` discipline applied consistently to all 10 empty dirs.
- **q95 floor/alarm ordering is coherent** across files: `floors.toml` 2.2 < `events.toml` alarm 2.35 < warn 2.6 — the alarm correctly precedes the floor.

---

## 2 · Cold-session simulation — what blocks M0, in the order hit

Simulating a fresh session that opens `KICKOFF_M0.md` and follows it literally.

| # | Step | What happens | Verdict |
|---|---|---|---|
| 1 | KICKOFF "read first" → `CLAUDE.md` | Hits **warpbus** (undefined, P-11) on line 3 of the charter's own vocabulary. Reads non-negotiable #1 (C/C++/CUDA only) and will collide with it at step 9. | friction |
| 2 | Begin work | No branch step in the KICKOFF checklist (P-10) → **works on `main`**, silently violating DoD #6 in session 1. | silent violation |
| 3 | Item 1 — "CMake skeleton" | **No generator, no toolchain version, no ctest invocation anywhere in the repo** (P-03). Invents `cmake -S . -B build -G ?`. Session 2 will invent differently. | divergence |
| 4 | Item 2 — `core/` burn physics | Actionable. §2.1 names real formulas (Bosch-Hale, Trubnikov, IPB98(y,2)). *But* QC-PHYSICS PHY-07: no particle balance → the plant is a 1-state ODE. | proceeds |
| 5 | Item 3 — "config in `contracts/scenarios/easy.toml`" | **Needs a TOML parser. None specced, `third_party/` does not exist, package managers forbidden** (P-01). | **HARD STOP** |
| 6 | Item 3 (cont.) | **What keys go in `easy.toml`?** No schema, no units, no example (P-02). Improvises the format every later scenario inherits. | **HARD STOP** |
| 7 | Item 4 — `fusor_mc` | Is it a binary or a `fusor mc` subcommand? §2.2 says one, §10/ROADMAP/KICKOFF say the other (P-21). CLI flags unspecced. | coin flip |
| 8 | Item 4 (cont.) | Wilson CIs over **what randomness?** Nothing in the repo says what Philox drives (QC-PHYSICS PHY-05, converges with P-02) — N=1000 runs of a deterministic ODE give a degenerate binomial. | gate is not a test |
| 9 | Item 5 — memcmp oracle | Item 5 is an in-run self-check, but **DoD #2 demands committed goldens** and "golden" is undefined (P-09). Scope says "nothing more". | contradiction |
| 10 | Item 6 — three.js dashboard | **Non-negotiable #1 forbids JS in the product** (P-08); Tauri/npm collides with "no package managers". Refuse, or learn the law is soft. | contradiction |
| 11 | Exit — snapshot | `<ts>` format undefined, `C:/fusion_snapshots` does not exist, no script in `tools/` (P-17). | improvises |
| 12 | Exit — DoD #1 | "The **gate falsifier** is green" — M0 has no falsifier in §8 (P-13). Searches for `F-BURN`, finds nothing. | confusion |
| 13 | Exit — DoD #3 | "The null is reported" — at M0 the PID *is* the null (P-13). Cannot tell if it complies. | confusion |

**Net: the session gets to step 5 (roughly item 3 of 6) before it is genuinely stuck**, and
reaches that point having already diverged on toolchain and branch. Steps 5 and 6 are one
artifact apart from being fixed: a named TOML parser + a `_TEMPLATE.toml`.

---

## 3 · Cross-file contradiction matrix

| Topic | File A says | File B says | SEV |
|---|---|---|---|
| Greenwald floor | spec §4.2:191 — "Greenwald f ≤ **1.0**" | `floors.toml:7` — `greenwald_max = **0.95**` | P-06 MAJOR |
| Nominal token budget | spec §4.1:180 + `events.toml:4` — "nominal hour ≤ **~100** tokens" | `events.toml:12,29` — 30 s ticks (**~1,200–1,400/hr**) and `max_tokens_per_min_nominal = 4` (**240/hr**) | P-07 MAJOR |
| Product languages | `CLAUDE.md:11` — "**C/C++20 + CUDA only** in the product" (exhaustive list) | spec §10:367 + §6:279 + §9-M0 + ROADMAP-M0 + KICKOFF:24 — **Tauri + three.js/WebGPU**, shipping at M0 | P-08 MAJOR |
| Package managers | `CLAUDE.md:55` + §10:372 — "**no package managers** in the product build" | §10:367 — Tauri (**cargo**) + three.js (**npm**) | P-08 MAJOR |
| Goldens at M0 | `DEFINITION_OF_DONE.md:7-9` — goldens **committed** every milestone | `KICKOFF_M0.md:12,22` — "scope (and **nothing more**)", no golden step | P-09 MAJOR |
| Milestone branch | `CLAUDE.md:48` + DoD:17 — commit on **the milestone branch** | `KICKOFF_M0.md:27-29` — exit checklist has **no branch step**; git has only `main` | P-10 MAJOR |
| MC invocation | spec §2.2:97 — `**fusor mc** --n 1000` (subcommand) | §10:372 + ROADMAP:9 + KICKOFF:13,21 — `**fusor_mc**` (separate target) | P-21 MINOR |
| Dashboard status | spec §9:334 — plain **scope** | ROADMAP:10 + KICKOFF:24 — "**(stretch)**, skipping does not block the gate" | P-20 MINOR |
| M1 contents | spec §9:337 — GS + vertical + diagnostics + coil circuits | ROADMAP:13-16 — **also** 1-D transport, scenario set, EKF/D-006 | P-22 MINOR |
| QC-catch D-range | spec footer:408 — "Logged as **D-003…D-008**" | `SESSION_LOG.md:17` — "**D-003…D-009**"; the 6th catch is D-009, and D-004 is not a catch | P-23 MINOR |
| `third_party/` | `CLAUDE.md:55` + §10:372 — vendored + pinned, mandatory | disk + §10 layout block — **does not exist, not listed** | P-01 BLOCKER |
| Glossary coverage | `CLAUDE.md:59` — "mandatory terms — drift fails review" | README/ROADMAP/DECISIONS/spec use **warpbus, molt, L-SILENCE, rev, trunk, rung** — none in the glossary | P-11 MAJOR |

---

## 4 · Cross-pollination log

**Received → confirmed/refuted with evidence from my slice:**

- **QC-PHYSICS PHY-05** (`fusor_mc`/Wilson CIs undefined — nothing is random): **CONFIRMED
  independently from the process side.** It merges with my P-02: the missing scenario schema
  is exactly where the stochastic model must be pre-registered. One artifact fixes both.
  Combined with PHY-07 (no particle balance → "a 1-state ODE a P-controller holds perfectly"),
  **two lanes now independently conclude the M0 gate as written is not a discriminating test.**
- **QC-MEMBRANE** on the budget law: converged with my P-07 from opposite directions (they
  from §4.1, me from the TOML). They rate BLOCKER-for-M3; I keep MAJOR on the cold-M0 axis.
  Same defect, and the independent derivation raises confidence.
- **QC-PHYSICS ruling on Greenwald** (table 0.95 wins, prose is stale): accepted; my P-06
  records the direction of the fix as "prose must cite the contract, not restate a number".
- **QC-CONTROL B2** (fp16/int8 policy vs bit-determinism): confirmed the *documentary* half —
  the bit-claim is asserted in four places I own (`CLAUDE.md:16-17`, `DoD:7-9`, D-002, §8
  F-GHOST). It also **strengthens my P-12**: arch-pinned goldens in a repo with
  `core.autocrlf=true` and no `.gitattributes` is a second, independent corruption path.
- **QC-CONTROL B5** (two fork semantics) and **B4** (LODESTAR's distill branch is the parent's
  *measured null*): both require **new D-entries superseding D-002/D-009**, not spec edits —
  DECISIONS.md is append-only. B4 additionally rewrites ROADMAP-M2 and §8's F-NULL-C text.

**Broadcast (my lane's contribution — the ripple map):** posted the same-commit file list for
every peer fix, most consequentially that **QC-PHYSICS PHY-02** (γ⁻¹ = 10–100 ms, not 1–5 ms)
lands on **§6:284 — "controller OFF → watch the VDE take it in ~50 ms"**. At the corrected
growth rate 50 ms is roughly one e-folding: the money shot of the 90-second M5 video shows a
plasma barely moving. That line sits in no auditor's assigned slice and must be re-timed
together with §2.1, §0, D-003, §9-M5 and ROADMAP-M5.

**Asked of all four lanes:** is your section's gate *testable as written* by a session holding
only this repo? Named concerns: F-NULL-C ("matched compute" undefined), F-VOICE ("zero
un-receipted audio" — measured how?), **F-PRESENCE-F** (blind-grade by whom? no rubric, no
grader, no held-out set exists in the repo), **F-INSTINCT** ("this domain's held-out set" does
not exist and nothing in ROADMAP schedules its creation).

**Answers received:**

- **QC-CONTROL B16 — F-NULL-C is not testable as written**, with seven holes (matched compute
  undefined; no pre-registered scenario set or N; **no train/held-out seed partition** — the
  parent enforces this *in code*, fusion has no such law; no common-random-numbers rule;
  **Wilson is a binomial interval and cannot apply to "tracking RMS"**, yet §8 applies it to
  both; no decision rule for a split outcome; and no branch if the null wins). The last one is
  a defect in *my* files, now logged as **P-26** — DoD's "partial credit does not exist" +
  "the gate falsifier is green" deadlock the roadmap against an outcome CLAUDE.md:29-30 and
  DoD clause 3 both explicitly permit.
- **QC-PHYSICS PHY-17 — my §6:284 ripple flag CONFIRMED and quantified**: mm-seed to ~20 cm
  wall contact is ~5–6 e-foldings, so the spec's own γ⁻¹ = 1–5 ms yields a 5–30 ms VDE and the
  quoted "~50 ms" implies γ⁻¹ ≈ 8–10 ms — the demo line is inconsistent with §2.1 *even before*
  PHY-02's correction. Under the corrected rate the honest figure is 100–300 ms. The line
  belonged to no lane; it is now owned.
- **QC-VOICE-RT** confirmed PHY-04 from the latency side and reports **Chain C is missing from
  §7 entirely** (operator's last word → first phoneme ≈ 2.5 s typical / 4+ s p95) — i.e. the M4
  gate's own headline number is unbudgeted. Adding a §7 row edits a table cross-referenced by
  DoD's closing clause ("Latency/VRAM targets (spec §7) bind at their named milestones"), so it
  is a same-commit edit to §7 + ROADMAP-M4 + a D-entry, not a §5 edit.

---

## 5 · Verdict

**Is the scaffold multi-session-ready? — Not yet. Two artifacts and one paragraph away.**

The *architecture* of the process is genuinely good and is the best part of this repo:
SESSION_LOG-as-state-of-truth, same-commit doc discipline, append-only decisions, a KICKOFF
per milestone written by the session that finishes the previous one, and a DoD that demands
receipts and nulls. That skeleton is sound, is inherited from projects where it demonstrably
worked, and I found no structural flaw in it. What is missing is **executable specificity**:
the method describes *what* must happen and almost never *how to type it*. Nine of my
twenty-five findings are of one shape — a discipline is declared in prose with no mechanism,
no name, and no example (branch names, golden format, snapshot timestamp, scenario schema,
build command, coding-session log entry, TOML parser, `<ts>`, the grader for F-PRESENCE-F).
A single motivated session will paper over each of these — differently each time. The one
*structural* defect in the method itself is P-26: the DoD has no branch for a milestone whose
falsifier honestly fires, which is the outcome the parent project already measured.

**Top 3 risks**

1. **The M0 session stops at KICKOFF item 3 and improvises the two things that outlive it.**
   No TOML parser + no scenario schema (P-01, P-02) blocks the session at the exact point
   where its improvisation becomes permanent: the config format every later scenario, golden
   and receipt inherits. Highest-value fix in the repo — one vendored header plus one
   `_TEMPLATE.toml` clears both blockers and PHY-05's stochastic-model gap at the same time.
2. **The non-negotiables get broken on day one, by the roadmap itself.** M0 ships a three.js
   dashboard that the charter's exhaustive language rule forbids (P-08). The damage is not the
   dashboard — it is teaching session 1 that a list headed "non-negotiable" is negotiable, in a
   repo whose entire multi-session strategy rests on doctrine surviving sessions that never
   meet each other. One carve-out sentence removes it.
3. **The evidence base is structurally fragile.** Gate proof lives in gitignored `runs/`
   (P-18), preserved only by a manual snapshot with an undefined path format and no script
   (P-17), while the goldens that anchor the determinism gate are undefined (P-09), created by
   no M0 step, and exposed to CRLF rewriting under a measured `core.autocrlf=true` with zero
   `.gitattributes` (P-12). For a project whose first non-negotiable principle is "measured,
   not claimed", the measurements are the least durable artifacts in the repo.

**Counts:** 3 BLOCKER · 11 MAJOR · 10 MINOR · 2 NIT · 6 verified passes (26 findings).
