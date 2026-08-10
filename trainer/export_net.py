#!/usr/bin/env python3
"""trainer/export_net.py — flat-weight export for the residual-on-LQG net (M2 S19, D-049).

Writes the byte format control/net.h reads: [magic 'FNET'][i32 n_in][i32 n_hidden]
[i32 n_out] then float32 W0,b0,W1,b1,W2,b2 (row-major, 2 hidden layers). This is the
seam the trainer (torch, later) exports through; here it emits the SUBSTRATE fixtures:
  - net_zero.bin      : all-zero weights (residual == 0 -> bit-identical to the null)
  - net_kat.bin       : deterministic weights (a known-answer net)
  - net_kat.txt       : the KAT input vector + the expected output (float32 weights,
                        computed here) — test_net.cpp asserts the C++ runtime agrees.
Pure stdlib (no numpy — tune_pid.py's law). Weights are rounded to float32 BEFORE the
expected-output computation so only float32-vs-float64 ARITHMETIC differs (tol 1e-3).
Forward order MATCHES net.h exactly (b + sum_j w[j]*x[j], tanh hidden, linear out).

Usage: python C:/fusion/trainer/export_net.py --out-dir C:/fusion/goldens/net
"""
import argparse, math, os, struct

NIN, NHID, NOUT = 18, 64, 1     # 6 features x 3 frames; width 64 (FORWARD_NOTES §5 budget)
MAGIC = 0x54454E46             # 'FNET'

def f32(x):                    # round a python float to float32 (what net.h stores/reads)
    return struct.unpack('f', struct.pack('f', x))[0]

def gen(kind):
    """deterministic weights; kind 'zero' or 'kat'."""
    def val(a, b, c):
        if kind == 'zero':
            return 0.0
        return f32(0.05 * math.sin(a * 12.9898 + b * 78.233 + c * 37.719))
    W0 = [val(0, i, j) for i in range(NHID) for j in range(NIN)]
    b0 = [val(1, i, 0) for i in range(NHID)]
    W1 = [val(2, i, j) for i in range(NHID) for j in range(NHID)]
    b1 = [val(3, i, 0) for i in range(NHID)]
    W2 = [val(4, i, j) for i in range(NOUT) for j in range(NHID)]
    b2 = [val(5, i, 0) for i in range(NOUT)]
    return W0, b0, W1, b1, W2, b2

def forward(w, x):
    """float64 arithmetic over float32-rounded weights (matches net.h's fixed order)."""
    W0, b0, W1, b1, W2, b2 = w
    h0 = [math.tanh(b0[i] + sum(W0[i * NIN + j] * x[j] for j in range(NIN)))
          for i in range(NHID)]
    h1 = [math.tanh(b1[i] + sum(W1[i * NHID + j] * h0[j] for j in range(NHID)))
          for i in range(NHID)]
    return [b2[i] + sum(W2[i * NHID + j] * h1[j] for j in range(NHID)) for i in range(NOUT)]

def write_bin(path, w):
    with open(path, 'wb') as f:
        f.write(struct.pack('<Iiii', MAGIC, NIN, NHID, NOUT))
        for arr in w:
            f.write(struct.pack('<%df' % len(arr), *arr))

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--out-dir', default='C:/fusion/goldens/net')
    a = ap.parse_args()
    os.makedirs(a.out_dir, exist_ok=True)
    wz = gen('zero'); write_bin(os.path.join(a.out_dir, 'net_zero.bin').replace('\\', '/'), wz)
    wk = gen('kat');  write_bin(os.path.join(a.out_dir, 'net_kat.bin').replace('\\', '/'), wk)
    # KAT input: a fixed, non-trivial vector; expected output from the float32 weights
    x = [f32(0.1 * math.sin(i * 1.7 + 0.3)) for i in range(NIN)]
    yk = forward(wk, x)
    kat = os.path.join(a.out_dir, 'net_kat.txt').replace('\\', '/')
    with open(kat, 'w', encoding='utf-8', newline='\n') as f:
        f.write('# net_kat: %d-in %d-hid %d-out; KAT input then expected output\n'
                % (NIN, NHID, NOUT))
        f.write(' '.join('%.9g' % v for v in x) + '\n')
        f.write(' '.join('%.9g' % v for v in yk) + '\n')
    print('export_net: wrote net_zero.bin, net_kat.bin, net_kat.txt to', a.out_dir)
    print('  KAT expected output:', ' '.join('%.6g' % v for v in yk))

if __name__ == '__main__':
    main()
