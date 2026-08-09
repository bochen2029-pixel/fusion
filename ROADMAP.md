# fusion — roadmap

The next phase to build is the **first unchecked milestone**. A milestone is DONE only per
[DEFINITION_OF_DONE.md](DEFINITION_OF_DONE.md) — gate falsifier green with receipts, nulls
reported, docs synced in the same commit. Spec: [FUSION_ARCHITECTURE_v0.1.md](FUSION_ARCHITECTURE_v0.1.md) §9.
Decisions live in [DECISIONS.md](DECISIONS.md) (append-only).

- [ ] **V0.2 — the QC-swarm revision (MANDATORY BEFORE M0; bootstrap: `KICKOFF_SPEC_V02.md`).**
      Disposition all 39 blockers + 60 majors from the five-auditor pass
      (`docs/qc/QC_CONSOLIDATED.md` + lane reports). Core design deliverables:
      `contracts/machine.toml` (pin the machine; resolve the γ-vs-Q two-machine
      contradiction) · the reward/cost function · the disruption/quench model + its event
      token · the writ grammar v1 (id/supersedes/ttl/cancel/formed_tick) · the scenario
      schema `_TEMPLATE.toml` + stochastic model (what is random, for Wilson CIs) ·
      F-INSTINCT's eval-set home + F-PRESENCE-F grader protocol (TinyVillage L12) ·
      the statecheck fence · GPU-arbitration policy + tick-jitter gate · crisis
      scheduling (probe batching, molt deferral) · nulls for the two new organs
      (tokenizer, ghost) · KICKOFF_M0 rewritten (TOML parser + third_party plan + build
      block). **Exit:** spec bumped to `FUSION_ARCHITECTURE_v0.2.md` (v0.1 kept beside),
      every blocker traceably dispositioned, REPO's cold-session simulation re-run clean.
- [ ] **M0 — the burn.** Tier-0 0-D burn physics (Bosch-Hale, alpha heating, Bremsstrahlung
      + synchrotron, IPB98(y,2) τ_E, live Q) + PID null + `fusor_mc` headless Monte-Carlo
      (Wilson CIs) + memcmp replay oracle + minimal three.js dashboard (stretch).
      **Exit:** replay bit-identical across two runs (memcmp green); PID sustains Q > 1 on
      the easy scenario; MC prints rates + CIs. Receipts: `runs/m0-*`.
- [ ] **M1 — the shape and the enemy.** Tier-1: free-boundary Grad-Shafranov (100–200 Hz)
      + 10 kHz linearized vertical model (D-003) + PF coil circuits + synthetic diagnostics
      + EKF innovation source (D-006) + 1-D transport + scenario set. MPPI oracle (CEM)
      proves every curriculum scenario solvable.
      **Exit:** open-loop run demonstrably disrupts (VDE); oracle lands flat-top through
      the full disturbance set; determinism suite green at tier-1.
- [ ] **M2 — the cerebellum.** Solver-then-distill → PPO polish on batched CUDA envs
      (2048+); flat-weight export → in-loop inference < 50 µs.
      **Exit:** **F-NULL-C** — the net beats tuned PID/MPC on disruption rate + tracking
      RMS at matched compute, Wilson CI, published either way.
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

**Parked (post-M5):** UE5 client on the same telemetry; tier-2 MHD eye-candy (can land
earlier as a pure-renderer stretch); the emit-QLoRA for this domain; multi-reactor rooms.
