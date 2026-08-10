// tests/test_net.cpp — the net-runtime gate (M2 S19, D-049; relay Q18 KAT + byte-golden).
// Three claims, the substrate before the training:
//   (1) KAT: the flat weights the trainer exports load into control/net.h and produce the
//       expected output the exporter computed — the CPU inference path is correct and
//       stable (regression + Python<->C++ agreement within float32 arithmetic tol).
//   (2) zero weights => zero output (the residual identity).
//   (3) a net-DRIVEN sim run with the zero net is BIT-IDENTICAL to the pure null (the
//       residual-on-LQG path is zero-cost-to-trajectory at zero weights — ship the
//       runtime before you train it).
#include "core/sim.h"
#include "core/sim_env.h"
#include "core/gs_free.h"
#include "control/net.h"
#include <cstdio>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace fusion;

int main(int argc, char** argv) {
    const std::string root = argc > 1 ? argv[1] : ".";
    const std::string dir = root + "/goldens/net";

    // (1) KAT — load net_kat.bin, forward the KAT input, compare to net_kat.txt
    NetMLP kat;
    if (!kat.load(dir + "/net_kat.bin")) {
        std::fprintf(stderr, "NET RED: cannot load %s/net_kat.bin (run export_net.py)\n",
                     dir.c_str()); return 1; }
    std::ifstream kf(dir + "/net_kat.txt");
    if (!kf) { std::fprintf(stderr, "NET RED: cannot open net_kat.txt\n"); return 1; }
    std::string line; std::vector<double> xin, yexp;
    while (std::getline(kf, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream ss(line); double v; std::vector<double> row;
        while (ss >> v) row.push_back(v);
        if (xin.empty()) xin = row; else yexp = row;
    }
    if (int(xin.size()) != kat.n_in || int(yexp.size()) != kat.n_out) {
        std::fprintf(stderr, "NET RED: KAT dims mismatch (in %zu/%d out %zu/%d)\n",
                     xin.size(), kat.n_in, yexp.size(), kat.n_out); return 1; }
    std::vector<float> xf(xin.begin(), xin.end()), yf(kat.n_out);
    kat.forward(xf.data(), yf.data());
    double maxerr = 0;
    for (int i = 0; i < kat.n_out; ++i) maxerr = std::fmax(maxerr, std::fabs(yf[i] - yexp[i]));
    std::printf("KAT: out %.9g expected %.9g  err %.2e\n", double(yf[0]), yexp[0], maxerr);
    if (maxerr > 1e-3) { std::fprintf(stderr, "NET RED: KAT output diverged (%.2e)\n", maxerr); return 1; }

    // (2) zero weights => zero output
    NetMLP z;
    if (!z.load(dir + "/net_zero.bin")) { std::fprintf(stderr, "NET RED: no net_zero.bin\n"); return 1; }
    std::vector<float> zo(z.n_out, 9.0f);
    z.forward(xf.data(), zo.data());
    for (int i = 0; i < z.n_out; ++i)
        if (zo[i] != 0.0f) { std::fprintf(stderr, "NET RED: zero net output %.3g != 0\n", zo[i]); return 1; }
    std::puts("zero net: output == 0 exactly  OK");

    // (3) net-driven run (zero net) == pure null, bit-identical, on the vertical path
    SimInputs in;
    in.m = load_machine(root + "/contracts/machine.toml");
    in.f = load_floors(root + "/contracts/floors.toml");
    in.g = load_gates(root + "/contracts/spine_gates.toml");
    in.d = load_dispersions(root + "/contracts/dispersions.toml");
    in.s = load_scenario(root + "/contracts/scenarios/vde_kick.toml");
    in.k = load_gains(root + "/control/gains_m2.toml");   // the LQ null
    in.ic = load_innov(root + "/contracts/events.toml");
    in.fctx = gs_free_context(in.m);
    for (uint64_t seed : { 800000ull, 800005ull, 900123ull }) {
        RunResult null_run = run_sim(in, seed, true);
        in.net = &z;                                      // zero residual
        RunResult net_run = run_sim(in, seed, true);
        in.net = nullptr;
        if (null_run.fnv != net_run.fnv || null_run.verdict != net_run.verdict) {
            std::fprintf(stderr, "NET RED: zero-net run seed %llu != null (%016llx vs %016llx)\n",
                         (unsigned long long)seed, (unsigned long long)null_run.fnv,
                         (unsigned long long)net_run.fnv); return 1; }
    }
    std::puts("zero-net sim == pure null, bit-identical (3 seeds)  OK");
    std::puts("NET GREEN (flat-weight KAT + residual identity + zero-cost-at-zero-weights)");
    return 0;
}
