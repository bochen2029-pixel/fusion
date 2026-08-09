#!/usr/bin/env python3
"""trainer/tune_pid.py — CEM tuning of the M0 PID null (dev rig; never in the product).

Seed law (contracts/objective.toml [seeds], enforced HERE): the trainer refuses any seed
>= 800000 (eval domain 800000..899999, gate domain 900000..999999). seedcheck (ctest)
runs --selftest-seed-guard.

Usage:
  python C:/fusion/trainer/tune_pid.py --selftest-seed-guard
  python C:/fusion/trainer/tune_pid.py --exe C:/fusion/build/Release/fusor_mc.exe \
         --root C:/fusion [--iters 15] [--pop 24] [--nseeds 48]
Writes control/gains_m0.toml + prints the tuning receipt block.
(Forward slashes in all paths — Git Bash eats backslashes.)
"""
import argparse, json, math, random, subprocess, sys, tempfile, os

TRAIN_MAX = 800000  # first refused seed (eval+gate domains)

def seed_allowed(seed: int) -> bool:
    return 0 <= seed < TRAIN_MAX

def selftest() -> int:
    ok = (not seed_allowed(900042)) and (not seed_allowed(850000)) and seed_allowed(12345)
    print("seed-guard selftest:", "GREEN" if ok else "RED")
    return 0 if ok else 1

PNAMES = ["Kp", "Ki", "Kd", "Kpn", "Kin", "P_ff_MW", "S_ff_e20", "Krad"]
LO = [0.5, 0.0, 0.0, 0.2, 0.0, 20.0, 1.0, 0.0]
HI = [40.0, 20.0, 10.0, 8.0, 5.0, 56.0, 5.0, 1.6]

def write_gains(path, p):
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write("[pid]\n")
        for n, v in zip(PNAMES, p):
            f.write(f"{n} = {v:.6f}\n")

def evaluate(exe, root, gains_path, seed_lo, n):
    if not seed_allowed(seed_lo) or not seed_allowed(seed_lo + n - 1):
        raise SystemExit(f"REFUSED: seeds [{seed_lo},{seed_lo+n}) touch the eval/gate domain")
    out = subprocess.run(
        [exe, "--root", root, "--scenario", f"{root}/contracts/scenarios/easy.toml",
         "--gains", gains_path, "--seeds", f"{seed_lo}:{seed_lo+n}", "--json"],
        capture_output=True, text=True, check=True)
    return json.loads(out.stdout.strip().splitlines()[-1])

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--selftest-seed-guard", action="store_true")
    ap.add_argument("--exe", default="C:/fusion/build/Release/fusor_mc.exe")
    ap.add_argument("--root", default="C:/fusion")
    ap.add_argument("--iters", type=int, default=15)
    ap.add_argument("--pop", type=int, default=24)
    ap.add_argument("--elites", type=int, default=6)
    ap.add_argument("--nseeds", type=int, default=48)
    ap.add_argument("--rngseed", type=int, default=7)
    ap.add_argument("--finalists", type=int, default=8)
    ap.add_argument("--playoff-n", type=int, default=400)
    ap.add_argument("--playoff-base", type=int, default=600000)
    a = ap.parse_args()
    if a.selftest_seed_guard:
        sys.exit(selftest())

    rng = random.Random(a.rngseed)
    mean = [6.0, 3.0, 1.5, 2.0, 1.0, 40.0, 2.5, 0.9]
    sig = [4.0, 2.5, 1.5, 1.5, 1.0, 5.0, 0.8, 0.4]
    tmp = tempfile.mkdtemp(prefix="fusion_cem_")
    hall = []                                   # all-time candidates for the playoff
    print(f"CEM: iters={a.iters} pop={a.pop} elites={a.elites} seeds/candidate={a.nseeds}")
    for it in range(a.iters):
        base = (it * a.nseeds * 7) % (TRAIN_MAX - a.nseeds)   # rotate train seeds
        scored = []
        for c in range(a.pop):
            p = [min(HI[i], max(LO[i], rng.gauss(mean[i], sig[i]))) for i in range(8)]
            gp = os.path.join(tmp, f"g{it}_{c}.toml").replace("\\", "/")
            write_gains(gp, p)
            j = evaluate(a.exe, a.root, gp, base, a.nseeds)
            scored.append((j["cost_mean"], p, j))
        scored.sort(key=lambda x: x[0])
        elites = scored[: a.elites]
        mean = [sum(e[1][i] for e in elites) / len(elites) for i in range(8)]
        sig = [max(0.02, math.sqrt(sum((e[1][i] - mean[i]) ** 2 for e in elites) / len(elites)) * 0.9 + 0.05)
               for i in range(8)]
        hall.extend((s[0], s[1]) for s in scored[:2])
        jb = scored[0][2]
        print(f"  it {it:2d}: best_cost {scored[0][0]:9.3f}  pass {jb['pass_frac']:.3f} "
              f"(spine {jb['spine']}, disrupt {jb['disrupt']})  seeds {base}:{base+a.nseeds}")
    # ---- PLAYOFF (D-032h lesson: 48-seed batch scores are ±10% noisy — batch-luck
    # selection cost the first run ~11 points of true pass rate). Re-score the all-time
    # top candidates on one large fresh train batch and pick by TRUE pass rate.
    hall.sort(key=lambda x: x[0])
    finalists = [h[1] for h in hall[: a.finalists]]
    print(f"PLAYOFF: {len(finalists)} finalists x {a.playoff_n} seeds @ {a.playoff_base}")
    best, best_key = None, None
    for i, p in enumerate(finalists):
        gp = os.path.join(tmp, f"final_{i}.toml").replace("\\", "/")
        write_gains(gp, p)
        j = evaluate(a.exe, a.root, gp, a.playoff_base, a.playoff_n)
        key = (-j["pass_frac"], j["cost_mean"])
        print(f"  finalist {i}: pass {j['pass_frac']:.4f}  wilson_lo {j['wilson_lo']:.4f}  "
              f"cost {j['cost_mean']:.2f}")
        if best_key is None or key < best_key:
            best_key, best, best_cost = key, list(p), j["cost_mean"]
    out = f"{a.root}/control/gains_m0.toml"
    with open(out, "w", encoding="utf-8", newline="\n") as f:
        f.write("# control/gains_m0.toml -- CEM-TUNED (trainer/tune_pid.py; receipt in runs/).\n")
        f.write(f"# budget: iters={a.iters} pop={a.pop} elites={a.elites} nseeds={a.nseeds} "
                f"rngseed={a.rngseed}; train-domain seeds only (< {TRAIN_MAX}).\n[pid]\n")
        for n, v in zip(PNAMES, best):
            f.write(f"{n} = {v:.6f}\n")
    print(f"WROTE {out}\nfinal: cost {best_cost:.3f}  gains " +
          " ".join(f"{n}={v:.3f}" for n, v in zip(PNAMES, best)))

if __name__ == "__main__":
    main()
