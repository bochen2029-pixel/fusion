// tests/test_ghost.cpp — the GHOST FORK PoC (M2 S18, D-048; relay Q23 promoted). The
// SimEnv is copyable BY DESIGN, so forking the plant at tick T is `SimEnv g = plant;`.
// This de-risks F-GHOST (M3) while the machinery is warm and doubles as the SimEnv
// copy-correctness proof the parity ctest wants. Two claims:
//   (i)  ghost-vs-ghost bit-determinism (F-GHOST criterion i): two forks from the same
//        T-state, run identically to the end, produce IDENTICAL golden fnv.
//   (ii) the fork explores a real COUNTERFACTUAL: a fork with VS disabled from T takes
//        the kick it would otherwise survive and DISRUPTS — the plant does not. That
//        divergence (via the sovereign command alone, shared Philox stream) is the M3
//        ghost's whole mechanism, shown to work.
#include "core/sim_env.h"
#include "core/gs_free.h"
#include <cstdio>

using namespace fusion;

int main(int argc, char** argv) {
    const std::string root = argc > 1 ? argv[1] : ".";
    SimInputs in;
    in.m = load_machine(root + "/contracts/machine.toml");
    in.f = load_floors(root + "/contracts/floors.toml");
    in.g = load_gates(root + "/contracts/spine_gates.toml");
    in.d = load_dispersions(root + "/contracts/dispersions.toml");
    in.s = load_scenario(root + "/contracts/scenarios/vde_kick.toml");
    in.k = load_gains(root + "/control/gains_m2.toml");   // the composite null
    in.ic = load_innov(root + "/contracts/events.toml");
    in.fctx = gs_free_context(in.m);        // ONE shared context (the relay Q13 law)

    const uint64_t seed = 800000;
    const uint64_t T = 30000;               // fork at t = 3.0 s — BEFORE the kick (t=5 s)

    // reference plant: run to completion
    SimEnv plant; plant.reset(in, seed, true);
    while (plant.step()) {}
    RunResult rp = plant.finish();

    // fork at T: step a fresh env to T, then copy it twice and run both to the end
    SimEnv atT; atT.reset(in, seed, true);
    for (uint64_t i = 0; i < T; ++i) atT.step();
    SimEnv g1 = atT;                        // the fork — a value copy (FreeTrack copyable)
    SimEnv g2 = atT;
    while (g1.step()) {}
    while (g2.step()) {}
    RunResult r1 = g1.finish();
    RunResult r2 = g2.finish();

    std::printf("plant  fnv %016llx verdict %u\n", (unsigned long long)rp.fnv, unsigned(rp.verdict));
    std::printf("ghost1 fnv %016llx verdict %u\n", (unsigned long long)r1.fnv, unsigned(r1.verdict));
    std::printf("ghost2 fnv %016llx verdict %u\n", (unsigned long long)r2.fnv, unsigned(r2.verdict));

    // (i) determinism: the two forks are bit-identical, and equal the plant's future
    if (r1.fnv != r2.fnv || r1.fnv != rp.fnv) {
        std::puts("GHOST RED: fork is not bit-deterministic (F-GHOST i)"); return 1; }

    // (ii) counterfactual: fork at T, disable VS from T, run to the end. The kick that
    // the plant survives must now kill the ghost — divergence via command alone.
    SimEnv cf = atT;
    cf.in.s.vs_on = false;                  // the counterfactual: "what if I don't hold it"
    while (cf.step()) {}
    RunResult rc = cf.finish();
    std::printf("counterfactual (VS off from T) fnv %016llx verdict %u z_max %.3f\n",
                (unsigned long long)rc.fnv, unsigned(rc.verdict), rc.z_max_m);
    if (rc.verdict != Verdict::DISRUPT) {
        std::puts("GHOST RED: the VS-off counterfactual did not disrupt (no divergence)"); return 1; }
    if (rp.verdict != Verdict::GOOD) {
        std::puts("GHOST RED: the plant did not survive (bad reference)"); return 1; }

    std::puts("GHOST GREEN (fork is bit-deterministic AND explores a real counterfactual)");
    return 0;
}
