// core/sim_env.h — THE STEPWISE PLANT (M2 S18, D-048). run_sim's one-shot loop body,
// extracted into a struct whose state lives in MEMBERS and whose `step()` advances ONE
// tick. run_sim is now a thin wrapper: reset() → while(step()){} → finish(). The proof
// the extraction is faithful is the golden fnv (bit-identical — the S15 fence pattern).
//
// WHY: PPO needs stepwise envs (inject an action per tick); the ghost needs fork-at-T
// (copy the env, run both forward, memcmp — F-GHOST). Both fall out of a copyable,
// steppable SimEnv. The batched training runner (fusor_train_env) builds N of these
// sharing ONE FreeContext (the relay Q13 landmine: never per-env context builds).
//
// FENCE NOTE: this is CORE-SIDE (the plant). It includes the true-state headers by
// design; it is NEVER in the policy TU's include closure (statecheck unaffected).
//
// COPYABILITY: SimEnv is default-copyable (value members + a shared_ptr fctx + vectors
// in RunResult). The ghost fork is `SimEnv g = plant;`. The dev trace FILE* must be null
// for a forked/batched env (asserted) — tracing is a single-run affordance.
#pragma once
#include "sim.h"            // FIRST — its poison guard passes before obs.h arms the fence
#include "physics_tier0.h"
#include "transport.h"
#include "vertical.h"
#include "gs_free.h"
#include "control/policy.h" // LAST of the project headers: PolicyState (a value member).
                            // obs.h arms FUSION_OBS_FENCE here — safe because every
                            // true-state header above is already included (guards set),
                            // so no later #include can trip the poison. sim_env.h is
                            // core-side (the plant); statecheck only walks the policy TU.
#include <memory>
#include <cstdio>

namespace fusion {

struct SimEnv {
    enum Mode : uint32_t { RUN = 0, TQ = 2, CQ = 3 };

    // ---- inputs (copied; fctx shared by refcount — the relay Q13 shared-context law) --
    SimInputs in;
    uint64_t  seed = 0;
    bool      record = false;

    // ---- init-time constants used across ticks (set in reset) ------------------------
    double   H98 = 1.0, t_hl = -1.0, hl_target = 0.0, tau_act = 0.2, t_puff = -1.0;
    uint64_t n_ticks = 0, puff_tick = ~0ull;
    double   dt_pid = 0.0, hold_lo = 5.0, hold_hi = 0.0;
    InnovCfg icfg;

    // ---- the loop cursor -------------------------------------------------------------
    uint64_t tick = 0;
    bool     terminated = false;

    // ---- burn state ------------------------------------------------------------------
    State      s{};
    Tier1Burn  t1;
    Derived    d{};
    Cmds       c{0, 0};
    PolicyState pst{};
    double  hl_H98 = 1.0;   bool hl_fired = false;
    double  T_obs_prev = 0, n_obs_prev = 0, ece_cal = 1.0, prad_obs_prev = 0;
    uint32_t mode = RUN;
    int     frad_dwell = 0, nGW_dwell = 0, tq_ticks_left = 0;
    double  W_preTQ = 0, W_postTQ = 0, tauCQ = 0;
    double  Thist[20] = {0};   int th_i = 0;
    double  minQ = 1e30, q_se = 0.0, eff = 0.0;   uint64_t q_n = 0, eff_n = 0;
    uint64_t sat_since = ~0ull;   bool sat_reported = false;

    // ---- vertical channel state ------------------------------------------------------
    VerticalModel vmod;   VerticalEKF vekf;   InnovGate vig;
    double  xvert[VerticalModel::N] = {0};
    double  probe_bias = 0.0;   int vde_dwell = 0;   bool vde_gate_fired = false;
    uint64_t kick_tick = ~0ull;
    double  zlat[2] = {0, 0}, ilat[1] = {0};
    double  Vcmd_applied = 0.0, i_meas_prev = 0.0;
    double  z_sq_sum = 0.0;   uint64_t z_sq_n = 0;
    double  z_int_acc = 0.0, z_peak_acc = 0.0, v_eff_sum = 0.0;   uint64_t v_eff_n = 0;
    double  margin_run_min = 1e30;
    double  ip_obs_ema = 0.0;
    static constexpr double ROGO_SIGMA = 0.005;   // diagnostics.toml [rogowski]

    // ---- the D-024 pipeline / free-boundary tracking state ---------------------------
    std::shared_ptr<const FreeContext> fctx;
    GsFreeSolver::FreeTrack ftrack;
    VerticalModel vnom;
    VertDerived vd_now, vd_pend;
    uint64_t pipe_apply = ~0ull;
    double  Ip_solve_ref = 0.0, z_solve_ref = 0.0, Ip_prev_solve = 0.0;
    double  alpha_prev_solve = 1.0, bp_prev_solve = 1.0, bp_ref = -1.0;
    bool    reval_latched = false;

    // ---- output ----------------------------------------------------------------------
    RunResult r{};
    std::FILE* trc = nullptr;   // dev trace; MUST be null for forked/batched envs

    // reset to tick 0 for (in, seed, record). Does every init run_sim did.
    void reset(const SimInputs& in_, uint64_t seed_, bool record_);
    // advance ONE tick. Returns true while more ticks remain; false when done
    // (terminated OR past n_ticks). The loop-terminal breaks return false mid-body.
    bool step();
    // finalize the RunResult (verdict, aggregates, fnv). Idempotent-safe.
    RunResult finish();

    // event tap (records POD only; membrane renders text post-run — M-M11)
    void ev(uint64_t tk, EvKind kk, uint32_t a, double v0, double v1) {
        if (record) r.events.push_back(EventRec{ tk, kk, a, v0, v1 });
    }
};

} // namespace fusion
