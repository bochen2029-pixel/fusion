#!/usr/bin/env python3
"""trainer/tune_lq.py — CEM polish of the LQ design scales (D-045; dev rig).

The search space is TWO log-scales (th1 on Q_z, th2 on Q_v) — the design itself is
objective-owned (lq_design.py); CEM polishes only scales (D-045's constraint). Cost:
fusor_mc cost_mean summed over vde_kick + rampdown on TRAIN seeds (guard identical to
tune_pid.py; seedcheck covers the law). Playoff protocol mandatory (D-032h/D-033:
small-batch scores are +-10% noisy). Candidates evaluate in PARALLEL subprocesses
(each fusor_mc run is deterministic; CEM's rng is seeded — the tuning is replayable).

Usage:
  python C:/fusion/trainer/tune_lq.py --exe C:/fusion/build/Release/fusor_mc.exe \
         --root C:/fusion --dump C:/fusion/runs/m2s1/lqdump.tsv \
         [--iters 8] [--pop 16] [--elites 4] [--nseeds 40] [--workers 10]
Writes control/gains_m2.toml (via lq_design) + prints the tuning receipt block.
(Forward slashes in all paths — Git Bash eats backslashes.)
"""
import argparse, concurrent.futures, json, math, os, random, subprocess, sys, tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import lq_design

TRAIN_MAX = 800000  # first refused seed (eval+gate domains)

def seed_allowed(seed: int) -> bool:
    return 0 <= seed < TRAIN_MAX

def evaluate(exe, root, gains_path, scen, seed_lo, n):
    if not seed_allowed(seed_lo) or not seed_allowed(seed_lo + n - 1):
        raise SystemExit(f"REFUSED: seeds [{seed_lo},{seed_lo+n}) touch the eval/gate domain")
    out = subprocess.run(
        [exe, "--root", root, "--scenario", f"{root}/contracts/scenarios/{scen}.toml",
         "--gains", gains_path, "--seeds", f"{seed_lo}:{seed_lo+n}", "--json"],
        capture_output=True, text=True, check=True)
    return json.loads(out.stdout.strip().splitlines()[-1])

SCENS = ["vde_kick", "rampdown"]

def score(exe, root, dump, th, gains_path, seed_lo, n):
    lq_design.design_and_write(dump, root, gains_path, th[0], th[1])
    cost = 0.0; det = []
    for sc in SCENS:
        j = evaluate(exe, root, gains_path, sc, seed_lo, n)
        cost += j["cost_mean"]; det.append(j)
    return cost, det

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--exe", default="C:/fusion/build/Release/fusor_mc.exe")
    ap.add_argument("--root", default="C:/fusion")
    ap.add_argument("--dump", default="C:/fusion/runs/m2s1/lqdump.tsv")
    ap.add_argument("--iters", type=int, default=8)
    ap.add_argument("--pop", type=int, default=16)
    ap.add_argument("--elites", type=int, default=4)
    ap.add_argument("--nseeds", type=int, default=40)
    ap.add_argument("--rngseed", type=int, default=7)
    ap.add_argument("--finalists", type=int, default=6)
    ap.add_argument("--playoff-n", type=int, default=200)
    ap.add_argument("--playoff-base", type=int, default=600000)
    ap.add_argument("--workers", type=int, default=10)
    a = ap.parse_args()

    rng = random.Random(a.rngseed)
    # Search box recentered PRE-CEM on the measured stability basin (receipted in
    # runs/m2s1: th=(0,0) and (-2,-2) die 10-20/20 from the 3 mm initial jitter —
    # the LQG-margins cliff; (-4,-4) holds 10/10 at 2.7x less VS effort than the PD;
    # (-3,-3) is marginal, z_rms rising). Design-stage bracketing, not post-hoc.
    mean, sig = [-3.8, -3.8], [0.5, 0.5]
    LO, HI = -4.8, -2.5
    tmp = tempfile.mkdtemp(prefix="fusion_lq_")
    hall = []
    print(f"CEM(lq): iters={a.iters} pop={a.pop} elites={a.elites} "
          f"seeds/cand={a.nseeds} scens={'+'.join(SCENS)}")
    for it in range(a.iters):
        base = (it * a.nseeds * 13) % (TRAIN_MAX - a.nseeds)   # rotate train seeds
        cands = [tuple(min(HI, max(LO, rng.gauss(mean[i], sig[i]))) for i in range(2))
                 for _ in range(a.pop)]
        scored = []
        with concurrent.futures.ThreadPoolExecutor(max_workers=a.workers) as ex:
            futs = {ex.submit(score, a.exe, a.root, a.dump, th,
                              os.path.join(tmp, f"g{it}_{ci}.toml").replace("\\", "/"),
                              base, a.nseeds): th for ci, th in enumerate(cands)}
            for f in concurrent.futures.as_completed(futs):
                c, det = f.result()
                scored.append((c, futs[f], det))
        scored.sort(key=lambda x: (x[0], x[1]))
        elites = scored[: a.elites]
        mean = [sum(e[1][i] for e in elites) / len(elites) for i in range(2)]
        sig = [max(0.05, math.sqrt(sum((e[1][i] - mean[i]) ** 2 for e in elites)
                                   / len(elites)) * 0.9 + 0.02) for i in range(2)]
        hall.extend((s[0], s[1]) for s in scored[:2])
        jb = scored[0][2]
        print(f"  it {it:2d}: best {scored[0][0]:9.3f} th=({scored[0][1][0]:+.3f},"
              f"{scored[0][1][1]:+.3f})  kick[G {jb[0]['good']}/{jb[0]['n']} zrms "
              f"{jb[0]['z_rms_mean']:.5f}]  ramp[G {jb[1]['good']}/{jb[1]['n']} zrms "
              f"{jb[1]['z_rms_mean']:.5f}]  seeds {base}:{base+a.nseeds}")
    # ---- PLAYOFF (D-032h/D-033): re-score all-time finalists on one large fresh
    # train batch; pick by TRUE cost. Dedup near-identical thetas first.
    hall.sort(key=lambda x: (x[0], x[1]))
    finalists, seen = [], set()
    for _, th in hall:
        key = (round(th[0], 2), round(th[1], 2))
        if key not in seen:
            seen.add(key); finalists.append(th)
        if len(finalists) >= a.finalists:
            break
    print(f"PLAYOFF: {len(finalists)} finalists x {a.playoff_n} seeds "
          f"@ {a.playoff_base} on {'+'.join(SCENS)}")
    results = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=a.workers) as ex:
        futs = {ex.submit(score, a.exe, a.root, a.dump, th,
                          os.path.join(tmp, f"final_{fi}.toml").replace("\\", "/"),
                          a.playoff_base, a.playoff_n): th
                for fi, th in enumerate(finalists)}
        for f in concurrent.futures.as_completed(futs):
            c, det = f.result()
            results.append((c, futs[f], det))
    results.sort(key=lambda x: (x[0], x[1]))
    for c, th, det in results:
        print(f"  th=({th[0]:+.3f},{th[1]:+.3f}): cost {c:9.3f}  "
              f"kick[G {det[0]['good']}/{det[0]['n']} zrms {det[0]['z_rms_mean']:.5f} "
              f"zpk {det[0]['z_peak_max']:.4f}]  ramp[G {det[1]['good']}/{det[1]['n']} "
              f"zrms {det[1]['z_rms_mean']:.5f}]")
    best_c, best_th, best_det = results[0]
    out = f"{a.root}/control/gains_m2.toml"
    lq_design.design_and_write(a.dump, a.root, out, best_th[0], best_th[1], verbose=True)
    print(f"WROTE {out}\nfinal: th1={best_th[0]:+.4f} th2={best_th[1]:+.4f} "
          f"playoff_cost {best_c:.3f} "
          f"(budget: iters={a.iters} pop={a.pop} elites={a.elites} nseeds={a.nseeds} "
          f"rngseed={a.rngseed} playoff {len(finalists)}x{a.playoff_n})")

if __name__ == "__main__":
    main()
