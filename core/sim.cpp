// core/sim.cpp — the run loop. State changes ONLY here, through the integrator.
// Determinism: fixed dt, fixed iteration counts, no wall-clock, single thread per run,
// Philox draws at init only (M0). Same (scenario, seed) => bit-identical trajectory.
#include "sim.h"
#include "philox.h"
#include <cmath>
#include <algorithm>

namespace fusion {

uint64_t fnv1a64(const void* data, size_t n, uint64_t h) {
    const unsigned char* p = static_cast<const unsigned char*>(data);
    for (size_t i = 0; i < n; ++i) { h ^= p[i]; h *= 1099511628211ull; }
    return h;
}

namespace {
struct Pid {
    double I = 0.0, e_prev = 0.0; bool primed = false;
    double step(double e, double dt, double Kp, double Ki, double Kd, bool windup_ok) {
        if (windup_ok) I += e * dt;
        const double d = primed ? (e - e_prev) / dt : 0.0;
        e_prev = e; primed = true;
        return Kp * e + Ki * I + Kd * d;
    }
};
} // namespace

RunResult run_sim(const SimInputs& in, uint64_t seed, bool record) {
    const MachineCfg& m = in.m; const ScenarioCfg& sc = in.s; const GainsCfg& k = in.k;
    RunResult r{};

    // ---- Draws (fixed order & block layout — documented; layout changes = D-entry) ----
    const uint32_t st = in.d.stream_plant, id = sc.scenario_id;
    const Gauss2 b0 = gauss2(seed, st, id, 0);   // Ip_jit, n_jit
    const Gauss2 b1 = gauss2(seed, st, id, 1);   // T_jit, z0 (z0 unused tier-0)
    const Gauss2 b2 = gauss2(seed, st, id, 2);   // H98 scatter (blind), tau_wall (unused)
    const Gauss2 b3 = gauss2(seed, st, id, 3);   // coil_R (unused), actuator-lag scatter
    const Draw2  b4 = draw2(seed, st, id, 4);    // puff time u, puff mag u (mag fixed in easy)
    const double H98 = std::clamp(m.H98 * (1.0 + in.d.H98_sig * b2.g0), 0.6, 1.4);
    const double tau_act = std::clamp(m.tau_act_s * (1.0 + in.d.actlag_frac * b3.g1), 0.05, 1.0);
    r.H98_drawn = H98;
    const double t_puff = sc.puff ? (sc.puff_t_lo + (sc.puff_t_hi - sc.puff_t_lo) * b4.u0) : -1.0;
    r.t_puff = t_puff;
    (void)b4.u1;

    // ---- Initial state (jittered) ----
    State s{};
    s.ne = sc.n_e20 * 1e20 * (1.0 + in.d.n_frac * b0.g1);
    const double T0 = sc.T_keV * (1.0 + in.d.T_frac * b1.g0);
    s.W = 3.0 * s.ne * m.V * T0 * E_KEV_J;
    s.nHe = 0.02 * s.ne;
    s.nimp = m.imp_seed_frac * s.ne;
    s.psi = 0.0;
    s.Ip = sc.Ip_MA * 1e6 * (1.0 + in.d.ip_frac * b0.g0);
    s.Paux = k.P_ff_MW * 1e6;
    s.Sgas = k.S_ff_e20 * 1e20;
    {   Derived d0 = derive(s, m, H98);
        s.Pal = d0.Palpha_inst;              // start the lag state at equilibrium
    }

    // ---- Loop ----
    const uint64_t n_ticks = uint64_t(sc.duration_s / DT_TICK + 0.5);
    const uint64_t puff_tick = (t_puff > 0) ? uint64_t(t_puff / DT_TICK + 0.5) : ~0ull;
    const double dt_pid = SUBCYCLE * DT_TICK;      // 1 kHz control
    Pid pidT, pidN;
    Cmds c{ s.Paux, s.Sgas };
    double paux_cmd_prev = s.Paux;
    Derived d = derive(s, m, H98);
    double prad_avg = d.Prad;                    // 5 s EMA — the rad-ff baseline (D-033)

    enum Mode : uint32_t { RUN = 0, TQ = 2, CQ = 3 };
    uint32_t mode = RUN;
    int frad_dwell = 0, nGW_dwell = 0, tq_ticks_left = 0;
    double W_preTQ = 0, W_postTQ = 0, tauCQ = 0;
    // T-history ring for the halving predicate (2 ms = 20 ticks)
    double Thist[20]; for (double& x : Thist) x = T0;
    int th_i = 0;

    const double hold_lo = (t_puff > 0) ? t_puff : 5.0;
    const double hold_hi = hold_lo + sc.hold_s;
    double minQ = 1e30, q_se = 0.0, eff = 0.0; uint64_t q_n = 0, eff_n = 0;
    bool terminated = false;

    for (uint64_t tick = 0; tick <= n_ticks; ++tick) {
        const double t = double(tick) * DT_TICK;

        if (tick == puff_tick && mode == RUN) s.nimp += sc.puff_mag * s.ne;  // the event

        if (mode == RUN && tick % SUBCYCLE == 0) {
            d = derive(s, m, H98);
            // ---- the null controller (1 kHz): T-loop -> P_aux, n-loop -> S_gas ----
            const double Pmax = m.P_aux_max_MW * 1e6;
            const double eT = sc.T_set_keV - d.T;
            const bool ok_T = (s.Paux > 1e5 && s.Paux < Pmax * 0.999) ||
                              (eT > 0) == (s.Paux < Pmax * 0.5);
            // D-033: bolometric radiation feedforward — counter the measured loss NOW,
            // before T moves (the pure-PID near-miss band was exactly this lag)
            prad_avg += (d.Prad - prad_avg) * (dt_pid / 5.0);
            const double rad_ff = k.Krad * std::max(0.0, d.Prad - prad_avg);
            double p = (k.P_ff_MW + pidT.step(eT, dt_pid, k.Kp, k.Ki, k.Kd, ok_T)) * 1e6
                     + rad_ff;
            p = std::clamp(p, 0.0, Pmax);
            const double slew = m.slew_MW_per_s * 1e6 * dt_pid;              // rate floor
            p = std::clamp(p, paux_cmd_prev - slew, paux_cmd_prev + slew);
            paux_cmd_prev = p; c.Paux_cmd = p;
            const double en = sc.n_set_e20 - s.ne / 1e20;
            double sg = (k.S_ff_e20 + pidN.step(en, dt_pid, k.Kpn, k.Kin, 0.0, true)) * 1e20;
            sg = std::clamp(sg, 0.0, m.S_gas_max_e20 * 1e20);
            // ---- governor floors (M0-active) ----
            if (s.ne >= in.f.greenwald_max_frac * m.nGW_e20 * 1e20) sg = 0.0;   // refuse up
            if (s.ne <= in.f.nmin_e20 * 1e20) sg = m.S_gas_max_e20 * 1e20;      // refuse down
            c.Sgas_cmd = sg;

            // ---- RK4 burn step, h = 1 ms (the sub-cycle law) ----
            const double h = dt_pid;
            const State k1 = rhs(s, d, c, m, tau_act);
            State s2 = s;
            s2.W += 0.5 * h * k1.W; s2.ne += 0.5 * h * k1.ne; s2.nHe += 0.5 * h * k1.nHe;
            s2.nimp += 0.5 * h * k1.nimp; s2.psi += 0.5 * h * k1.psi; s2.Pal += 0.5 * h * k1.Pal;
            s2.Paux += 0.5 * h * k1.Paux; s2.Sgas += 0.5 * h * k1.Sgas;
            const State k2 = rhs(s2, derive(s2, m, H98), c, m, tau_act);
            State s3 = s;
            s3.W += 0.5 * h * k2.W; s3.ne += 0.5 * h * k2.ne; s3.nHe += 0.5 * h * k2.nHe;
            s3.nimp += 0.5 * h * k2.nimp; s3.psi += 0.5 * h * k2.psi; s3.Pal += 0.5 * h * k2.Pal;
            s3.Paux += 0.5 * h * k2.Paux; s3.Sgas += 0.5 * h * k2.Sgas;
            const State k3 = rhs(s3, derive(s3, m, H98), c, m, tau_act);
            State s4 = s;
            s4.W += h * k3.W; s4.ne += h * k3.ne; s4.nHe += h * k3.nHe;
            s4.nimp += h * k3.nimp; s4.psi += h * k3.psi; s4.Pal += h * k3.Pal;
            s4.Paux += h * k3.Paux; s4.Sgas += h * k3.Sgas;
            const State k4 = rhs(s4, derive(s4, m, H98), c, m, tau_act);
            const double w = h / 6.0;
            s.W += w * (k1.W + 2 * k2.W + 2 * k3.W + k4.W);
            s.ne += w * (k1.ne + 2 * k2.ne + 2 * k3.ne + k4.ne);
            s.nHe += w * (k1.nHe + 2 * k2.nHe + 2 * k3.nHe + k4.nHe);
            s.nimp += w * (k1.nimp + 2 * k2.nimp + 2 * k3.nimp + k4.nimp);
            s.psi += w * (k1.psi + 2 * k2.psi + 2 * k3.psi + k4.psi);
            s.Pal += w * (k1.Pal + 2 * k2.Pal + 2 * k3.Pal + k4.Pal);
            s.Paux += w * (k1.Paux + 2 * k2.Paux + 2 * k3.Paux + k4.Paux);
            s.Sgas += w * (k1.Sgas + 2 * k2.Sgas + 2 * k3.Sgas + k4.Sgas);
            s.ne = std::max(s.ne, 1e17); s.nHe = std::max(s.nHe, 0.0);
            s.nimp = std::max(s.nimp, 0.0); s.W = std::max(s.W, 1e3);
            d = derive(s, m, H98);

            // ---- spine gates + terminal physics (per burn step; dwell in ticks) ----
            // quench_precursor: f_rad > 1.0 dwell, OR T halves in 2 ms (predicate mirror)
            const double T_2ms_ago = Thist[th_i];
            const bool halved = d.T < in.g.t_drop_frac * T_2ms_ago;
            if (d.frad > in.g.frad_trip || halved) frad_dwell += SUBCYCLE; else frad_dwell = 0;
            if (frad_dwell >= in.g.qp_dwell_ticks) {
                r.verdict = Verdict::SPINE_SHUTDOWN; terminated = true;       // CTL-10:
                r.t_end = t; break;                                           // a FAILURE
            }
            // radiative collapse -> thermal quench
            if (d.T < 0.5) { mode = TQ; W_preTQ = s.W;
                W_postTQ = 3.0 * s.ne * m.V * m.T_postTQ_keV * E_KEV_J;
                tq_ticks_left = int(m.tauTQ_ms * 1e-3 / DT_TICK + 0.5); }
            // density limit
            if (s.ne > 1.05 * m.nGW_e20 * 1e20) nGW_dwell += SUBCYCLE; else nGW_dwell = 0;
            if (nGW_dwell >= 100 && mode == RUN) { mode = TQ; W_preTQ = s.W;
                W_postTQ = 3.0 * s.ne * m.V * m.T_postTQ_keV * E_KEV_J;
                tq_ticks_left = int(m.tauTQ_ms * 1e-3 / DT_TICK + 0.5); }
        }
        Thist[th_i] = d.T; th_i = (th_i + 1) % 20;

        if (mode == TQ) {          // thermal quench: W -> W_post over tau_TQ, per-tick
            const int n0 = int(m.tauTQ_ms * 1e-3 / DT_TICK + 0.5);
            s.W -= (W_preTQ - W_postTQ) / double(n0);
            if (--tq_ticks_left <= 0) {
                s.W = W_postTQ; mode = CQ;
                const double Zeff_q = 2.0;
                const double eta_q = 2.0 * 2.8e-8 * Zeff_q / std::pow(m.T_postTQ_keV, 1.5);
                const double Rp_q = eta_q * 2.0 * m.R0 / (m.a * m.a * m.kappa_a);
                tauCQ = std::max(m.Lp_uH * 1e-6 / Rp_q, m.tauCQ_floor_ms * 1e-3);
            }
        } else if (mode == CQ) {   // current quench: exponential Ip decay, per-tick
            s.Ip *= std::exp(-DT_TICK / tauCQ);
            if (s.Ip < 0.5e6) { r.verdict = Verdict::DISRUPT; terminated = true; r.t_end = t; break; }
        }

        // ---- scoring taps ----
        if (mode == RUN) {
            if (t >= hold_lo && t <= hold_hi) minQ = std::min(minQ, d.Q);
            if (t >= 5.0) { const double e = d.Q - sc.q_report_set; q_se += e * e; ++q_n; }
            const double u = s.Paux / (m.P_aux_max_MW * 1e6); eff += u * u; ++eff_n;
        }
        if (record && tick % 100 == 0) {
            GoldenRec g{ tick, s.W, s.ne, s.nHe, s.nimp, d.T, d.Q, s.Paux, s.Ip, mode, 0u };
            r.golden.push_back(g);
        }
    }

    if (!terminated) { r.verdict = Verdict::GOOD; r.t_end = sc.duration_s; }
    r.minQ_window = (minQ > 1e29) ? 0.0 : minQ;
    r.q_rms = q_n ? std::sqrt(q_se / double(q_n)) : 0.0;
    r.effort = eff_n ? eff / double(eff_n) : 0.0;
    const bool window_complete = (t_puff > 0) && (hold_hi <= sc.duration_s) && !terminated;
    r.pass = (r.verdict == Verdict::GOOD) && window_complete && (r.minQ_window >= sc.q_min);
    if (!sc.puff) r.pass = (r.verdict == Verdict::GOOD) && (r.minQ_window >= sc.q_min);
    if (record && !r.golden.empty())
        r.fnv = fnv1a64(r.golden.data(), r.golden.size() * sizeof(GoldenRec));
    return r;
}

} // namespace fusion
