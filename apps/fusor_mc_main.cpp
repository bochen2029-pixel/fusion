// apps/fusor_mc_main.cpp — headless Monte-Carlo: the gate for every controller change.
// fusor_mc --root <repo> --scenario <path> --seeds A:B [--gains <path>] [--json]
//          [--dump-golden <seed> --goldens-dir <dir>]
#include "core/sim.h"
#include "core/gs.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <string>
#include <vector>

using namespace fusion;

static void wilson(double p, double n, double& lo, double& hi) {
    if (n <= 0) { lo = hi = 0; return; }
    const double z = 1.959963985, z2 = z * z;
    const double den = 1.0 + z2 / n;
    const double c = p + z2 / (2 * n);
    const double half = z * std::sqrt(p * (1 - p) / n + z2 / (4 * n * n));
    lo = (c - half) / den; hi = (c + half) / den;
}

static int run(int argc, char** argv);
int main(int argc, char** argv) {
    try { return run(argc, argv); }
    catch (const std::exception& e) {
        std::fprintf(stderr, "fusor_mc: FATAL: %s\n", e.what());
        return 3;
    }
}
static int run(int argc, char** argv) {
    std::string root = ".", scen_path, gains_path, goldens_dir;
    uint64_t s_lo = 0, s_hi = 0; long long dump_seed = -1; bool json = false, vfails = false;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        auto next = [&]() -> std::string { return (i + 1 < argc) ? argv[++i] : ""; };
        if (a == "--root") root = next();
        else if (a == "--scenario") scen_path = next();
        else if (a == "--gains") gains_path = next();
        else if (a == "--goldens-dir") goldens_dir = next();
        else if (a == "--json") json = true;
        else if (a == "--verbose-fails") vfails = true;
        else if (a == "--dump-golden") dump_seed = std::atoll(next().c_str());
        else if (a == "--seeds") {
            std::string v = next(); auto p = v.find(':');
            s_lo = std::strtoull(v.substr(0, p).c_str(), nullptr, 10);
            s_hi = std::strtoull(v.substr(p + 1).c_str(), nullptr, 10);
        }
    }
    if (scen_path.empty()) { std::fprintf(stderr, "fusor_mc: --scenario required\n"); return 2; }

    SimInputs in;
    in.m = load_machine(root + "/contracts/machine.toml");
    in.f = load_floors(root + "/contracts/floors.toml");
    in.g = load_gates(root + "/contracts/spine_gates.toml");
    in.d = load_dispersions(root + "/contracts/dispersions.toml");
    in.s = load_scenario(scen_path);
    in.k = gains_path.empty() ? load_gains(root + "/control/gains_m0.toml")
                              : load_gains(gains_path);
    in.ic = load_innov(root + "/contracts/events.toml");
    if (in.s.vert_on) in.vd = gs_vertical_derive(in.m);   // once per process (D-038)
    const ObjectiveCfg obj = load_objective(root + "/contracts/objective.toml");

    if (dump_seed >= 0) {                       // golden production path
        RunResult r = run_sim(in, uint64_t(dump_seed), true);
        const std::string base = goldens_dir + "/m0-" + in.s.name + "-" + std::to_string(dump_seed);
        FILE* f = std::fopen((base + ".golden").c_str(), "wb");
        if (!f) { std::fprintf(stderr, "cannot write %s\n", base.c_str()); return 2; }
        std::fwrite(r.golden.data(), sizeof(GoldenRec), r.golden.size(), f);
        std::fclose(f);
        FILE* h = std::fopen((base + ".fnv").c_str(), "wb");
        std::fprintf(h, "%016llx\n", (unsigned long long)r.fnv);
        std::fclose(h);
        std::printf("golden %s: %zu recs fnv %016llx verdict %u pass %d\n", base.c_str(),
                    r.golden.size(), (unsigned long long)r.fnv, unsigned(r.verdict), int(r.pass));
        return 0;
    }

    uint64_t n = 0, good = 0, disrupt = 0, spine = 0, npass = 0;
    double q_rms_sum = 0, cost_sum = 0, minq_sum = 0; uint64_t q_n = 0;
    for (uint64_t seed = s_lo; seed < s_hi; ++seed) {
        RunResult r = run_sim(in, seed, false);
        ++n;
        double cost = 0;
        switch (r.verdict) {
            case Verdict::GOOD: {
                ++good; q_rms_sum += r.q_rms; minq_sum += r.minQ_window; ++q_n;
                const double sh = std::max(0.0, in.s.q_min - r.minQ_window);
                cost += obj.minq_shortfall_w * sh * sh + (r.pass ? 0.0 : obj.gate_miss_cost);
                break;
            }
            case Verdict::DISRUPT: ++disrupt; cost += obj.disrupt_cost; break;
            case Verdict::SPINE_SHUTDOWN: ++spine; cost += obj.spine_cost; break;
            case Verdict::TIMEOUT: cost += obj.timeout_cost; break;
        }
        if (r.pass) ++npass;
        else if (vfails)
            std::printf("  FAIL seed %llu  verdict %u  minQ %.3f  H98 %.3f  puff@%.2fs\n",
                        (unsigned long long)seed, unsigned(r.verdict), r.minQ_window,
                        r.H98_drawn, r.t_puff);
        cost += obj.q_weight * r.q_rms * r.q_rms + obj.effort_weight * r.effort * 100.0;
        cost_sum += cost;
    }
    const double N = double(n ? n : 1);
    const double p_pass = double(npass) / N;
    double lo, hi; wilson(p_pass, N, lo, hi);
    if (json) {
        std::printf("{\"n\":%llu,\"good\":%llu,\"disrupt\":%llu,\"spine\":%llu,"
                    "\"pass\":%llu,\"pass_frac\":%.6f,\"wilson_lo\":%.6f,\"wilson_hi\":%.6f,"
                    "\"q_rms_mean\":%.6f,\"minq_mean\":%.6f,\"cost_mean\":%.4f}\n",
                    (unsigned long long)n, (unsigned long long)good,
                    (unsigned long long)disrupt, (unsigned long long)spine,
                    (unsigned long long)npass, p_pass, lo, hi,
                    q_n ? q_rms_sum / double(q_n) : 0.0,
                    q_n ? minq_sum / double(q_n) : 0.0, cost_sum / N);
    } else {
        std::printf("fusor_mc  scenario=%s  seeds=[%llu,%llu)  N=%llu\n",
                    in.s.name.c_str(), (unsigned long long)s_lo, (unsigned long long)s_hi,
                    (unsigned long long)n);
        std::printf("  GOOD %llu  DISRUPT %llu  SPINE_SHUTDOWN %llu\n",
                    (unsigned long long)good, (unsigned long long)disrupt,
                    (unsigned long long)spine);
        std::printf("  pass %llu/%llu = %.4f  Wilson95 [%.4f, %.4f]\n",
                    (unsigned long long)npass, (unsigned long long)n, p_pass, lo, hi);
        std::printf("  q_rms(mean,GOOD) %.4f   minQ(mean,GOOD) %.4f   cost(mean) %.2f\n",
                    q_n ? q_rms_sum / double(q_n) : 0.0,
                    q_n ? minq_sum / double(q_n) : 0.0, cost_sum / N);
    }
    return 0;
}
