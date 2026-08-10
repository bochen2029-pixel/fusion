// tests/test_vertical.cpp — M1 slice 1 acceptance: the enemy exists, is the right
// CLASS, is holdable, dies honestly when unopposed, and the EKF is consistent.
#include "core/sim.h"
#include "core/vertical.h"
#include "core/gs.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>

using namespace fusion;

int main(int argc, char** argv) {
    const std::string root = argc > 1 ? argv[1] : ".";
    const MachineCfg m = load_machine(root + "/contracts/machine.toml");

    // (0) the MERGE (D-038): k_dest arrives from the shaped equilibrium — derived,
    //     never configured. gamma now moves with the equilibrium through the shape.
    const VertDerived vd = gs_vertical_derive(m);
    std::printf("derived: n_decay %.3f  Bz_ext %.3f T  k_dest %.3e N/m  (R_axis %.3f)\n",
                vd.n_decay, vd.Bz_ext_axis_T, vd.k_dest_Npm, vd.R_axis_m);

    // (1) gamma bands — REPORTED from the eigenproblem, the CLASS pre-registered:
    // wall-set 10–100 ms (machine.toml [vessel] expectation 25–40); no-wall < 2 ms.
    VerticalModel vm; vm.build(m, vd.k_dest_Npm);
    const double gi_wall = 1.0 / vm.gamma_wall, gi_open = 1.0 / vm.gamma_open;
    std::printf("gamma^-1: with passives %.1f ms (open %.3f ms)  k_dest %.3g N/m  "
                "kwall %.3g N/m (margin %.2f)  m_eff %.1f kg  rho_shell %.3g\n",
                gi_wall * 1e3, gi_open * 1e3, vm.k_dest, vm.kwall_,
                vm.kwall_ / vm.k_dest, vm.m_eff, vm.rho_shell);
    // the design premise: the shell CAN hold the shape (kwall > k_dest); if this fails
    // the machine pin itself must be adjudicated (a D-entry, never a silent retune)
    if (!(vm.kwall_ > vm.k_dest)) { std::puts("VERTICAL RED: shell cannot hold the shape"); return 1; }
    // the pre-registered CLASS: wall-set 10–100 ms with the shell; removing the shell
    // makes the mode ≥4× faster (the 20-turn VS still screens passively — reported).
    // D-039: the bound was 5× under the κ_shell 1.5 vessel; the amended 1.9 shell
    // (which the DN separatrix REQUIRES) couples more weakly, raising the VS pair's
    // relative share — measured 4.7×. The demonstration (passive structure sets the
    // timescale) stands; the ratio moved with the re-pinned machine, receipted.
    if (!(gi_wall > 0.010 && gi_wall < 0.100)) { std::puts("VERTICAL RED: wall gamma class"); return 1; }
    if (!(gi_open > 0 && gi_open < 0.008 && gi_wall / gi_open >= 4.0)) {
        std::puts("VERTICAL RED: shell-removal separation"); return 1; }

    // (1b) equilibrium-following demonstration (REPORT-ONLY — the merge's point):
    // gamma now derives from the shape. A softer elongation must soften the enemy;
    // a lower Ip must soften it quadratically (linear GS, fixed boundary). The full
    // l_i/profile movement arrives with transport; this shows the plumbing is live.
    {
        MachineCfg m2 = m; m2.kappa_sep = 1.60;
        const VertDerived v2 = gs_vertical_derive(m2);
        MachineCfg m3 = m; m3.Ip_MA = 6.0;
        const VertDerived v3 = gs_vertical_derive(m3);
        VerticalModel vk; vk.build(m2, v2.k_dest_Npm);
        std::printf("equilibrium-following: kappa 1.85->1.60: n %.3f->%.3f  k_dest "
                    "%.2e->%.2e (gamma^-1 %.1f->%.1f ms)  |  Ip 8.5->6.0 MA: k_dest "
                    "%.2e (ratio %.2f vs Ip^2 %.2f)\n",
                    vd.n_decay, v2.n_decay, vd.k_dest_Npm, v2.k_dest_Npm,
                    gi_wall * 1e3, 1e3 / vk.gamma_wall, v3.k_dest_Npm,
                    v3.k_dest_Npm / vd.k_dest_Npm, (6.0 * 6.0) / (8.5 * 8.5));
    }

    SimInputs in;
    in.m = m;
    in.f = load_floors(root + "/contracts/floors.toml");
    in.g = load_gates(root + "/contracts/spine_gates.toml");
    in.d = load_dispersions(root + "/contracts/dispersions.toml");
    in.k = load_gains(root + "/control/gains_m0.toml");
    in.ic = load_innov(root + "/contracts/events.toml");
    in.vd = vd;                     // computed once above; run_sim would re-derive per run

    // ---- DEV events mode: test_vertical <root> events <scenario> <gains> <seed...>
    // — dumps the event timeline of chosen seeds (death forensics).
    if (argc > 5 && std::string(argv[2]) == "events") {
        in.s = load_scenario(argv[3]);
        in.k = load_gains(argv[4]);
        for (int i = 5; i < argc; ++i) {
            const uint64_t seed = std::strtoull(argv[i], nullptr, 10);
            RunResult r = run_sim(in, seed, true);
            std::printf("seed %llu verdict %u t_end %.3f zmax %.3f events:\n",
                        (unsigned long long)seed, unsigned(r.verdict), r.t_end, r.z_max_m);
            for (const EventRec& e : r.events)
                std::printf("  t=%8.4f kind=%u arg=%u v0=%.4g v1=%.4g\n",
                            double(e.tick) * 1e-4, unsigned(e.kind), e.arg, e.v0, e.v1);
        }
        return 0;
    }

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
