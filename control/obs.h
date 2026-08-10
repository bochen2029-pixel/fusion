// control/obs.h — THE FENCE (CTL-14b, D-043). The observation frame the policy runtime
// is allowed to see: diagnostics-derived values, estimator outputs, commanded/scheduled
// quantities, machine constants, and spine SIGNALS. Nothing here may expose the
// true-state struct; the policy TU includes THIS header first, and the true-state
// headers (physics_tier0.h, sim.h) carry a poison guard that fails the build if they
// are ever pulled into a fence TU. The `statecheck` ctest walks the include closure
// and token-scans control/ — the mechanical backstop.
#pragma once
#define FUSION_OBS_FENCE 1

namespace fusion {

// one 1 kHz burn-control observation (built from ECE/interferometer/bolometer lanes;
// latching and cross-calibration happen fence-side in PolicyState)
struct BurnObs {
    double T_meas_keV;      // ECE core chord, cross-calibrated (constant from init cal)
    double n_meas;          // interferometer line/volume average [m^-3]
    double prad_meas_W;     // bolometric sum (diagnostics.toml [bolometer]: noisy +
                            // 1-sub-cycle latency since M2 S17a/D-046; the null runs
                            // Krad=0 so it is honest-but-inert here — a rad-using
                            // controller eats the noise)
    double paux_state_W;    // the actuator's own reported state (drive electronics)
    double T_set_keV, n_set_e20;
    bool   heat_permit;     // spine signal: false after a vde gate fired
};

// one 10 kHz vertical-control observation
struct VertObs {
    double z_est, v_est;    // the EKF's estimates (the estimator is diagnostics-fed)
    double i_vs_meas_A;     // coil_sensors channel (one-tick latency line)
    // M2 slice 1 (D-045): the LQG taps. All estimator outputs (fence-legal by the
    // header's own charter); certainty equivalence wants the ESTIMATES, not raw
    // channels.
    double q_s_est = 0.0;   // EKF screening-mode estimate (cs-scaled coordinate)
    double i_vs_est_A = 0.0;// EKF actuated-circuit-current estimate
    double k_dest_sched = 0.0; // the OBSERVER'S applied k_dest [N/m] — the rtEFIT-class
    // published value the LQ schedule keys on. STATED SIMPLIFICATION (D-045 per
    // D-041/D-042): the equilibrium solve behind it consumes plant-side beta_p/alpha
    // ("reconstruction assumed good"); routed through this frame struct so the
    // statecheck fence sees it — never a side channel.
};

struct BurnCmd { double Paux_cmd_W; double Sgas_cmd; };

} // namespace fusion
