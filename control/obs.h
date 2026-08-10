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
    double prad_meas_W;     // bolometric sum (tier-1: noise-free channel, stated)
    double paux_state_W;    // the actuator's own reported state (drive electronics)
    double T_set_keV, n_set_e20;
    bool   heat_permit;     // spine signal: false after a vde gate fired
};

// one 10 kHz vertical-control observation
struct VertObs {
    double z_est, v_est;    // the EKF's estimates (the estimator is diagnostics-fed)
    double i_vs_meas_A;     // coil_sensors channel (one-tick latency line)
};

struct BurnCmd { double Paux_cmd_W; double Sgas_cmd; };

} // namespace fusion
