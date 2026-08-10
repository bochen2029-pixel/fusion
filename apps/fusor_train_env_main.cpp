// apps/fusor_train_env_main.cpp — fusor_train_env: the BATCHED-ENV runner (M2 S18,
// D-048; spec §2.2 "the fourth consumer of the one-dynamics-source law", D-011). It
// steps many SimEnvs in parallel, sharing ONE FreeContext (the relay Q13 law: never a
// per-env context build), and reports training-scale throughput. CPU-first (relay §4);
// CUDA is a later port (the tick loop is SimEnv::step). Determinism is per-env (Philox
// counter RNG); thread-safety rides gs_solve's thread_local scratch + the const shared
// context. --parity asserts each threaded env is bit-identical to serial run_sim.
//
// Wall-clock lives HERE (the trainer harness), never in the sim path (SimEnv is clock-free).
// fusor_train_env --root <repo> --scenario <path> [--gains <path>] --seeds A:B
//                 [--threads N] [--parity]
#include "core/sim.h"
#include "core/sim_env.h"
#include "core/gs_free.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>

using namespace fusion;

int main(int argc, char** argv) {
    std::string root = ".", scen, gains;
    uint64_t s_lo = 0, s_hi = 0;
    int threads = (int)std::thread::hardware_concurrency();
    if (threads <= 0) threads = 4;
    bool parity = false;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        auto next = [&]() -> std::string { return (i + 1 < argc) ? argv[++i] : ""; };
        if (a == "--root") root = next();
        else if (a == "--scenario") scen = next();
        else if (a == "--gains") gains = next();
        else if (a == "--threads") threads = std::atoi(next().c_str());
        else if (a == "--parity") parity = true;
        else if (a == "--seeds") {
            std::string v = next(); auto p = v.find(':');
            s_lo = std::strtoull(v.substr(0, p).c_str(), nullptr, 10);
            s_hi = std::strtoull(v.substr(p + 1).c_str(), nullptr, 10);
        }
    }
    if (scen.empty() || s_hi <= s_lo) {
        std::fprintf(stderr, "usage: fusor_train_env --scenario <p> --seeds A:B "
                             "[--threads N] [--gains p] [--parity]\n");
        return 2;
    }

    SimInputs tin;
    tin.m = load_machine(root + "/contracts/machine.toml");
    tin.f = load_floors(root + "/contracts/floors.toml");
    tin.g = load_gates(root + "/contracts/spine_gates.toml");
    tin.d = load_dispersions(root + "/contracts/dispersions.toml");
    tin.s = load_scenario(scen);
    tin.k = gains.empty() ? load_gains(root + "/control/gains_m2.toml") : load_gains(gains);
    tin.ic = load_innov(root + "/contracts/events.toml");
    tin.fctx = tin.s.vert_on ? gs_free_context(tin.m) : nullptr;   // built ONCE, shared

    const uint64_t N = s_hi - s_lo;
    std::vector<uint32_t> verdict(N, 0);
    std::vector<uint64_t> fnv(N, 0);
    std::vector<uint64_t> ticks(N, 0);
    std::atomic<uint64_t> next_i{0};

    // each worker pulls indices atomically and steps a full SimEnv to completion. The
    // SimInputs is copied per env (fctx shared by refcount). record=false (no golden
    // vector churn) except we still want a fnv for parity -> record only in parity mode.
    auto worker = [&]() {
        for (;;) {
            const uint64_t i = next_i.fetch_add(1);
            if (i >= N) break;
            SimEnv e;
            e.reset(tin, s_lo + i, parity);          // record=true only for parity fnv
            uint64_t nt = 0;
            while (e.step()) ++nt;
            RunResult r = e.finish();
            verdict[i] = uint32_t(r.verdict);
            fnv[i] = r.fnv;
            ticks[i] = nt;
        }
    };

    const auto t0 = std::chrono::high_resolution_clock::now();
    std::vector<std::thread> pool;
    for (int t = 0; t < threads; ++t) pool.emplace_back(worker);
    for (auto& th : pool) th.join();
    const auto t1 = std::chrono::high_resolution_clock::now();
    const double wall_s = std::chrono::duration<double>(t1 - t0).count();

    uint64_t good = 0, disrupt = 0, spine = 0, tot_ticks = 0;
    for (uint64_t i = 0; i < N; ++i) {
        switch (Verdict(verdict[i])) {
            case Verdict::GOOD: ++good; break;
            case Verdict::DISRUPT: ++disrupt; break;
            case Verdict::SPINE_SHUTDOWN: ++spine; break;
            default: break;
        }
        tot_ticks += ticks[i];
    }
    std::printf("fusor_train_env  scenario=%s  seeds=[%llu,%llu)  N=%llu  threads=%d\n",
                tin.s.name.c_str(), (unsigned long long)s_lo, (unsigned long long)s_hi,
                (unsigned long long)N, threads);
    std::printf("  GOOD %llu  DISRUPT %llu  SPINE %llu\n",
                (unsigned long long)good, (unsigned long long)disrupt, (unsigned long long)spine);
    std::printf("  wall %.3f s   throughput %.1f env/s   %.2f Mtick/s\n",
                wall_s, N / wall_s, tot_ticks / wall_s / 1e6);

    if (parity) {
        // bit-exact parity: threaded SimEnv vs serial run_sim, per seed (thread-safety
        // does not corrupt determinism — the one-dynamics-source law across consumers).
        uint64_t bad = 0;
        for (uint64_t i = 0; i < N; ++i) {
            RunResult ref = run_sim(tin, s_lo + i, true);
            if (ref.fnv != fnv[i] || uint32_t(ref.verdict) != verdict[i]) {
                if (bad < 5)
                    std::fprintf(stderr, "  PARITY RED seed %llu: threaded fnv %016llx "
                                 "!= serial %016llx\n", (unsigned long long)(s_lo + i),
                                 (unsigned long long)fnv[i], (unsigned long long)ref.fnv);
                ++bad;
            }
        }
        if (bad) { std::fprintf(stderr, "PARITY RED: %llu/%llu seeds diverged\n",
                                (unsigned long long)bad, (unsigned long long)N); return 1; }
        std::printf("  PARITY GREEN: all %llu threaded envs bit-identical to serial run_sim\n",
                    (unsigned long long)N);
    }
    return 0;
}
