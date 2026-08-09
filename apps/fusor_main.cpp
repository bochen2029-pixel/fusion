// apps/fusor_main.cpp — one verbose run: the timeline a human reads.
// fusor --root <repo> --scenario <path> [--seed N] [--gains <path>]
#include "core/sim.h"
#include "core/philox.h"
#include "core/physics_tier0.h"
#include <cstdio>
#include <cstdlib>
#include <string>

using namespace fusion;

static int run(int argc, char** argv);
int main(int argc, char** argv) {
    try { return run(argc, argv); }
    catch (const std::exception& e) {
        std::fprintf(stderr, "fusor: FATAL: %s\n", e.what());
        return 3;
    }
}
static int run(int argc, char** argv) {
    std::string root = ".", scen_path, gains_path; uint64_t seed = 1;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        auto next = [&]() -> std::string { return (i + 1 < argc) ? argv[++i] : ""; };
        if (a == "--root") root = next();
        else if (a == "--scenario") scen_path = next();
        else if (a == "--gains") gains_path = next();
        else if (a == "--seed") seed = std::strtoull(next().c_str(), nullptr, 10);
    }
    if (scen_path.empty()) { std::fprintf(stderr, "fusor: --scenario required\n"); return 2; }
    SimInputs in;
    in.m = load_machine(root + "/contracts/machine.toml");
    in.f = load_floors(root + "/contracts/floors.toml");
    in.g = load_gates(root + "/contracts/spine_gates.toml");
    in.d = load_dispersions(root + "/contracts/dispersions.toml");
    in.s = load_scenario(scen_path);
    in.k = gains_path.empty() ? load_gains(root + "/control/gains_m0.toml")
                              : load_gains(gains_path);
    RunResult r = run_sim(in, seed, true);
    std::printf("fusor  %s seed=%llu  H98=%.3f  puff@%.2fs\n", in.s.name.c_str(),
                (unsigned long long)seed, r.H98_drawn, r.t_puff);
    for (size_t i = 0; i < r.golden.size(); i += 100) {      // print every 1 s
        const GoldenRec& g = r.golden[i];
        std::printf("  t=%6.1fs  T=%6.3f keV  ne=%.3fe20  Q=%6.3f  Paux=%5.1f MW  mode=%u\n",
                    double(g.tick) * DT_TICK, g.T, g.ne / 1e20, g.Q, g.Paux / 1e6, g.mode);
    }
    const char* v = r.verdict == Verdict::GOOD ? "GOOD" :
                    r.verdict == Verdict::DISRUPT ? "DISRUPT" :
                    r.verdict == Verdict::SPINE_SHUTDOWN ? "SPINE_SHUTDOWN" : "TIMEOUT";
    std::printf("  verdict %s  pass %d  minQ[win] %.3f  q_rms %.3f  t_end %.2fs  fnv %016llx\n",
                v, int(r.pass), r.minQ_window, r.q_rms, r.t_end, (unsigned long long)r.fnv);
    return 0;
}
