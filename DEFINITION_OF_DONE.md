# fusion — definition of done (per milestone)

A milestone is DONE when **all** of the following hold — partial credit does not exist:

1. **The gate falsifier is green** (ROADMAP exit criteria) with a dated receipt in
   `runs/` stating config, seeds, numbers, and CIs where applicable.
2. **The determinism suite is green** — memcmp replay oracle bit-identical on the
   milestone's goldens; goldens updated and committed in `goldens/`.
3. **The null is reported** — whatever organ shipped beat (or honestly lost to) its
   baseline, published either way in the receipt. No organ ships without its null run.
4. **Docs synced in the same commit as the work:** ROADMAP checkbox flipped,
   SESSION_LOG entry appended, DECISIONS.md entries added for anything that changed a
   pre-registered table or design call.
5. **The next KICKOFF exists** — the finishing session writes `KICKOFF_M<n+1>.md` before
   it ends (the handoff is part of the milestone).
6. **Snapshot taken** to `C:/fusion_snapshots/<ts>` (captures gitignored `runs/`), and the
   milestone committed on its branch.

Latency/VRAM targets (spec §7) bind at their named milestones; a miss is not a silent
re-spec — it is a receipt plus a D-entry adjusting the target with the measured reason.
