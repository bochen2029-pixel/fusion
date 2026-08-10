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

double policy_vs(const VertObs& o, const GainsCfg& k, PolicyState& st) {
    // D-040: 60 Hz derivative filter on the rate estimate (the artifact stays out of
    // the drive); the 100 us control period is a fence-side constant by law.
    const double LP_A = 1.0 - std::exp(-2.0 * 3.14159265358979 * 60.0 * 1.0e-4);
    st.fv_lp += LP_A * (o.v_est - st.fv_lp);
    return std::clamp(-(k.Kpz * o.z_est + k.Kdz * st.fv_lp + k.Kivs * o.i_vs_meas_A),
                      -2000.0, 2000.0);
}

} // namespace fusion
