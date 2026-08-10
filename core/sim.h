// core/sim.h — one deterministic run: integrator + governor + gates + verdict + golden.
#pragma once
#include "config.h"
#include "physics_tier0.h"
#include <vector>
#include <cstdint>

namespace fusion {

enum class Verdict : uint32_t { GOOD = 0, DISRUPT = 1, SPINE_SHUTDOWN = 2, TIMEOUT = 3 };

#pragma pack(push, 1)
struct GoldenRec {          // fixed-size POD, every 100 ticks (10 ms) — the golden unit
    uint64_t tick;
    double W, ne, nHe, nimp, T, Q, Paux, Ip;
    uint32_t mode, pad;
};

// The event tap (M0.5): fixed-size POD events the plant emits; membrane/ renders text
// AFTER the run (M-M11: no text exists plant-side). Kinds mirror events.toml vocabulary.
enum class EvKind : uint32_t {
    PhaseStart = 0, CtrlSatOn = 1, CtrlSatOff = 2, GateFire = 3,
    TerminalTQ = 4, TerminalCQ = 5, TerminalDisrupt = 6, SpineShutdown = 7,
    Innov = 8,          // [plant] innov mag=Xs cluster=vertical (M1 slice 1)
};
struct EventRec { uint64_t tick; EvKind kind; uint32_t arg; double v0, v1; };
#pragma pack(pop)

struct RunResult {
    Verdict verdict = Verdict::GOOD;
    bool pass = false;              // easy-class criterion: survived AND held Q in window
    double minQ_window = 0.0;       // min Q over [t_puff, t_puff+hold_s]
    double q_rms = 0.0;             // vs q_report_set over [5 s, end], GOOD portion
    double effort = 0.0;            // mean (Paux/Pmax)^2 (objective actuator term)
    double t_end = 0.0, t_puff = -1.0;
    double H98_drawn = 1.0;
    // M1 slice 1 (vertical channel; zeros when [vertical] is off)
    double gamma_wall = 0.0, gamma_open = 0.0;   // REPORTED eigen growth rates [1/s]
    double z_max_m = 0.0;                         // max |Z| over the run
    double nis_mean = 0.0;                        // EKF consistency statistic
    long   innov_events = 0;
    uint64_t fnv = 0;               // FNV-1a64 over the golden byte stream
    std::vector<GoldenRec> golden;  // filled when record=true
    std::vector<EventRec> events;   // filled when record=true (the tap rides the golden flag)
};

struct SimInputs {
    MachineCfg m; FloorsCfg f; GatesCfg g; DispersionsCfg d; ScenarioCfg s; GainsCfg k;
    InnovCfg ic;                    // events.toml [innovation] (M1 slice 1)
    VertDerived vd;                 // equilibrium-derived vertical inputs (D-038).
                                    // Callers that Monte-Carlo vert_on scenarios fill it
                                    // ONCE (gs_vertical_derive); run_sim computes it on
                                    // demand when unset (correct, ~0.5 s — fine for
                                    // single runs, wasteful in loops).
};

RunResult run_sim(const SimInputs& in, uint64_t seed, bool record);
uint64_t  fnv1a64(const void* data, size_t n, uint64_t h = 14695981039346656037ull);

} // namespace fusion
