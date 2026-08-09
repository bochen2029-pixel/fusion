# fusion — roadmap

The next phase to build is the **first unchecked milestone**. A milestone is DONE only per
[DEFINITION_OF_DONE.md](DEFINITION_OF_DONE.md) — gate green with receipts, nulls reported,
docs synced in the same commit. Spec: [FUSION_ARCHITECTURE_v0.2.md](FUSION_ARCHITECTURE_v0.2.md) §9.
Decisions live in [DECISIONS.md](DECISIONS.md) (append-only).
**Current kickoff: [`KICKOFF_M1.md`](KICKOFF_M1.md)** — the finishing session updates
this line (P-15's structural pointer).

- [x] **V0.2 — the QC-swarm revision (MANDATORY BEFORE M0; bootstrap: `KICKOFF_SPEC_V02.md`;
      scoped 2–4 sessions, machine.toml first).** Disposition all 39 blockers + 60 majors
      from the five-auditor pass (`docs/qc/QC_CONSOLIDATED.md` + lane reports) **and the
      fresh-eyes second-instrument findings (`docs/qc/FRESH_EYES.md` — D-018)**. Core
      design deliverables:
      `contracts/machine.toml` (pin **synthetic FUSOR-1**; derive γ AND Q>1 capability
      from that one config) · the objective/reward function (one file, every consumer) ·
      the disruption/quench model + its event token · the writ grammar v1
      (id/supersedes/ttl/cancel/formed_tick **+ the session input-tape**) · the scenario
      schema `_TEMPLATE.toml` + stochastic model (what is random, for Wilson CIs) **+ the
      floor-legal-but-lethal class** · F-INSTINCT's eval-set home + F-PRESENCE-F grader
      protocol (TinyVillage L12) **+ its third arm (event-triggered twin) + F-VERACITY**
      (D-018) · the statecheck fence · GPU-arbitration policy + tick-jitter gate + CPU
      budget · crisis scheduling (probe batching, molt deferral) · nulls for the two new
      organs (tokenizer, ghost — scored on the lethal-legal class) · **the eight orphaned
      dispositions (M2 ladder first) + the EKF D-entry** · scope rulings implemented
      (D-020; tier-2 = operator's call) · ripple sweep + citation hygiene · KICKOFF_M0
      rewritten (TOML parser + third_party plan + build block). **Exit:** spec bumped to
      `FUSION_ARCHITECTURE_v0.2.md` (v0.1 kept beside), every blocker — five lanes AND
      FRESH_EYES — traceably dispositioned, REPO's cold-session simulation re-run clean.
- [x] **M0 — the burn.** Tier-0 0-D burn physics (Bosch-Hale, alpha heating, Bremsstrahlung
      + synchrotron + line radiation + Ohmic + particle/ash balance, implicit IPB98(y,2),
      live Q) + CEM-tuned PID null + `fusor_mc` headless Monte-Carlo (Wilson CIs) +
      memcmp replay oracle + committed goldens.
      **DONE 2026-08-09 — F-BURN-0 GREEN, attempt 1: pass 985/1000 = 0.985, Wilson95
      [0.975, 0.991] (required ≥0.90/LB 0.88); 0 disruptions, 0 spine shutdowns; ctest
      5/5. Receipt: `runs/m0-burn-2026-08-09.md`.**
- [x] **M0.5 — the thesis slice (shadow; D-019).** `fusor_shadow`: plant event tap →
      tokenizer → hash-chained tape → the Qwen3.5-9B triple (fusion seats, structural
      plant-lane boundaries, fork-stub Sentinel, caps + budget drops) → **three arms +
      tokenizer null + F-VERACITY code grader**, dry-run on a composed 45-min session
      (one survival, two deaths on tape).
      **DONE 2026-08-09 — no gate claims, first data everywhere: event-twin parity at
      3.1× cost (D-018 measured); poll twin absent 5/6 answers; dial-zero would-fire
      85% (guards contained to 36); Skeptic caught the planted falsehood citing the
      record; Sentinel's gated would-fire on the floor-legal writ = FRESH-3's datum;
      the tokenizer null confabulates where events give citation. D-034; receipt
      `runs/m05-thesis-slice-2026-08-09.md`. M3 rescopes onto this rig.**
- [ ] **M1 — the shape and the enemy.** Tier-1: free-boundary Grad-Shafranov (100–200 Hz)
      + 10 kHz linearized vertical model (D-003) + PF coil circuits + synthetic diagnostics
      + EKF innovation source (D-006) + 1-D transport + scenario set. MPPI oracle (CEM)
      proves every curriculum scenario solvable.
      **Exit:** open-loop run demonstrably disrupts (VDE); oracle lands flat-top through
      the full disturbance set; determinism suite green at tier-1.
- [ ] **M2 — the cerebellum.** Solver-then-distill → PPO polish on batched CUDA envs
      (2048+); flat-weight export → in-loop inference < 50 µs.
      **Exit:** **F-NULL-C** — the net beats the tuned PID+LQ null (the MPC is a later,
      separately-receipted comparison — D-020) on disruption rate + tracking RMS at
      matched compute, Wilson CI, published either way.
- [ ] **M3 — the membrane (shadow).** Warpbus + the tokenizer of machine experience
      (`contracts/events.toml`) + the triple watching in log-only soak mode (auricle M4.5
      pattern: ledger, margins, surprisal, briefs) + ghost-forks live.
      **Exit:** **F-GHOST** (fork-vs-reality memcmp + warning-precedes-commit on tape) +
      shadow receipts + fire-rate report.
- [ ] **M4 — the voice.** ASR in (Nemotron 3.5 streaming primary / sherpa fallback behind
      `asr.h`), Piper TTS out, full duplex, VAD barge-in ≤ 150 ms (D-005), voiced-vs-drafted
      ledger. **Solicited speech only** (D-007).
      **Exit:** **F-VOICE** measured end-to-end.
- [ ] **M5 — the talking reactor.** Writs at H0/H1 through floors.toml, etiquette caps,
      unsolicited initiative iff F-INSTINCT passed (else stays shadow), the toggle demo,
      **F-KEEPUP-F + F-PRESENCE-F** scripted soak, and the 90-second video.
      **Exit:** the video + the published ledger, wins and losses both.

**Parked (post-M5):** UE5 client on the same telemetry; tier-2 MHD (cut for v1 — D-031;
the renderer ships tier-1-derived emissivity + artistic noise; pure-renderer revisit
only); MCTS scenario planning + sysid-by-backprop (D-020, named nulls if revived); the
emit-QLoRA for this domain; multi-reactor rooms.
