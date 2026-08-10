#!/usr/bin/env python3
"""trainer/spectrum.py — trace spectral forensics (dev rig; the D-045 pumping check).

Reads a test_vertical trace TSV (t, z, v, xz, xv, nu, S, Ivs, xI, Vcmd), computes the
DFT magnitude of a chosen column over a time window, prints the band table + peaks.
The instrument behind "trace + spectrum first, Q-inflation never" (S12/S13's law).
Pure stdlib. Usage:
  python C:/fusion/trainer/spectrum.py <trace.tsv> [--col z] [--t0 5.5] [--t1 8.0] \
         [--f0 20] [--f1 400] [--df 2]
"""
import argparse, math, sys

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("trace")
    ap.add_argument("--col", default="z")
    ap.add_argument("--t0", type=float, default=5.5)
    ap.add_argument("--t1", type=float, default=8.0)
    ap.add_argument("--f0", type=float, default=20.0)
    ap.add_argument("--f1", type=float, default=400.0)
    ap.add_argument("--df", type=float, default=2.0)
    a = ap.parse_args()
    rows = []
    with open(a.trace, encoding="utf-8") as f:
        hdr = f.readline().split()
        ci = hdr.index(a.col); ti = hdr.index("t")
        for line in f:
            v = line.split()
            if len(v) != len(hdr):
                continue
            t = float(v[ti])
            if a.t0 <= t <= a.t1:
                rows.append((t, float(v[ci])))
    if len(rows) < 32:
        sys.exit(f"spectrum: only {len(rows)} samples in [{a.t0},{a.t1}]")
    ts = [r[0] for r in rows]; xs = [r[1] for r in rows]
    dt = (ts[-1] - ts[0]) / (len(ts) - 1)
    mu = sum(xs) / len(xs)
    xs = [x - mu for x in xs]
    rms = math.sqrt(sum(x * x for x in xs) / len(xs))
    print(f"# {a.trace} col={a.col} window=[{a.t0},{a.t1}] n={len(xs)} "
          f"fs={1.0/dt:.0f} Hz  mean={mu:.6g}  rms={rms:.6g}")
    # single-frequency DFT per band point (windowless Goertzel-style; fine for peaks)
    table = []
    f = a.f0
    while f <= a.f1 + 1e-9:
        wr = wi = 0.0
        for i, x in enumerate(xs):
            ph = 2.0 * math.pi * f * i * dt
            wr += x * math.cos(ph); wi -= x * math.sin(ph)
        amp = 2.0 * math.hypot(wr, wi) / len(xs)
        table.append((f, amp))
        f += a.df
    amax = max(t[1] for t in table) or 1e-300
    for f, amp in table:
        bar = "#" * int(50 * amp / amax)
        print(f"{f:7.1f} Hz  {amp:.4e}  {bar}")
    peaks = sorted(table, key=lambda t: -t[1])[:5]
    print("peaks: " + "  ".join(f"{f:.0f}Hz={amp:.3e}" for f, amp in peaks))

if __name__ == "__main__":
    main()
