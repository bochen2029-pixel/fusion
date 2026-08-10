// core/sim.cpp — the run loop. State changes ONLY here, through the integrator.
// Determinism: fixed dt, fixed iteration counts, no wall-clock, single thread per run,
// Philox draws at init only (M0). Same (scenario, seed) => bit-identical trajectory.
// M2 S18 (D-048): the loop body lives in SimEnv::step() (core/sim_env.h) — run_sim is
// the thin wrapper. The extraction is bit-identical (golden fnv is the proof).
#include "sim.h"
#include "sim_env.h"
#include "philox.h"
#include "vertical.h"
#include "gs.h"
#include "gs_free.h"
#include "transport.h"
#include "control/policy.h"
#include <cstdio>
#include <cmath>
#include <algorithm>

namespace fusion {

uint64_t fnv1a64(const void* data, size_t n, uint64_t h) {
    const unsigned char* p = static_cast<const unsigned char*>(data);
    for (size_t i = 0; i < n; ++i) { h ^= p[i]; h *= 1099511628211ull; }
    return h;
}

void SimEnv::reset(const SimInputs& in_, uint64_t seed_, bool record_) {
    in = in_; seed = seed_; record = record_;
    const MachineCfg& m = in.m; const ScenarioCfg& sc = in.s; const GainsCfg& k = in.k;
    r = RunResult{};
    tick = 0; terminated = false;

    // ---- Draws (fixed order & block layout — documented; layout changes = D-entry) ----
    const uint32_t st = in.d.stream_plant, id = sc.scenario_id;
    const Gauss2 b0 = gauss2(seed, st, id, 0);   // Ip_jit, n_jit
    const Gauss2 b1 = gauss2(seed, st, id, 1);   // T_jit, z0 (z0 unused tier-0)
    const Gauss2 b2 = gauss2(seed, st, id, 2);   // H98 scatter (blind), tau_wall (unused)
    const Gauss2 b3 = gauss2(seed, st, id, 3);   // coil_R (unused), actuator-lag scatter
    const Draw2  b4 = draw2(seed, st, id, 4);    // puff time u, puff mag u (mag fixed in easy)
    const Draw2  b5 = draw2(seed, st, id, 5);    // hl_backtransition time u, target u (D-042
                                                 // — new block APPENDED; layout stable)
    H98 = std::clamp(m.H98 * (1.0 + in.d.H98_sig * b2.g0), 0.6, 1.4);
    t_hl = sc.hl ? (sc.hl_t_lo + (sc.hl_t_hi - sc.hl_t_lo) * b5.u0) : -1.0;
    hl_target = in.d.hl_lo + (in.d.hl_hi - in.d.hl_lo) * b5.u1;
    tau_act = std::clamp(m.tau_act_s * (1.0 + in.d.actlag_frac * b3.g1), 0.05, 1.0);
    r.H98_drawn = H98;
    t_puff = sc.puff ? (sc.puff_t_lo + (sc.puff_t_hi - sc.puff_t_lo) * b4.u0) : -1.0;
    r.t_puff = t_puff;
    (void)b4.u1;

    // ---- Initial state (jittered) ----
    s = State{};
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

    // ---- D-042: the tier-1 1-D burn (20-node T(rho), n(rho); transport.h) ----------
    t1 = Tier1Burn{};
    t1.init(m, T0, s.ne);
    hl_H98 = H98;
    hl_fired = false;
    {   // mirrors + the frozen ECE cross-calibration from the initialized profiles
        double W0 = 0, nb0 = 0, Tv0 = 0;
        for (int j = 0; j < Tier1Burn::NRHO; ++j) {
            W0 += 3.0 * t1.n[j] * t1.T[j] * E_KEV_J * t1.Vj(j);
            nb0 += t1.n[j] * t1.Vj(j);
            Tv0 += t1.T[j] * t1.Vj(j);
        }
        s.W = W0; s.ne = nb0 / m.V;
        T_obs_prev = 0.5 * (t1.T[1] + t1.T[2]);
        n_obs_prev = s.ne;
        ece_cal = std::clamp(T_obs_prev / std::max(Tv0 / m.V, 1e-3), 1.0, 3.0);
    }

    // ---- Loop setup ----
    n_ticks = uint64_t(sc.duration_s / DT_TICK + 0.5);
    puff_tick = (t_puff > 0) ? uint64_t(t_puff / DT_TICK + 0.5) : ~0ull;
    dt_pid = SUBCYCLE * DT_TICK;      // 1 kHz control
    c = Cmds{ s.Paux, s.Sgas };
    d = derive(s, m, H98);
    pst = PolicyState{};
    pst.paux_cmd_prev_W = s.Paux;
    pst.prad_avg_W = d.Prad;                     // 5 s EMA — the rad-ff baseline (D-033)
    prad_obs_prev = d.Prad;                      // D-046: bolometer latch (1 sub-cycle)

    mode = RUN;
    frad_dwell = 0; nGW_dwell = 0; tq_ticks_left = 0;
    W_preTQ = 0; W_postTQ = 0; tauCQ = 0;
    for (double& x : Thist) x = T0;
    th_i = 0;

    hold_lo = (t_puff > 0) ? t_puff : 5.0;
    hold_hi = hold_lo + sc.hold_s;
    minQ = 1e30; q_se = 0.0; eff = 0.0; q_n = 0; eff_n = 0;
    sat_since = ~0ull; sat_reported = false;
    ev(0, EvKind::PhaseStart, 0, sc.Ip_MA, 0.0);

    // ---- M1: the vertical channel (D-035 · D-038 · D-040) --------------------------
    vmod = VerticalModel{}; vekf = VerticalEKF{}; vig = InnovGate{};
    for (double& x : xvert) x = 0.0;
    probe_bias = 0.0; vde_dwell = 0; vde_gate_fired = false;
    kick_tick = ~0ull;
    zlat[0] = zlat[1] = 0.0; ilat[0] = 0.0;
    Vcmd_applied = 0.0; i_meas_prev = 0.0;
    z_sq_sum = 0.0; z_sq_n = 0;
    z_int_acc = 0.0; z_peak_acc = 0.0; v_eff_sum = 0.0; v_eff_n = 0;
    margin_run_min = 1e30;
    ip_obs_ema = 0.0;
    icfg = in.ic;                     // events.toml [innovation], loaded by the caller
    fctx.reset();
    ftrack = GsFreeSolver::FreeTrack{};
    vnom = VerticalModel{};
    vd_now = VertDerived{}; vd_pend = VertDerived{};
    pipe_apply = ~0ull;
    Ip_solve_ref = 0.0; z_solve_ref = 0.0; Ip_prev_solve = 0.0;
    alpha_prev_solve = 1.0; bp_prev_solve = 1.0; bp_ref = -1.0;
    reval_latched = false;
    if (sc.vert_on) {
        fctx = in.fctx ? in.fctx : gs_free_context(m);
        fctx->solver.seed_track(fctx->seed, ftrack);
        vd_now = fctx->solver.derive_at_axis(ftrack.st, fctx->ref.eq.Ip_A);
        Ip_solve_ref = fctx->ref.eq.Ip_A; Ip_prev_solve = fctx->ref.eq.Ip_A;
        // D-040: the plant is built with the drawn plant_scatter truths (tau_wall,
        // actuated-circuit R — b2.g1/b3.g0, drawn-reserved since M0, now consumed);
        // the EKF inits from the NOMINAL build. That gap IS the D-023 information set.
        const double tw_sc = std::clamp(1.0 + in.d.tauw_frac * b2.g1, 0.80, 1.20);
        const double cr_sc = std::clamp(1.0 + in.d.coilR_frac * b3.g0, 0.90, 1.10);
        vmod.build(m, vd_now.k_dest_Npm, tw_sc, cr_sc);    // the scattered truth
        r.gamma_wall = vmod.gamma_wall; r.gamma_open = vmod.gamma_open;
        xvert[0] = in.d.z0_mm * 1e-3 * b1.g1;              // the drawn z0 jitter, at last
        z_solve_ref = xvert[0];
        ip_obs_ema = s.Ip;                                 // D-046: observer Ip seed = truth
        vnom.build(m, vd_now.k_dest_Npm);
        vekf.init(vnom, 0.75e-3, 20.0);       // magnetics 4-probe avg; coil 0.2% of 10 kA
        zlat[0] = zlat[1] = xvert[0]; ilat[0] = 0.0;
        if (sc.vde_kick) {
            const double kt = sc.kick_t_lo + (sc.kick_t_hi - sc.kick_t_lo) * b4.u1;
            kick_tick = uint64_t(kt / DT_TICK + 0.5);      // b4.u1: drawn-reserved, now used
        }
    }

    trc = (in.trace_path && sc.vert_on) ? std::fopen(in.trace_path, "wb") : nullptr;
    if (trc) std::fprintf(trc, "t\tz\tv\txz\txv\tnu\tS\tIvs\txI\tVcmd\tkd_p\tkd_o\tIp\n");
}

// One tick. Returns true while more ticks remain; false when done (terminated or past
// n_ticks). The loop-terminal breaks of the original return false mid-body (skipping
// this tick's scoring + golden, exactly as the `break` did). Bit-identical by fnv.
bool SimEnv::step() {
    if (terminated || tick > n_ticks) return false;
    const MachineCfg& m = in.m; const ScenarioCfg& sc = in.s; const GainsCfg& k = in.k;
    const double t = double(tick) * DT_TICK;

    if (tick == puff_tick && mode == RUN) s.nimp += sc.puff_mag * s.ne;  // the event

    // ---- vertical channel, per tick (10 kHz control + EKF + innovation gate) ----
    if (sc.vert_on && mode == RUN) {
        // D-046: the observer's rogowski Ip — noised off the TRUTH s.Ip, EMA-smoothed
        // (diagnostics.toml [rogowski]). Drawn every tick (stream 3, own counter key)
        // so the EMA is well-defined; consumed only at the observer relink (below).
        {
            const Gauss2 gr = gauss2(seed, 3u, sc.scenario_id ^ 0x0F0517C0u,
                                    uint32_t(tick & 0xFFFFFFFFu));
            const double ip_rogo = s.Ip * (1.0 + ROGO_SIGMA * gr.g0);
            const double ROGO_EMA_A = 1.0 - std::exp(-DT_TICK / (10.0e-3));   // tau 10 ms
            ip_obs_ema += ROGO_EMA_A * (ip_rogo - ip_obs_ema);
        }
        // ---- the D-024 pipeline (D-041): solve@T applies@T+50 (200 Hz slots) ----
        {
            // APPLY first (same-tick sequencing: a solve dispatched at T applies at
            // T+50 BEFORE the T+50 request check — else the cadence silently halves)
            if (tick == pipe_apply) {
                pipe_apply = ~0ull;
                // observer re-linearization at apply; x and P CARRY (D-023 clause).
                // D-046: the observer's Ip is the ROGOWSKI channel; the plant retunes
                // with truth s.Ip. k_dest stays the tracking-solve (rtEFIT) value.
                vnom.retune(ip_obs_ema, vd_pend.k_dest_Npm);
                vekf.relink(vnom);
                r.k_dest_end = vd_pend.k_dest_Npm;
            }
            const bool slot = (tick % 50) == 0;
            const bool revalidate = std::fabs(xvert[0] - z_solve_ref) > in.reval_bound_m;
            const bool inflight = pipe_apply != ~0ull;
            if (slot || revalidate) {
                if (inflight) {
                    if (revalidate && !reval_latched) {
                        ev(tick, EvKind::GsLate, 0, xvert[0], z_solve_ref);
                        ++r.gs_late;
                        reval_latched = true;
                    }
                } else {
                    // D-042: the PROFILE STATE also triggers/parameterizes solves —
                    // alpha and the burn's beta_p enter the GS source.
                    const double Bpa = 1.25663706212e-6 * s.Ip /
                        (2.0 * 3.14159265358979 * m.a *
                         std::sqrt((1.0 + m.kappa_a * m.kappa_a) / 2.0));
                    const double bp_now = 2.0 * 1.25663706212e-6 *
                        ((2.0 / 3.0) * s.W / m.V) / std::max(Bpa * Bpa, 1e-12);
                    if (bp_ref <= 0) bp_ref = bp_now;
                    const double bp_scale = std::clamp(bp_now / bp_ref, 0.5, 2.0);
                    const bool moved = std::fabs(s.Ip - Ip_prev_solve) >
                                       1e-4 * std::fabs(Ip_prev_solve) ||
                                       std::fabs(t1.alpha_prof - alpha_prev_solve) > 5e-3 ||
                                       std::fabs(bp_scale - bp_prev_solve) > 5e-3;
                    if (moved || revalidate) {
                        vd_pend = fctx->solver.track_step(m, ftrack, s.Ip,
                                                          t1.alpha_prof, bp_scale);
                        Ip_prev_solve = s.Ip;
                        alpha_prev_solve = t1.alpha_prof;
                        bp_prev_solve = bp_scale;
                        ++r.gs_solves;
                    } else vd_pend = vd_now;           // stationary: reuse (identical)
                    pipe_apply = tick + 50;
                    z_solve_ref = xvert[0];
                    reval_latched = false;
                    vd_now = vd_pend;
                    Ip_solve_ref = s.Ip;
                }
            }
            // per-tick plant retune: linear-in-Ip anchor to the latest solve (frozen coil)
            const double scale = (Ip_solve_ref > 1.0) ? s.Ip / Ip_solve_ref : 1.0;
            vmod.retune(s.Ip, vd_now.k_dest_Npm * scale);
            // D-047: track the vertical stability margin (kwall/k_dest) — the
            // lethal-legal spine.
            if (vmod.k_dest > 1.0) {
                const double marg = vmod.kwall_ / vmod.k_dest;
                if (margin_run_min > marg) margin_run_min = marg;
            }
        }
        if (tick == kick_tick) vmod.kick_state(xvert, sc.kick_mm * 1e-3);   // D-039
        // observer-in-the-loop (CTL-11/21): the VS PD feeds back the EKF ESTIMATE
        double Vcmd = 0.0;
        if (sc.vs_on) {
            // D-045: the LQG taps (EKF q_s/I_vs estimates) + the OBSERVER'S applied k_dest.
            // D-049: + the calibrated innovation nu/sqrt(S) (last update — 1-tick lag) as
            // the net's contingency feature; in.net (nullptr => pure null) is the residual.
            const double innorm = vekf.innov_last /
                                  std::sqrt(vekf.S_last > 1e-30 ? vekf.S_last : 1e-30);
            const VertObs vo{ k.vs_truth ? xvert[0] : vekf.xz,
                              k.vs_truth ? xvert[1] : vekf.xv,
                              i_meas_prev,
                              vekf.x[2], vekf.x[3],
                              vnom.k_dest,
                              innorm };
            Vcmd = policy_vs(vo, k, pst, in.net);
        }
        vmod.step_rk4(xvert, Vcmd, DT_TICK);
        Vcmd_applied = Vcmd;
        r.z_max_m = std::max(r.z_max_m, std::fabs(xvert[0]));
        // D-045 metrics law: z_rms over the settled window; D-044c smoothness; VS effort.
        if (t >= 2.0) { z_sq_sum += xvert[0] * xvert[0]; ++z_sq_n; }
        if (tick >= kick_tick && double(tick - kick_tick) * DT_TICK < 2.0) {
            z_int_acc += std::fabs(xvert[0]) * DT_TICK;
            z_peak_acc = std::max(z_peak_acc, std::fabs(xvert[0]));
        }
        v_eff_sum += (Vcmd_applied / 2000.0) * (Vcmd_applied / 2000.0); ++v_eff_n;
        // synthetic diagnostics (diagnostics.toml): noise + bias walk + LATENCY
        const Gauss2 gn = gauss2(seed, 3u, sc.scenario_id, uint32_t(tick & 0xFFFFFFFFu));
        const Gauss2 gi = gauss2(seed, 3u, sc.scenario_id ^ 0x5F375A86u,
                                 uint32_t(tick & 0xFFFFFFFFu));   // coil-current chan
        probe_bias += 3.0e-6 * gn.g1;                  // 0.001 FS/s walk, FS 0.3 m
        const double z_meas = zlat[tick % 2] + probe_bias + 0.75e-3 * gn.g0;
        const double i_meas = ilat[0] + 20.0 * gi.g0;
        zlat[tick % 2] = xvert[0];                     // 2-tick ring
        ilat[0] = xvert[2 + VerticalModel::NP];        // 1-tick line
        i_meas_prev = i_meas;
        vekf.predict(Vcmd_applied, DT_TICK);           // the controller's own output
        vekf.update(z_meas);
        vekf.update_i(i_meas);
        const double mag = vig.feed(vekf.innov_last, vekf.S_last, icfg.window_ticks,
                                    icfg.sigma_token_vertical, icfg.sigma_alarm_vertical,
                                    icfg.dwell_windows, icfg.refractory_ticks);
        if (mag > 0) { ev(tick, EvKind::Innov, 0, mag, xvert[0]); ++r.innov_events; }
        if (trc && (tick % 5 == 0))
            std::fprintf(trc, "%.4f\t%.6g\t%.6g\t%.6g\t%.6g\t%.6g\t%.6g\t%.6g\t%.6g"
                              "\t%.6g\t%.6g\t%.6g\t%.6g\n",
                         t, xvert[0], xvert[1], vekf.xz, vekf.xv, vekf.innov_last,
                         vekf.S_last, xvert[2 + VerticalModel::NP], vekf.x[3],
                         Vcmd_applied, vmod.k_dest, vnom.k_dest, s.Ip);
        // spine gate [vde_detected]: fires, receipts, kills heating
        if (std::fabs(xvert[0]) > in.g.z_trip_m && std::fabs(xvert[1]) > in.g.zdot_trip)
            ++vde_dwell; else vde_dwell = 0;
        if (vde_dwell >= in.g.vde_dwell_ticks && !vde_gate_fired) {
            vde_gate_fired = true;
            ev(tick, EvKind::GateFire, 1, xvert[0], xvert[1]);
        }
        // wall contact -> the shared terminal path (TQ cause=vde)
        if (std::fabs(xvert[0]) > 0.19) {
            mode = TQ; W_preTQ = s.W;
            W_postTQ = 3.0 * s.ne * m.V * m.T_postTQ_keV * E_KEV_J;
            tq_ticks_left = int(m.tauTQ_ms * 1e-3 / DT_TICK + 0.5);
            ev(tick, EvKind::TerminalTQ, 2, xvert[0], d.T);
        }
    }

    if (mode == RUN && tick % SUBCYCLE == 0) {
        d = derive(s, m, hl_H98);
        // ---- D-042: the hl_backtransition timeline ----
        if (sc.hl && t >= t_hl && t < t_hl + sc.hl_dur_s) {
            if (!hl_fired) { hl_fired = true;
                ev(tick, EvKind::HLBack, 0, hl_target, t_hl); }
            hl_H98 = std::min(H98, hl_target);
        } else hl_H98 = H98;
        // ---- D-042: the burn PIDs read DIAGNOSTICS (one-sub-cycle latency, latched) ----
        const Gauss2 gd = gauss2(seed, 3u, sc.scenario_id ^ 0xC3C3C3C3u,
                                 uint32_t((tick / SUBCYCLE) & 0xFFFFFFFFu));
        const double T_obs = (T_obs_prev / ece_cal) * (1.0 + 0.03 * gd.g0);
        const double n_obs = n_obs_prev + 0.03e20 * gd.g1;
        // D-046: the bolometer channel (own Philox key, 1-sub-cycle latch). Krad=0 => inert.
        const Gauss2 gb = gauss2(seed, 3u, sc.scenario_id ^ 0xB0105011u,
                                 uint32_t((tick / SUBCYCLE) & 0xFFFFFFFFu));
        const double prad_obs = prad_obs_prev * (1.0 + 0.05 * gb.g0);
        // ---- the null controller (1 kHz), BEHIND THE FENCE (D-043) ----
        const BurnObs bo{ T_obs, n_obs, prad_obs, s.Paux,
                          sc.T_set_keV, sc.n_set_e20, !vde_gate_fired };
        const BurnCmd bc = policy_burn(bo, k, m, pst, dt_pid);
        c.Paux_cmd = bc.Paux_cmd_W;
        double sg = bc.Sgas_cmd;
        // ---- governor floors (M0-active; D-041: Greenwald tracks the LIVE Ip) ----
        const double nGW_now = m.nGW_e20 * 1e20 * (s.Ip / (m.Ip_MA * 1e6));
        if (s.ne >= in.f.greenwald_max_frac * nGW_now) sg = 0.0;            // refuse up
        if (s.ne <= in.f.nmin_e20 * 1e20) sg = m.S_gas_max_e20 * 1e20;      // refuse down
        c.Sgas_cmd = sg;

        // ---- D-042: the 1-D transport step (implicit, 20 nodes) OWNS T/n ----
        Tier1Burn::StepIO io{};
        io.Pal_W = s.Pal; io.Paux_W = s.Paux; io.Pohm_W = d.Pohm;
        io.Sgas = s.Sgas; io.fDT = d.nDT / std::max(s.ne, 1e17);
        io.nimp_frac = s.nimp / std::max(s.ne, 1e17);
        io.Zeff = d.Zeff; io.H98 = hl_H98;
        io.B0 = m.B0; io.wall_refl = m.wall_refl; io.Vmach = m.V;
        t1.step(m, dt_pid, io);
        if (t1.step_profile(m, dt_pid, d.Zeff, s.Ip))
            ev(tick, EvKind::Sawtooth, 0, t1.q0_rep, t1.alpha_prof);
        d.Prad = io.Prad_W; d.Pheat = io.Pheat_W;
        d.Palpha_inst = io.Palpha_inst_W; d.Pfus = io.Pfus_W;
        d.frad = d.Prad / std::max(d.Pheat, 1e6);

        // ---- RK4 for the 0-D remainder, h = 1 ms (the sub-cycle law) ----
        const double dIp = (sc.ramp && t >= sc.ramp_t0 && t < sc.ramp_t1)
            ? (sc.ramp_ip_end_MA - sc.Ip_MA) * 1e6 / (sc.ramp_t1 - sc.ramp_t0) : 0.0;
        const double h = dt_pid;
        const State k1 = rhs(s, d, c, m, dIp, tau_act);
        State s2 = s;
        s2.W += 0.5 * h * k1.W; s2.ne += 0.5 * h * k1.ne; s2.nHe += 0.5 * h * k1.nHe;
        s2.nimp += 0.5 * h * k1.nimp; s2.psi += 0.5 * h * k1.psi; s2.Pal += 0.5 * h * k1.Pal;
        s2.Ip += 0.5 * h * k1.Ip;
        s2.Paux += 0.5 * h * k1.Paux; s2.Sgas += 0.5 * h * k1.Sgas;
        const State k2 = rhs(s2, derive(s2, m, hl_H98), c, m, dIp, tau_act);
        State s3 = s;
        s3.W += 0.5 * h * k2.W; s3.ne += 0.5 * h * k2.ne; s3.nHe += 0.5 * h * k2.nHe;
        s3.nimp += 0.5 * h * k2.nimp; s3.psi += 0.5 * h * k2.psi; s3.Pal += 0.5 * h * k2.Pal;
        s3.Ip += 0.5 * h * k2.Ip;
        s3.Paux += 0.5 * h * k2.Paux; s3.Sgas += 0.5 * h * k2.Sgas;
        const State k3 = rhs(s3, derive(s3, m, hl_H98), c, m, dIp, tau_act);
        State s4 = s;
        s4.W += h * k3.W; s4.ne += h * k3.ne; s4.nHe += h * k3.nHe;
        s4.nimp += h * k3.nimp; s4.psi += h * k3.psi; s4.Pal += h * k3.Pal;
        s4.Ip += h * k3.Ip;
        s4.Paux += h * k3.Paux; s4.Sgas += h * k3.Sgas;
        const State k4 = rhs(s4, derive(s4, m, hl_H98), c, m, dIp, tau_act);
        const double w = h / 6.0;
        s.W += w * (k1.W + 2 * k2.W + 2 * k3.W + k4.W);
        s.ne += w * (k1.ne + 2 * k2.ne + 2 * k3.ne + k4.ne);
        s.nHe += w * (k1.nHe + 2 * k2.nHe + 2 * k3.nHe + k4.nHe);
        s.nimp += w * (k1.nimp + 2 * k2.nimp + 2 * k3.nimp + k4.nimp);
        s.psi += w * (k1.psi + 2 * k2.psi + 2 * k3.psi + k4.psi);
        s.Ip += w * (k1.Ip + 2 * k2.Ip + 2 * k3.Ip + k4.Ip);
        s.Pal += w * (k1.Pal + 2 * k2.Pal + 2 * k3.Pal + k4.Pal);
        s.Paux += w * (k1.Paux + 2 * k2.Paux + 2 * k3.Paux + k4.Paux);
        s.Sgas += w * (k1.Sgas + 2 * k2.Sgas + 2 * k3.Sgas + k4.Sgas);
        // D-042: the profile mirrors overwrite the discarded 0-D W/ne advance
        s.W = io.W_J; s.ne = io.nbar;
        s.ne = std::max(s.ne, 1e17); s.nHe = std::max(s.nHe, 0.0);
        s.nimp = std::max(s.nimp, 0.0); s.W = std::max(s.W, 1e3);
        d = derive(s, m, hl_H98);
        d.Prad = io.Prad_W; d.Pheat = io.Pheat_W;
        d.Palpha_inst = io.Palpha_inst_W; d.Pfus = io.Pfus_W;
        d.frad = d.Prad / std::max(d.Pheat, 1e6);
        d.Q = d.Pfus / std::max(s.Paux, 0.5e6);
        // diagnostics latch (one-sub-cycle latency; ECE cross-cal frozen at init)
        T_obs_prev = io.T_core; n_obs_prev = io.nbar;
        prad_obs_prev = io.Prad_W;                    // D-046: bolometer latch

        // ---- spine gates + terminal physics (per burn step; dwell in ticks) ----
        const double T_2ms_ago = Thist[th_i];
        const bool halved = d.T < in.g.t_drop_frac * T_2ms_ago;
        if (d.frad > in.g.frad_trip || halved) frad_dwell += SUBCYCLE; else frad_dwell = 0;
        if (frad_dwell >= in.g.qp_dwell_ticks) {
            ev(tick, EvKind::GateFire, 0, d.frad, d.T);
            ev(tick, EvKind::SpineShutdown, 0, d.frad, 0.0);
            r.verdict = Verdict::SPINE_SHUTDOWN; terminated = true;       // CTL-10:
            r.t_end = t; return false;                                    // a FAILURE
        }
        // radiative collapse -> thermal quench
        if (d.T < 0.5) { mode = TQ; W_preTQ = s.W;
            W_postTQ = 3.0 * s.ne * m.V * m.T_postTQ_keV * E_KEV_J;
            tq_ticks_left = int(m.tauTQ_ms * 1e-3 / DT_TICK + 0.5);
            ev(tick, EvKind::TerminalTQ, 0, d.T, d.frad); }
        // ctrl saturation event
        const double Pmax = m.P_aux_max_MW * 1e6;
        const bool at_cap = s.Paux >= 0.999 * Pmax;
        if (at_cap && sat_since == ~0ull) { sat_since = tick; sat_reported = false; }
        if (!at_cap && sat_since != ~0ull) {
            if (sat_reported)
                ev(tick, EvKind::CtrlSatOff, 0, double(tick - sat_since) * DT_TICK, 0.0);
            sat_since = ~0ull;
        }
        if (at_cap && !sat_reported && double(tick - sat_since) * DT_TICK >= 0.5) {
            sat_reported = true;
            ev(tick, EvKind::CtrlSatOn, 0, s.Paux / 1e6, 0.0);
        }
        // density limit
        if (s.ne > 1.05 * m.nGW_e20 * 1e20) nGW_dwell += SUBCYCLE; else nGW_dwell = 0;
        if (nGW_dwell >= 100 && mode == RUN) { mode = TQ; W_preTQ = s.W;
            W_postTQ = 3.0 * s.ne * m.V * m.T_postTQ_keV * E_KEV_J;
            tq_ticks_left = int(m.tauTQ_ms * 1e-3 / DT_TICK + 0.5);
            ev(tick, EvKind::TerminalTQ, 1, d.T, s.ne / (m.nGW_e20 * 1e20)); }
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
            ev(tick, EvKind::TerminalCQ, 0, tauCQ * 1e3, s.Ip / 1e6);
        }
    } else if (mode == CQ) {   // current quench: exponential Ip decay, per-tick
        s.Ip *= std::exp(-DT_TICK / tauCQ);
        if (s.Ip < 0.5e6) {
            ev(tick, EvKind::TerminalDisrupt, 0, t, s.Ip / 1e6);
            r.verdict = Verdict::DISRUPT; terminated = true; r.t_end = t; return false; }
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
    ++tick;
    return tick <= n_ticks;
}

RunResult SimEnv::finish() {
    const ScenarioCfg& sc = in.s;
    if (!terminated) { r.verdict = Verdict::GOOD; r.t_end = sc.duration_s; }
    if (sc.vert_on) r.nis_mean = vekf.nis_mean();
    if (z_sq_n) r.z_rms = std::sqrt(z_sq_sum / double(z_sq_n));
    r.z_int_post = z_int_acc; r.z_peak_post = z_peak_acc;
    if (v_eff_n) r.v_effort = v_eff_sum / double(v_eff_n);
    r.margin_min = (margin_run_min < 1e29) ? margin_run_min : 0.0;   // D-047
    r.minQ_window = (minQ > 1e29) ? 0.0 : minQ;
    r.q_rms = q_n ? std::sqrt(q_se / double(q_n)) : 0.0;
    r.effort = eff_n ? eff / double(eff_n) : 0.0;
    const bool window_complete = (t_puff > 0) && (hold_hi <= sc.duration_s) && !terminated;
    r.pass = (r.verdict == Verdict::GOOD) && window_complete && (r.minQ_window >= sc.q_min);
    if (!sc.puff) r.pass = (r.verdict == Verdict::GOOD) && (r.minQ_window >= sc.q_min);
    if (trc) { std::fclose(trc); trc = nullptr; }
    r.li_end = t1.li_1d; r.q0_end = t1.q0_rep;
    r.alpha_end = t1.alpha_prof; r.sawteeth = t1.sawteeth;
    if (record && !r.golden.empty())
        r.fnv = fnv1a64(r.golden.data(), r.golden.size() * sizeof(GoldenRec));
    return r;
}

// The one-shot wrapper — identical semantics to the pre-S18 loop (golden fnv is proof).
RunResult run_sim(const SimInputs& in, uint64_t seed, bool record) {
    SimEnv e;
    e.reset(in, seed, record);
    while (e.step()) {}
    return e.finish();
}

} // namespace fusion
