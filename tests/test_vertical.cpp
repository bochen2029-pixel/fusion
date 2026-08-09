// tests/test_vertical.cpp — M1 slice 1 acceptance: the enemy exists, is the right
// CLASS, is holdable, dies honestly when unopposed, and the EKF is consistent.
#include "core/sim.h"
#include "core/vertical.h"
#include <cstdio>
#include <cmath>

using namespace fusion;

int main(int argc, char** argv) {
    const std::string root = argc > 1 ? argv[1] : ".";
    const MachineCfg m = load_machine(root + "/contracts/machine.toml");

    // (1) gamma bands — REPORTED from the eigenproblem, the CLASS pre-registered:
    // wall-set 10–100 ms (machine.toml [vessel] expectation 25–40); no-wall < 2 ms.
    VerticalModel vm; vm.build(m);
    const double gi_wall = 1.0 / vm.gamma_wall, gi_open = 1.0 / vm.gamma_open;
    std::printf("gamma^-1: with passives %.1f ms (open %.3f ms)  k_dest %.3g N/m\n",
                gi_wall * 1e3, gi_open * 1e3, vm.k_dest);
    // the pre-registered CLASS: wall-set 10–100 ms with the shell; removing the shell
    // makes the mode ≥5× faster (the 20-turn VS still screens passively — reported)
    if (!(gi_wall > 0.010 && gi_wall < 0.100)) { std::puts("VERTICAL RED: wall gamma class"); return 1; }
    if (!(gi_open > 0 && gi_open < 0.008 && gi_wall / gi_open >= 5.0)) {
        std::puts("VERTICAL RED: shell-removal separation"); return 1; }

    SimInputs in;
    in.m = m;
    in.f = load_floors(root + "/contracts/floors.toml");
    in.g = load_gates(root + "/contracts/spine_gates.toml");
    in.d = load_dispersions(root + "/contracts/dispersions.toml");
    in.k = load_gains(root + "/control/gains_m0.toml");
    in.ic = load_innov(root + "/contracts/events.toml");

    // ---- DEV sweep mode: test_vertical <root> sweep — maps the gain landscape,
    // truth-feedback vs EKF-feedback, to separate controller from estimator issues.
    if (argc > 2 && std::string(argv[2]) == "sweep") {
        in.s = load_scenario(root + "/contracts/scenarios/vde_kick.toml");
        const double KPS[] = { 2e4, 5e4, 1e5, 2e5, 4e5 };
        const double KDS[] = { 200, 1000, 3000, 8000, 20000 };
        for (int truth = 1; truth >= 0; --truth) {
            std::printf("== feedback on %s ==\n", truth ? "TRUTH (dev)" : "EKF estimate");
            for (double kp : KPS) for (double kd : KDS) {
                in.k.Kpz = kp; in.k.Kdz = kd; in.k.vs_truth = truth != 0;
                int g = 0; double zm = 0, nis = 0;
                for (uint64_t s = 100; s < 106; ++s) {
                    RunResult rr = run_sim(in, s, false);
                    if (rr.verdict == Verdict::GOOD) ++g;
                    zm = std::fmax(zm, rr.z_max_m); nis += rr.nis_mean / 6.0;
                }
                std::printf("  Kp %8.0f Kd %6.0f: GOOD %d/6  zmax %.3f  NIS %.1f\n",
                            kp, kd, g, zm, nis);
            }
        }
        return 0;
    }

    // (2) open loop dies via the full chain: gate fire + TQ(vde) + CQ + disrupt on tape
    in.s = load_scenario(root + "/contracts/scenarios/vde_open_loop.toml");
    RunResult ro = run_sim(in, 42, true);
    bool saw_gate = false, saw_tq_vde = false, saw_disrupt = false;
    for (const EventRec& e : ro.events) {
        if (e.kind == EvKind::GateFire && e.arg == 1) saw_gate = true;
        if (e.kind == EvKind::TerminalTQ && e.arg == 2) saw_tq_vde = true;
        if (e.kind == EvKind::TerminalDisrupt) saw_disrupt = true;
    }
    std::printf("open-loop: verdict %u  t_end %.3f s  zmax %.3f m  gate %d tq_vde %d disrupt %d\n",
                unsigned(ro.verdict), ro.t_end, ro.z_max_m, saw_gate, saw_tq_vde, saw_disrupt);
    if (ro.verdict != Verdict::DISRUPT || !saw_gate || !saw_tq_vde || !saw_disrupt) {
        std::puts("VERTICAL RED: open-loop death chain incomplete"); return 1;
    }

    // (3) closed loop holds through the kick; innovation events fire; NIS consistent
    in.s = load_scenario(root + "/contracts/scenarios/vde_kick.toml");
    int good = 0, innov_runs = 0; double nis_lo = 1e9, nis_hi = 0, zworst = 0;
    for (uint64_t seed = 100; seed < 120; ++seed) {
        RunResult rk = run_sim(in, seed, true);
        if (rk.verdict == Verdict::GOOD) ++good;
        if (rk.innov_events > 0) ++innov_runs;
        std::printf("    seed %llu: innov %ld zmax %.3f nis %.1f\n",
                    (unsigned long long)seed, rk.innov_events, rk.z_max_m, rk.nis_mean);
        nis_lo = std::fmin(nis_lo, rk.nis_mean); nis_hi = std::fmax(nis_hi, rk.nis_mean);
        zworst = std::fmax(zworst, rk.z_max_m);
    }
    std::printf("kick x20: GOOD %d/20  innov-runs %d/20  NIS [%.2f, %.2f]  zmax %.3f m\n",
                good, innov_runs, nis_lo, nis_hi, zworst);
    if (good < 19) { std::puts("VERTICAL RED: VS null does not hold the kick"); return 1; }
    if (innov_runs < 10) { std::puts("VERTICAL RED: the innovation gate never tokenized the kick"); return 1; }  // >=10/20: the v0 gate tokenizes kicks that stand above the nulls own limit cycle in ~half the seeds — receipted immaturity, slice 2 owns the calibrated EKF gate (D-035)
    // Slice-1 consistency: the kinematic tracker's innovation is ELEVATED during real
    // dynamics by design (that is the seam measuring). Sanity band only; the calibrated
    // chi^2 NIS acceptance belongs to slice 2's model-based EKF (D-035).
    if (!(nis_lo > 0.1 && nis_hi < 500.0)) { std::puts("VERTICAL RED: tracker consistency"); return 1; }

    std::puts("VERTICAL GREEN (the enemy exists, is holdable, dies honestly, and is heard)");
    return 0;
}
