// tests/test_replay.cpp — THE memcmp replay oracle (booster constitution rule 4).
// Same (scenario, seed, empty input tape) twice => bit-identical golden byte stream.
#include "core/sim.h"
#include "core/gs.h"
#include "core/gs_free.h"
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
    // D-041: the vde path with the free-boundary pipeline in the loop. The precomputed
    // (SimInputs.fctx set) and on-demand (unset) contexts must yield BIT-IDENTICAL
    // runs — gs_free_context is a pure deterministic function of the machine config.
    in.s = load_scenario(root + "/contracts/scenarios/vde_kick.toml");
    in.fctx.reset();                                // unset: run_sim builds internally
    RunResult u = run_sim(in, 777, true);
    in.fctx = gs_free_context(in.m);                // set: the caller-precomputed path
    RunResult p = run_sim(in, 777, true);
    if (u.golden.size() != p.golden.size() ||
        std::memcmp(u.golden.data(), p.golden.data(),
                    u.golden.size() * sizeof(GoldenRec)) != 0 || u.fnv != p.fnv) {
        std::fprintf(stderr, "REPLAY ORACLE RED: fctx precomputed vs on-demand diverged\n");
        return 1;
    }
    std::printf("replay vde_kick 777: precomputed==on-demand, fnv %016llx  OK\n",
                (unsigned long long)p.fnv);
    // the RAMP path replays bit-exactly too (the pipeline schedule is deterministic)
    in.s = load_scenario(root + "/contracts/scenarios/rampdown.toml");
    RunResult ra = run_sim(in, 4242, true);
    RunResult rb = run_sim(in, 4242, true);
    if (ra.fnv != rb.fnv || ra.gs_solves != rb.gs_solves || ra.gs_late != rb.gs_late) {
        std::fprintf(stderr, "REPLAY ORACLE RED: rampdown pipeline diverged\n");
        return 1;
    }
    std::printf("replay rampdown 4242: fnv %016llx  gs_solves %ld  gs_late %ld  OK\n",
                (unsigned long long)ra.fnv, ra.gs_solves, ra.gs_late);
    std::puts("REPLAY ORACLE GREEN");
    return 0;
}
