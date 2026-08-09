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
#pragma pack(pop)

struct RunResult {
    Verdict verdict = Verdict::GOOD;
    bool pass = false;              // easy-class criterion: survived AND held Q in window
    double minQ_window = 0.0;       // min Q over [t_puff, t_puff+hold_s]
    double q_rms = 0.0;             // vs q_report_set over [5 s, end], GOOD portion
    double effort = 0.0;            // mean (Paux/Pmax)^2 (objective actuator term)
    double t_end = 0.0, t_puff = -1.0;
    double H98_drawn = 1.0;
    uint64_t fnv = 0;               // FNV-1a64 over the golden byte stream
    std::vector<GoldenRec> golden;  // filled when record=true
};

struct SimInputs {
    MachineCfg m; FloorsCfg f; GatesCfg g; DispersionsCfg d; ScenarioCfg s; GainsCfg k;
};

RunResult run_sim(const SimInputs& in, uint64_t seed, bool record);
uint64_t  fnv1a64(const void* data, size_t n, uint64_t h = 14695981039346656037ull);

} // namespace fusion
