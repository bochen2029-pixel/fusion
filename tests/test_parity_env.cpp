// tests/test_parity_env.cpp — parity_train_env (M2 S18, D-048; the relay Q18 gate). The
// batched training envs are the FOURTH consumer of the one-dynamics-source law (D-011);
// a ctest asserts bit-exact parity. Two checks:
//   (1) SimEnv stepwise == run_sim one-shot (the refactor is faithful — belt to the
//       golden_check suspenders, on the vertical + pipeline path this time).
//   (2) THREADED SimEnv == serial run_sim (thread-safety does not corrupt determinism:
//       gs_solve's thread_local scratch + the const shared FreeContext hold).
#include "core/sim.h"
#include "core/sim_env.h"
#include "core/gs_free.h"
#include <cstdio>
#include <vector>
#include <thread>
#include <atomic>

using namespace fusion;

int main(int argc, char** argv) {
    const std::string root = argc > 1 ? argv[1] : ".";
    SimInputs in;
    in.m = load_machine(root + "/contracts/machine.toml");
    in.f = load_floors(root + "/contracts/floors.toml");
    in.g = load_gates(root + "/contracts/spine_gates.toml");
    in.d = load_dispersions(root + "/contracts/dispersions.toml");
    in.k = load_gains(root + "/control/gains_m2.toml");
    in.ic = load_innov(root + "/contracts/events.toml");

    // vertical + pipeline path (the hardest determinism surface) + a burn-only path
    for (const char* scn : { "vde_kick", "rampdown", "easy" }) {
        in.s = load_scenario(root + "/contracts/scenarios/" + scn + ".toml");
        in.fctx = in.s.vert_on ? gs_free_context(in.m) : nullptr;

        // (1) stepwise SimEnv == one-shot run_sim, seed by seed
        for (uint64_t seed : { 800000ull, 800001ull, 4242ull }) {
            SimEnv e; e.reset(in, seed, true);
            while (e.step()) {}
            RunResult a = e.finish();
            RunResult b = run_sim(in, seed, true);
            if (a.fnv != b.fnv || a.verdict != b.verdict || a.gs_solves != b.gs_solves) {
                std::fprintf(stderr, "PARITY RED: %s seed %llu stepwise != one-shot "
                             "(%016llx vs %016llx)\n", scn, (unsigned long long)seed,
                             (unsigned long long)a.fnv, (unsigned long long)b.fnv);
                return 1;
            }
        }

        // (2) threaded == serial over a small block (thread-safety parity)
        const uint64_t LO = 800000, NB = 32;
        std::vector<uint64_t> tf(NB, 0); std::vector<uint32_t> tv(NB, 0);
        std::atomic<uint64_t> ni{0};
        auto work = [&]() {
            for (;;) { const uint64_t i = ni.fetch_add(1); if (i >= NB) break;
                SimEnv e; e.reset(in, LO + i, true);
                while (e.step()) {}
                RunResult r = e.finish(); tf[i] = r.fnv; tv[i] = uint32_t(r.verdict); }
        };
        std::vector<std::thread> pool;
        const int T = 8;
        for (int t = 0; t < T; ++t) pool.emplace_back(work);
        for (auto& th : pool) th.join();
        for (uint64_t i = 0; i < NB; ++i) {
            RunResult ref = run_sim(in, LO + i, true);
            if (ref.fnv != tf[i] || uint32_t(ref.verdict) != tv[i]) {
                std::fprintf(stderr, "PARITY RED: %s seed %llu threaded != serial\n",
                             scn, (unsigned long long)(LO + i));
                return 1;
            }
        }
        std::printf("parity %s: stepwise==one-shot AND %llu threaded==serial  OK\n",
                    scn, (unsigned long long)NB);
    }
    std::puts("PARITY_TRAIN_ENV GREEN (SimEnv is the plant, batched and threaded, bit-exact)");
    return 0;
}
