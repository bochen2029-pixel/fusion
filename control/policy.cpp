// control/policy.cpp — the policy runtime TU. THE FENCE IS THIS TU'S INCLUDE CLOSURE:
// obs.h (first — arms the poison) + policy.h + core/config.h. The statecheck ctest
// walks it and token-scans this directory; the true-state headers refuse to coexist
// with the fence macro. Math is the receipted null, verbatim (goldens prove the
// refactor bit-identical).
#include "obs.h"
#include "policy.h"
#include <cmath>
#include <algorithm>

namespace fusion {

static double pid_step(double& I, double& e_prev, bool& primed, double e, double dt,
                       double Kp, double Ki, double Kd, bool windup_ok) {
    if (windup_ok) I += e * dt;
    const double d = primed ? (e - e_prev) / dt : 0.0;
    e_prev = e; primed = true;
    return Kp * e + Ki * I + Kd * d;
}

BurnCmd policy_burn(const BurnObs& o, const GainsCfg& k, const MachineCfg& m,
                    PolicyState& st, double dt_s) {
    const double Pmax = m.P_aux_max_MW * 1e6;
    const double eT = o.T_set_keV - o.T_meas_keV;
    const bool ok_T = (o.paux_state_W > 1e5 && o.paux_state_W < Pmax * 0.999) ||
                      (eT > 0) == (o.paux_state_W < Pmax * 0.5);
    // D-033: bolometric radiation feedforward — counter the measured loss NOW,
    // before T moves (the pure-PID near-miss band was exactly this lag)
    st.prad_avg_W += (o.prad_meas_W - st.prad_avg_W) * (dt_s / 5.0);
    const double rad_ff = k.Krad * std::max(0.0, o.prad_meas_W - st.prad_avg_W);
    double p = (k.P_ff_MW + pid_step(st.pidT_I, st.pidT_eprev, st.pidT_primed,
                                     eT, dt_s, k.Kp, k.Ki, k.Kd, ok_T)) * 1e6
             + rad_ff;
    if (!o.heat_permit) p = 0.0;               // spine response: bound the energy
    p = std::clamp(p, 0.0, Pmax);
    const double slew = m.slew_MW_per_s * 1e6 * dt_s;                    // rate floor
    p = std::clamp(p, st.paux_cmd_prev_W - slew, st.paux_cmd_prev_W + slew);
    st.paux_cmd_prev_W = p;
    const double en = o.n_set_e20 - o.n_meas / 1e20;
    double sg = (k.S_ff_e20 + pid_step(st.pidN_I, st.pidN_eprev, st.pidN_primed,
                                       en, dt_s, k.Kpn, k.Kin, 0.0, true)) * 1e20;
    sg = std::clamp(sg, 0.0, m.S_gas_max_e20 * 1e20);
    return BurnCmd{ p, sg };                   // governor floors apply PLANT-SIDE
}

double policy_vs(const VertObs& o, const GainsCfg& k, PolicyState& st,
                 const NetMLP* net) {
    if (k.lq_on && k.lq_n >= 2) {
        // D-045: the LQG vertical null. u = -K(kd)*xhat on all four EKF states — K
        // linearly interpolated over the k_dest schedule (the observer's applied
        // value, clamped to the grid ends). Replaces the PD+derivative-filter path
        // entirely: the 60 Hz filter was a PD artifact guard, and the LQ design
        // model CONTAINS the damped artifact (pumping absence receipted).
        const int n = k.lq_n;
        int i = 0;
        while (i < n - 2 && o.k_dest_sched > k.lq_kd[i + 1]) ++i;
        const double w = std::clamp((o.k_dest_sched - k.lq_kd[i]) /
                                    (k.lq_kd[i + 1] - k.lq_kd[i]), 0.0, 1.0);
        const double Kz = k.lq_Kz[i] + w * (k.lq_Kz[i + 1] - k.lq_Kz[i]);
        const double Kv = k.lq_Kv[i] + w * (k.lq_Kv[i + 1] - k.lq_Kv[i]);
        const double Kq = k.lq_Kq[i] + w * (k.lq_Kq[i + 1] - k.lq_Kq[i]);
        const double Kk = k.lq_Ki[i] + w * (k.lq_Ki[i + 1] - k.lq_Ki[i]);
        // D-049: the residual-on-LQG net. It outputs a Z-REFERENCE OFFSET (relay Q15 —
        // the LQG converts it to the phase-lead voltage the near-rail regime needs, not a
        // weak raw-voltage residual). nullptr OR zero weights => z_off = 0 => the pure
        // LQG null (bit-identical — the ship-runtime-before-train proof).
        double z_off = 0.0;
        if (net && net->loaded && net->n_in == NetVertCfg::NIN && net->n_out == 1) {
            // the 6 features, scaled to O(1) (the trainer contract — export_net.py matches)
            const float f[NetVertCfg::NFEAT] = {
                float(o.z_est * 100.0), float(o.v_est), float(o.q_s_est * 1e-3),
                float(o.i_vs_est_A * 1e-2), float(o.k_dest_sched / 2.5e7),
                float(o.innov_norm) };
            // shift the 3-frame ring (newest last); prime => fill all frames on tick 0
            const int F = NetVertCfg::NFEAT;
            if (!st.obs_primed) {
                for (int fr = 0; fr < NetVertCfg::NFRAME; ++fr)
                    for (int j = 0; j < F; ++j) st.obs_hist[fr * F + j] = f[j];
                st.obs_primed = true;
            } else {
                for (int j = 0; j < (NetVertCfg::NFRAME - 1) * F; ++j)
                    st.obs_hist[j] = st.obs_hist[j + F];
                for (int j = 0; j < F; ++j)
                    st.obs_hist[(NetVertCfg::NFRAME - 1) * F + j] = f[j];
            }
            float out = 0.0f;
            net->forward(st.obs_hist, &out);
            z_off = std::clamp(double(out), -1.0, 1.0) * NetVertCfg::Z_OFF_BOUND;
        }
        return std::clamp(-(Kz * (o.z_est - z_off) + Kv * o.v_est + Kq * o.q_s_est
                            + Kk * o.i_vs_est_A), -2000.0, 2000.0);
    }
    // D-040: 60 Hz derivative filter on the rate estimate (the artifact stays out of
    // the drive); the 100 us control period is a fence-side constant by law.
    const double LP_A = 1.0 - std::exp(-2.0 * 3.14159265358979 * 60.0 * 1.0e-4);
    st.fv_lp += LP_A * (o.v_est - st.fv_lp);
    return std::clamp(-(k.Kpz * o.z_est + k.Kdz * st.fv_lp + k.Kivs * o.i_vs_meas_A),
                      -2000.0, 2000.0);
}

} // namespace fusion
