#!/usr/bin/env python3
"""trainer/lq_design.py — offline DARE design of the LQ vertical null (D-045; dev rig).

Reads the lqdump TSV (test_vertical <root> lqdump 12 1.4e7 3.5e7 out.tsv): per k_dest
grid point, the EKF's OWN discrete 4-state model (Phi, Bd) at NOMINAL constants on the
frozen-coil co-move manifold — exactly the runtime filter behavior, mirrored. Solves
the DARE per point (structured doubling; residual + closed-loop checks ASSERT), writes
a complete gains file: [pid]/[vs] copied VERBATIM from control/gains_m0.toml (the
tuned burn null untouched; PD retained runtime-selectable) + the [lq] schedule.

Q/R from contracts/objective.toml (D-045): Q = diag(qz*10^th1, qz*tau_v^2*10^th2, 0, 0),
R = [actuators].effort / v_max^2 (v_max = 2000 V, the supply clamp in policy_vs).
CEM (tune_lq.py) owns th1/th2 — this file is the deterministic design function.
Pure stdlib (no numpy — tune_pid.py's own law). Forward slashes in all paths.

Usage:
  python C:/fusion/trainer/lq_design.py --dump runs/m2s1/lqdump.tsv --root C:/fusion \
         --out C:/fusion/control/gains_m2.toml [--th1 0] [--th2 0] [--tau-v 0.02] [--print-k]
"""
import argparse, math, re, sys

VMAX = 2000.0   # the supply clamp (policy_vs; D-045)
N = 4

# ---- 4x4 dense helpers (deterministic, fixed order) ---------------------------------
def eye(n):
    return [[1.0 if i == j else 0.0 for j in range(n)] for i in range(n)]

def mmul(A, B):
    n, K, m = len(A), len(B), len(B[0])
    C = [[0.0] * m for _ in range(n)]
    for i in range(n):
        Ai, Ci = A[i], C[i]
        for k in range(K):
            a = Ai[k]
            if a == 0.0:
                continue
            Bk = B[k]
            for j in range(m):
                Ci[j] += a * Bk[j]
    return C

def madd(A, B):
    return [[A[i][j] + B[i][j] for j in range(len(A[0]))] for i in range(len(A))]

def transpose(A):
    return [[A[j][i] for j in range(len(A))] for i in range(len(A[0]))]

def solve(A, B):
    """X = A^-1 B, Gauss-Jordan with partial pivoting (deterministic order)."""
    n, m = len(A), len(B[0])
    M = [A[i][:] + B[i][:] for i in range(n)]
    for c in range(n):
        p = max(range(c, n), key=lambda r: abs(M[r][c]))
        if abs(M[p][c]) < 1e-300:
            raise SystemExit("lq_design: singular solve")
        M[c], M[p] = M[p], M[c]
        piv = M[c][c]
        M[c] = [v / piv for v in M[c]]
        for r in range(n):
            if r != c and M[r][c] != 0.0:
                f = M[r][c]
                M[r] = [vr - f * vc for vr, vc in zip(M[r], M[c])]
    return [row[n:] for row in M]

# ---- DARE (structured doubling; scalar R) -------------------------------------------
def dare_sda(A, Bc, Q, Rs, iters=100):
    """Stabilizing X for X = A'XA - A'XB(R+B'XB)^-1 B'XA + Q; returns (X, K)."""
    Ak = [row[:] for row in A]
    G = [[Bc[i] * Bc[j] / Rs for j in range(N)] for i in range(N)]
    H = [row[:] for row in Q]
    for _ in range(iters):
        W = madd(eye(N), mmul(G, H))
        WiA = solve(W, Ak)                       # W^-1 A_k
        WiG = solve(W, G)                        # W^-1 G_k
        An = mmul(Ak, WiA)
        Gn = madd(G, mmul(Ak, mmul(WiG, transpose(Ak))))
        Hn = madd(H, mmul(transpose(Ak), mmul(H, WiA)))
        dh = max(abs(Hn[i][j] - H[i][j]) for i in range(N) for j in range(N))
        hm = max(abs(Hn[i][j]) for i in range(N) for j in range(N))
        Ak, G, H = An, Gn, Hn
        if dh <= 1e-14 * max(hm, 1.0):
            break
    X = H
    XB = [sum(X[i][j] * Bc[j] for j in range(N)) for i in range(N)]
    BXB = sum(Bc[i] * XB[i] for i in range(N))
    XA = mmul(X, A)
    K = [sum(Bc[i] * XA[i][j] for i in range(N)) / (Rs + BXB) for j in range(N)]
    return X, K

def dare_checks(A, Bc, Q, Rs, X, K):
    """Residual (relative) + closed-loop spectral radius + 2 s decay. ASSERTS."""
    Acl = [[A[i][j] - Bc[i] * K[j] for j in range(N)] for i in range(N)]
    T = mmul(transpose(A), mmul(X, Acl))
    resid = max(abs(T[i][j] + Q[i][j] - X[i][j]) for i in range(N) for j in range(N))
    xmax = max(abs(X[i][j]) for i in range(N) for j in range(N))
    rel = resid / max(xmax, 1e-300)
    if rel > 1e-8:
        raise SystemExit(f"lq_design: DARE residual {rel:.2e} > 1e-8")
    # spectral radius: 200 warm normalized iters, then 100 measuring mean log growth
    v = [1.0, 0.5, 0.25, 0.125]
    for _ in range(200):
        w = [sum(Acl[i][j] * v[j] for j in range(N)) for i in range(N)]
        nr = math.sqrt(sum(x * x for x in w)) or 1e-300
        v = [x / nr for x in w]
    lg = 0.0
    for _ in range(100):
        w = [sum(Acl[i][j] * v[j] for j in range(N)) for i in range(N)]
        nr = math.sqrt(sum(x * x for x in w)) or 1e-300
        lg += math.log(nr)
        v = [x / nr for x in w]
    rho = math.exp(lg / 100.0)
    if rho >= 0.99999:
        raise SystemExit(f"lq_design: closed loop not stable (rho {rho:.6f})")
    # time-domain: 20000 steps (2 s at dt 1e-4) from a 1 mm offset must decay
    x = [1e-3, 0.0, 0.0, 0.0]
    n0 = math.sqrt(sum(a * a for a in x))
    for _ in range(20000):
        x = [sum(Acl[i][j] * x[j] for j in range(N)) for i in range(N)]
    if math.sqrt(sum(a * a for a in x)) >= n0:
        raise SystemExit("lq_design: closed loop did not decay over 2 s")
    return rho

# ---- IO -----------------------------------------------------------------------------
def read_dump(path):
    rows = []
    for line in open(path, encoding="utf-8"):
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        v = [float(x) for x in line.split()]
        if len(v) != 6 + 16 + 4:
            raise SystemExit(f"lq_design: bad dump row ({len(v)} cols)")
        Phi = [[v[6 + 4 * i + j] for j in range(4)] for i in range(4)]
        Bd = [v[22 + i] for i in range(4)]
        rows.append({"kd": v[0], "Ip": v[1], "gw": v[2], "kwall": v[3],
                     "m_eff": v[4], "cs": v[5], "Phi": Phi, "Bd": Bd})
    if len(rows) < 2:
        raise SystemExit("lq_design: dump needs >= 2 grid points")
    if any(rows[i + 1]["kd"] <= rows[i]["kd"] for i in range(len(rows) - 1)):
        raise SystemExit("lq_design: dump kd must ascend")
    return rows

def read_objective(root):
    txt = open(f"{root}/contracts/objective.toml", encoding="utf-8").read()
    try:
        import tomllib
        t = tomllib.loads(txt)
        return float(t["tracking"]["z_position_m"]), float(t["actuators"]["effort"])
    except Exception:
        mz = re.search(r"^z_position_m\s*=\s*([0-9.eE+-]+)", txt, re.M)
        me = re.search(r"^effort\s*=\s*([0-9.eE+-]+)", txt, re.M)
        if not mz or not me:
            raise SystemExit("lq_design: objective.toml parse failed")
        return float(mz.group(1)), float(me.group(1))

def toml_block(txt, name):
    """Extract '[name]' through the line before the next '[' header. ASSERT found."""
    m = re.search(rf"^\[{name}\]\s*$(.*?)(?=^\[|\Z)", txt, re.M | re.S)
    if not m or not m.group(1).strip():
        raise SystemExit(f"lq_design: [{name}] block missing in gains_m0.toml")
    return f"[{name}]\n" + m.group(1).strip() + "\n"

def design(dump_path, root, th1, th2, tau_v):
    rows = read_dump(dump_path)
    qz, effort = read_objective(root)
    Rs = effort / (VMAX * VMAX)
    Q = [[0.0] * N for _ in range(N)]
    Q[0][0] = qz * (10.0 ** th1)
    Q[1][1] = qz * tau_v * tau_v * (10.0 ** th2)
    out = []
    for rw in rows:
        X, K = dare_sda(rw["Phi"], rw["Bd"], Q, Rs)
        rho = dare_checks(rw["Phi"], rw["Bd"], Q, Rs, X, K)
        out.append({"kd": rw["kd"], "Ip": rw["Ip"], "gw": rw["gw"], "K": K, "rho": rho})
    return out, qz, effort

def design_and_write(dump_path, root, out_path, th1, th2, tau_v=0.02, verbose=False):
    pts, qz, effort = design(dump_path, root, th1, th2, tau_v)
    m0 = open(f"{root}/control/gains_m0.toml", encoding="utf-8").read()
    pid_b, vs_b = toml_block(m0, "pid"), toml_block(m0, "vs")
    def arr(key, vals, fmt):
        return f"{key} = [" + ", ".join(fmt % v for v in vals) + "]\n"
    with open(out_path, "w", encoding="utf-8", newline="\n") as f:
        f.write("# control/gains_m2.toml -- the M2 composite null (D-045).\n"
                "# [pid]/[vs] VERBATIM from gains_m0.toml (M0 CEM receipt; PD retained\n"
                "# runtime-selectable). [lq] = DARE schedule (trainer/lq_design.py):\n"
                f"# th1={th1:.6f} th2={th2:.6f} tau_v={tau_v:.4f}  "
                f"qz={qz:.1f} effort={effort:.4f} vmax={VMAX:.0f}\n")
        f.write(pid_b)
        f.write(vs_b)
        f.write("[lq]\non = true\n")
        f.write(arr("kdest", [p["kd"] for p in pts], "%.8e"))
        f.write(arr("Kz", [p["K"][0] for p in pts], "%.10g"))
        f.write(arr("Kv", [p["K"][1] for p in pts], "%.10g"))
        f.write(arr("Kq", [p["K"][2] for p in pts], "%.10g"))
        f.write(arr("Ki", [p["K"][3] for p in pts], "%.10g"))
    if verbose:
        print(f"lq_design: {len(pts)} points -> {out_path}")
        print("  kd          Ip[MA]  g^-1[ms]  rho_cl    Kz          Kv        Kq        Ki")
        for p in pts:
            print(f"  {p['kd']:.3e} {p['Ip']/1e6:6.2f} {1e3/p['gw']:8.1f}  "
                  f"{p['rho']:.6f}  {p['K'][0]:10.4g} {p['K'][1]:9.4g} "
                  f"{p['K'][2]:9.4g} {p['K'][3]:9.4g}")
    return pts

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--dump", required=True)
    ap.add_argument("--root", default="C:/fusion")
    ap.add_argument("--out", default="")
    ap.add_argument("--th1", type=float, default=0.0)
    ap.add_argument("--th2", type=float, default=0.0)
    ap.add_argument("--tau-v", type=float, default=0.02)
    ap.add_argument("--print-k", action="store_true")
    a = ap.parse_args()
    out = a.out or f"{a.root}/control/gains_m2.toml"
    design_and_write(a.dump, a.root, out, a.th1, a.th2, a.tau_v,
                     verbose=(a.print_k or True))

if __name__ == "__main__":
    main()
