// control/policy.h — the POLICY RUNTIME behind the fence (CTL-14b, D-043): the
// shipping null controller (CEM-tuned burn PIDs + rad feedforward + the VS PD with
// derivative-filtered rate feedback). Links against control/obs.h ONLY — the
// statecheck ctest proves this TU's include closure cannot reach the true state.
// The math is the S10-S14 null verbatim (bit-identical refactor; goldens prove it).
#pragma once
#include "obs.h"
#include "core/config.h"    // GainsCfg + machine constants (contracts — no state)

namespace fusion {

struct PolicyState {
    // burn side
    double pidT_I = 0.0, pidT_eprev = 0.0; bool pidT_primed = false;
    double pidN_I = 0.0, pidN_eprev = 0.0; bool pidN_primed = false;
    double prad_avg_W = 0.0;         // 5 s EMA — the rad-ff baseline (D-033)
    double paux_cmd_prev_W = 0.0;
    // vertical side
    double fv_lp = 0.0;              // 60 Hz derivative filter state (D-040)
};

// 1 kHz: returns the actuator commands. dt_s is the control period (1 ms).
BurnCmd policy_burn(const BurnObs& o, const GainsCfg& k, const MachineCfg& m,
                    PolicyState& st, double dt_s);

// 10 kHz: returns the VS voltage command [V], clamped to the supply.
double policy_vs(const VertObs& o, const GainsCfg& k, PolicyState& st);

} // namespace fusion
