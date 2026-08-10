// core/sim.h — one deterministic run: integrator + governor + gates + verdict + golden.
#pragma once
#ifdef FUSION_OBS_FENCE
#error "statecheck fence (CTL-14b): a policy TU pulled in the true-state header"
#endif
#include "config.h"
#include "physics_tier0.h"
#include <vector>
#include <cstdint>
#include <memory>

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
    GsLate = 9,         // D-024/D-041: a revalidation request hit an in-flight solve —
                        // the pipeline could not serve it timely (deterministic
                        // surrogate of wall-late; F-KEEPUP's aggregate-pacing datum)
    Sawtooth = 10,      // D-042: q0 < 1 core crash (state-triggered; PHY-14)
    HLBack = 11,        // D-042: H->L back-transition step (confinement collapse)
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
    double z_rms = 0.0;                           // RMS Z over the settled window
                                                  // (t >= 2 s, RUN mode) — D-045's
                                                  // health-metric law (zmax hid a
                                                  // 62 mm standing wave once; S12)
    double z_int_post = 0.0, z_peak_post = 0.0;   // D-044c smoothness: int |z| dt and
                                                  // peak |z| over [t_evt, t_evt+2 s]
    double v_effort = 0.0;                        // mean (Vcmd/2000)^2 — the VS channel
                                                  // under the objective's [actuators]
                                                  // effort term (D-045)
    double nis_mean = 0.0;                        // EKF consistency statistic
    long   innov_events = 0;
    long   gs_solves = 0, gs_late = 0;           // D-041 pipeline accounting
    double k_dest_end = 0.0;                     // last applied k_dest (ramp receipts)
    double margin_min = 0.0;                     // min vertical stability margin
                                                 // (kwall/k_dest) over the run — the
                                                 // lethal-legal receipt's spine (D-047):
                                                 // a floor-legal ramp walks this toward 1
                                                 // while every command stays legal
    double li_end = 0.0, q0_end = 0.0, alpha_end = 0.0;   // D-042 profile reports
    long   sawteeth = 0;
    uint64_t fnv = 0;               // FNV-1a64 over the golden byte stream
    std::vector<GoldenRec> golden;  // filled when record=true
    std::vector<EventRec> events;   // filled when record=true (the tap rides the golden flag)
};

struct FreeContext;                 // gs_free.h (D-041): tables + converged seed state

struct SimInputs {
    MachineCfg m; FloorsCfg f; GatesCfg g; DispersionsCfg d; ScenarioCfg s; GainsCfg k;
    InnovCfg ic;                    // events.toml [innovation] (M1 slice 1)
    VertDerived vd;                 // fixed-boundary derive (D-038) — since D-041 the
                                    // VERIFICATION cross-check; the runtime k_dest
                                    // source is the free-boundary context below.
    std::shared_ptr<const FreeContext> fctx;   // D-041: built ONCE per process by MC
                                    // callers (gs_free_context); run_sim builds on
                                    // demand when unset (correct, ~2 s — single runs).
    const char* trace_path = nullptr;   // DEV-ONLY: per-tick vertical TSV (forensics;
                                    // file I/O in the loop — never set in gates/MC)
    double reval_bound_m = 0.020;   // D-024 pipeline: |dZ| beyond this forces an early
                                    // re-solve; a request hitting an in-flight solve is
                                    // a gs_late event. Default = the shipped bound (2 cm,
                                    // D-041); the gs_late anger test (D-046) tightens it
                                    // to prove the late path fires and counts
                                    // deterministically. Golden-safe: default unchanged.
};

RunResult run_sim(const SimInputs& in, uint64_t seed, bool record);
uint64_t  fnv1a64(const void* data, size_t n, uint64_t h = 14695981039346656037ull);

} // namespace fusion
