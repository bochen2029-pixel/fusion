// control/policy.h — the POLICY RUNTIME behind the fence (CTL-14b, D-043): the
// shipping null controller (CEM-tuned burn PIDs + rad feedforward + the VS PD with
// derivative-filtered rate feedback). Links against control/obs.h ONLY — the
// statecheck ctest proves this TU's include closure cannot reach the true state.
// The math is the S10-S14 null verbatim (bit-identical refactor; goldens prove it).
#pragma once
#include "obs.h"
#include "net.h"            // NetMLP — the residual-on-LQG policy net (fence-legal)
#include "core/config.h"    // GainsCfg + machine constants (contracts — no state)

namespace fusion {

// M2 S19 (D-049): the net's vertical observation — 6 features, 3-frame stack (CTL-18).
// Feature order + scales are the trainer contract (export_net.py must match). RESID_BOUND
// is the z-reference offset authority (relay Q15: a z-offset, NOT a raw voltage residual —
// the LQG converts it to the phase-lead voltage the regime near the rail needs).
struct NetVertCfg {
    static constexpr int NFEAT = 6, NFRAME = 3, NIN = NFEAT * NFRAME;   // = 18
    static constexpr double Z_OFF_BOUND = 0.010;   // +/- 10 mm z-reference offset
    // per-feature input scales (net likes O(1) inputs): z*100, v*1, q_s*1e-3, i_vs*1e-2,
    // k_dest/2.5e7, innov*1. Documented; the trainer normalizes identically.
};

struct PolicyState {
    // burn side
    double pidT_I = 0.0, pidT_eprev = 0.0; bool pidT_primed = false;
    double pidN_I = 0.0, pidN_eprev = 0.0; bool pidN_primed = false;
    double prad_avg_W = 0.0;         // 5 s EMA — the rad-ff baseline (D-033)
    double paux_cmd_prev_W = 0.0;
    // vertical side
    double fv_lp = 0.0;              // 60 Hz derivative filter state (D-040)
    // M2 S19: the 3-frame observation ring for the net (newest last); primed flag so the
    // first two ticks replicate the first frame (no zero-padding transient).
    float  obs_hist[NetVertCfg::NIN] = {0};
    bool   obs_primed = false;
};

// 1 kHz: returns the actuator commands. dt_s is the control period (1 ms).
BurnCmd policy_burn(const BurnObs& o, const GainsCfg& k, const MachineCfg& m,
                    PolicyState& st, double dt_s);

// 10 kHz: returns the VS voltage command [V], clamped to the supply. `net` is the
// residual-on-LQG policy (nullptr => the pure null; the ship-runtime-before-train seam).
double policy_vs(const VertObs& o, const GainsCfg& k, PolicyState& st,
                 const NetMLP* net = nullptr);

} // namespace fusion
