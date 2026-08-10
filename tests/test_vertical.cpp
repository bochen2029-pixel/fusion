// tests/test_vertical.cpp — M1 slice 1 acceptance: the enemy exists, is the right
// CLASS, is holdable, dies honestly when unopposed, and the EKF is consistent.
#include "core/sim.h"
#include "core/vertical.h"
#include "core/gs.h"
#include "core/gs_free.h"
#include "core/philox.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <chrono>
#include <vector>
#include <algorithm>

using namespace fusion;

int main(int argc, char** argv) {
    const std::string root = argc > 1 ? argv[1] : ".";
    const MachineCfg m = load_machine(root + "/contracts/machine.toml");

    // (0) the runtime k_dest source is the FREE-BOUNDARY equilibrium (D-041 rewired
    //     per D-039's pre-registration); the fixed-boundary derive (D-038) remains
    //     the independent cross-check — both printed, ratio band held in test_gs.
    const auto fctx = gs_free_context(m);
    const VertDerived vd = gs_vertical_derive(m);
    std::printf("derived: FREE k_dest %.3e (n %.3f, Bz %.3f T)  |  fixed-bnd cross-check "
                "%.3e (ratio %.2f)\n",
                fctx->ref.k_dest_Npm, fctx->ref.n_decay, fctx->ref.Bz_ext_axis_T,
                vd.k_dest_Npm, fctx->ref.k_dest_Npm / vd.k_dest_Npm);

    // (1) gamma bands — REPORTED from the eigenproblem at the RUNTIME k_dest, the
    // CLASS pre-registered: wall-set 10–100 ms; no-wall < 8 ms.
    VerticalModel vm; vm.build(m, fctx->ref.k_dest_Npm);
    const double gi_wall = 1.0 / vm.gamma_wall, gi_open = 1.0 / vm.gamma_open;
    std::printf("gamma^-1: with passives %.1f ms (open %.3f ms)  k_dest %.3g N/m  "
                "kwall %.3g N/m (margin %.2f)  m_eff %.1f kg  rho_shell %.3g\n",
                gi_wall * 1e3, gi_open * 1e3, vm.k_dest, vm.kwall_,
                vm.kwall_ / vm.k_dest, vm.m_eff, vm.rho_shell);
    // the design premise: the shell CAN hold the shape (kwall > k_dest); if this fails
    // the machine pin itself must be adjudicated (a D-entry, never a silent retune)
    if (!(vm.kwall_ > vm.k_dest)) { std::puts("VERTICAL RED: shell cannot hold the shape"); return 1; }
    // the pre-registered CLASS: wall-set 10–100 ms with the shell; removing the shell
    // makes the mode ≥5× faster (the 20-turn VS still screens passively — reported).
    // D-040 restores the original 5× bound: gamma_open is now the UNDAMPED inertial
    // reference (the artifact damper is a closed-loop regularization device, excluded
    // from the no-shell datum); gamma_wall is the sim's actual slow mode, artifact-
    // damped (+~17% — the regularization's stated, receipted touch on gamma).
    if (!(gi_wall > 0.010 && gi_wall < 0.100)) { std::puts("VERTICAL RED: wall gamma class"); return 1; }
    if (!(gi_open > 0 && gi_open < 0.008 && gi_wall / gi_open >= 5.0)) {
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
    in.vd = vd;                     // the cross-check record (unused by run_sim now)
    in.fctx = fctx;                 // the runtime context, once (D-041)

    // ---- DEV budget mode: test_vertical <root> budget — the §7.4 tick-budget datum
    // (KICKOFF build note: measure the linear step + EKF early, receipt p99.9).
    // Wall-clock lives HERE, outside run_sim — the sim path stays clock-free.
    if (argc > 2 && std::string(argv[2]) == "budget") {
        VerticalModel bm; bm.build(m, vd.k_dest_Npm);
        VerticalEKF be; be.init(bm, 0.75e-3, 20.0);
        double xb[VerticalModel::N] = {0}; xb[0] = 1e-3;
        const int NB = 2000, BATCH = 100;                  // 200k ticks in 100-tick batches
        std::vector<double> ns_per(NB);
        for (int b = 0; b < NB; ++b) {
            const auto t0 = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < BATCH; ++i) {
                const double V = -(5e4 * be.xz + 200.0 * be.xv);
                bm.retune(8.5e6 - (i & 15) * 1e3, vd.k_dest_Npm);   // D-041 per-tick path
                bm.step_rk4(xb, std::clamp(V, -2000.0, 2000.0), 1e-4);
                be.predict(V, 1e-4);
                be.update(xb[0] + 1e-4 * ((i & 7) - 3.5)); // deterministic dither
                be.update_i(xb[2 + VerticalModel::NP]);
            }
            const auto t1 = std::chrono::high_resolution_clock::now();
            ns_per[b] = std::chrono::duration<double, std::nano>(t1 - t0).count() / BATCH;
            if (xb[0] > 0.15) { xb[0] = 1e-3; for (int i = 1; i < VerticalModel::N; ++i) xb[i] = 0; }
        }
        std::sort(ns_per.begin(), ns_per.end());
        std::printf("tick budget (vertical step + EKF, batch-of-%d medians): "
                    "p50 %.0f ns  p99 %.0f ns  p99.9 %.0f ns  (100 us tick)\n",
                    BATCH, ns_per[NB / 2], ns_per[int(NB * 0.99)], ns_per[NB - 2]);
        return 0;
    }

    // ---- DEV trace mode: test_vertical <root> trace <seed> <out.tsv> — replicates
    // the sim's vertical block (same draws) and dumps the filter's view per tick.
    if (argc > 4 && std::string(argv[2]) == "trace") {
        const uint64_t seed = std::strtoull(argv[3], nullptr, 10);
        VerticalModel pm; pm.build(m, vd.k_dest_Npm, 1.0, 1.0);   // nominal plant here
        VerticalEKF fe; fe.init(pm, 0.75e-3, 20.0);
        double xs[VerticalModel::N] = {0}; xs[0] = 3e-3;
        double zl[2] = {xs[0], xs[0]}, il = 0, bias = 0, Vap = 0, fvlp = 0;
        const double LPA = 1.0 - std::exp(-2.0 * 3.14159265358979 * 60.0 * 1e-4);
        FILE* f = std::fopen(argv[4], "wb");
        std::fprintf(f, "t\tz\tv\txz\txv\tnu\tS\tIvs\txI\tVcmd\n");
        const uint32_t sid = fnv1a32("vde_kick");
        for (uint64_t tk = 0; tk < 80000; ++tk) {
            if (tk == 50000) pm.kick_state(xs, 0.060);
            fvlp += LPA * (fe.xv - fvlp);
            const double V = std::clamp(-(5e4 * fe.xz + 200.0 * fvlp), -2000.0, 2000.0);
            pm.step_rk4(xs, V, 1e-4);
            Vap = V;
            const Gauss2 gn = gauss2(seed, 3u, sid, uint32_t(tk));
            const Gauss2 gi = gauss2(seed, 3u, sid ^ 0x5F375A86u, uint32_t(tk));
            bias += 3.0e-6 * gn.g1;
            const double zm = zl[tk % 2] + bias + 0.75e-3 * gn.g0;
            const double im = il + 20.0 * gi.g0;
            zl[tk % 2] = xs[0]; il = xs[2 + VerticalModel::NP];
            fe.predict(Vap, 1e-4);
            fe.update(zm);
            fe.update_i(im);
            if (tk % 5 == 0)
                std::fprintf(f, "%.4f\t%.6g\t%.6g\t%.6g\t%.6g\t%.6g\t%.6g\t%.6g\t%.6g\t%.6g\n",
                             tk * 1e-4, xs[0], xs[1], fe.xz, fe.xv, fe.innov_last,
                             fe.S_last, xs[2 + VerticalModel::NP], fe.x[3], Vap);
        }
        std::fclose(f);
        std::printf("trace done: nis_mean %.2f\n", fe.nis_mean());
        return 0;
    }

    // ---- DEV events mode: test_vertical <root> events <scenario> <gains> <seed...>
    // — dumps the event timeline of chosen seeds (death forensics).
    if (argc > 5 && std::string(argv[2]) == "events") {
        in.s = load_scenario(argv[3]);
        in.k = load_gains(argv[4]);
        in.trace_path = "runs/m1s6/simtrace.tsv";   // DEV forensics (D-041)
        for (int i = 5; i < argc; ++i) {
            const uint64_t seed = std::strtoull(argv[i], nullptr, 10);
            RunResult r = run_sim(in, seed, true);
            std::printf("seed %llu verdict %u t_end %.3f zmax %.3f nis %.2f events:\n",
                        (unsigned long long)seed, unsigned(r.verdict), r.t_end, r.z_max_m,
                        r.nis_mean);
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
    // D-040: the model-based filter tokenizes EVERY kick (was >=10/20 under the v0
    // kinematic-tracker gate — D-035's receipted immaturity, now closed).
    if (innov_runs < 18) { std::puts("VERTICAL RED: the innovation gate missed kicks"); return 1; }
    // D-023's acceptance, live at last: rolling chi^2 NIS within the pre-registered
    // band [0.5, 2.0] across plant-scattered seeds — the events.toml sigma-thresholds
    // only MEAN something while this holds. (Measured 1.27-1.30: scatter + single-mode
    // reduction + contract latencies, honestly carried by the filter's S.)
    if (!(nis_lo > 0.5 && nis_hi < 2.0)) { std::puts("VERTICAL RED: EKF chi^2 NIS acceptance"); return 1; }

    std::puts("VERTICAL GREEN (the enemy exists, is holdable, dies honestly, and is heard)");
    return 0;
}
