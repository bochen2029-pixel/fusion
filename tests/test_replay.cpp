// tests/test_replay.cpp — THE memcmp replay oracle (booster constitution rule 4).
// Same (scenario, seed, empty input tape) twice => bit-identical golden byte stream.
#include "core/sim.h"
#include "core/gs.h"
#include <cstdio>
#include <cstring>

using namespace fusion;

int main(int argc, char** argv) {
    const std::string root = argc > 1 ? argv[1] : ".";
    SimInputs in;
    in.m = load_machine(root + "/contracts/machine.toml");
    in.f = load_floors(root + "/contracts/floors.toml");
    in.g = load_gates(root + "/contracts/spine_gates.toml");
    in.d = load_dispersions(root + "/contracts/dispersions.toml");
    in.s = load_scenario(root + "/contracts/scenarios/easy.toml");
    in.k = load_gains(root + "/control/gains_m0.toml");
    for (uint64_t seed : { 900001ull, 123ull, 4242ull }) {
        RunResult a = run_sim(in, seed, true);
        RunResult b = run_sim(in, seed, true);
        if (a.golden.size() != b.golden.size() ||
            std::memcmp(a.golden.data(), b.golden.data(),
                        a.golden.size() * sizeof(GoldenRec)) != 0 ||
            a.fnv != b.fnv) {
            std::fprintf(stderr, "REPLAY ORACLE RED: seed %llu diverged\n",
                         (unsigned long long)seed);
            return 1;
        }
        std::printf("replay seed %llu: %zu recs, fnv %016llx == %016llx  OK\n",
                    (unsigned long long)seed, a.golden.size(),
                    (unsigned long long)a.fnv, (unsigned long long)b.fnv);
    }
    // The MERGE (D-038): the vde path with the equilibrium in the loop. The precomputed
    // (SimInputs.vd set) and on-demand (unset) derivations must yield BIT-IDENTICAL
    // runs — gs_vertical_derive is a pure deterministic function of the machine config.
    in.s = load_scenario(root + "/contracts/scenarios/vde_kick.toml");
    in.vd = VertDerived{};                          // unset: run_sim derives internally
    RunResult u = run_sim(in, 777, true);
    in.vd = gs_vertical_derive(in.m);               // set: the caller-precomputed path
    RunResult p = run_sim(in, 777, true);
    if (u.golden.size() != p.golden.size() ||
        std::memcmp(u.golden.data(), p.golden.data(),
                    u.golden.size() * sizeof(GoldenRec)) != 0 || u.fnv != p.fnv) {
        std::fprintf(stderr, "REPLAY ORACLE RED: vd precomputed vs on-demand diverged\n");
        return 1;
    }
    std::printf("replay vde_kick 777: precomputed==on-demand, fnv %016llx  OK\n",
                (unsigned long long)p.fnv);
    std::puts("REPLAY ORACLE GREEN");
    return 0;
}
