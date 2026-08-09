# fusion — definition of done (per milestone)

A milestone is DONE when **all** of the following hold — partial credit does not exist:

1. **The gate is green** — the milestone's ROADMAP exit criteria including its named
   falsifier where one exists (every gated milestone names one from v0.2 on; M0.5's
   shadow rig claims no gate by design — P-13) — with a dated receipt in `runs/` stating
   config, seeds, numbers, CIs where applicable, and the exact reproduce command block.
2. **The determinism suite is green** — memcmp replay oracle bit-identical on the
   milestone's goldens; goldens updated and committed in `goldens/`.
3. **The null is reported** — whatever organ shipped beat (or honestly lost to) its
   baseline, published either way in the receipt. No organ ships without its null run.
   **Scope (P-13): organs that HAVE a declared null (learned controllers, the tokenizer,
   the ghost, the triple). Where the organ IS the null (M0's PID), say so in the receipt
   — the clause is then satisfied, not violated.**
   **If the null wins, the milestone is still DONE** when the loss is receipted and the
   null is *adopted as the shipping organ* (the booster's own precedent: a measured null
   is a result, not a failure — QC finding P-26 closed this deadlock).
4. **Docs synced in the same commit as the work:** ROADMAP checkbox flipped,
   SESSION_LOG entry appended, DECISIONS.md entries added for anything that changed a
   pre-registered table or design call.
5. **The next KICKOFF exists** — the finishing session writes `KICKOFF_M<n+1>.md` before
   it ends (the handoff is part of the milestone).
6. **Snapshot taken** to `C:/fusion_snapshots/<yyyyMMdd-HHmmss>` via `tools/snapshot.ps1`
   (P-17: the format and the script are defined; captures gitignored `runs/`), and the
   milestone committed on its branch (`m<N>-<slug>`; `main` merges at gates).

Latency/VRAM targets (spec §7) bind at their named milestones; a miss is not a silent
re-spec — it is a receipt plus a D-entry adjusting the target with the measured reason.
