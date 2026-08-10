// core/config.h — pre-registered contract tables, loaded once into PODs.
// Single source of truth: contracts/*.toml. Changing a value there = a D-entry.
#pragma once
#include <cstdint>
#include <string>

namespace fusion {

struct MachineCfg {
    // geometry
    double R0 = 0, a = 0, kappa_sep = 0, kappa_a = 0, delta = 0, V = 0, Sp = 0;
    // field/current
    double B0 = 0, Ip_MA = 0;
    // operating point
    double nbar_e20 = 0, T_avg_keV = 0, H98 = 1.0, Zeff_ref = 1.8, M_amu = 2.5;
    // limits
    double troyon_C = 2.8, nGW_e20 = 0, q95_hard = 2.0, nmin_e20 = 0.3;
    // radiation
    double wall_refl = 0.8; double imp_seed_frac = 1e-4;
    // alphas
    double E_alpha_MeV = 3.52, pfus_over_pa = 5.03, f_conf = 0.92;
    // heating actuators (D-032)
    double P_aux_max_MW = 60.0, tau_act_s = 0.2, slew_MW_per_s = 40.0;
    double S_gas_max_e20 = 8.0, tau_gas_s = 0.1; // particle source cap [1e20 m^-3 s^-1]
    // disruption
    double T_postTQ_keV = 0.015, tauTQ_ms = 0.5, tauCQ_floor_ms = 3.0, Lp_uH = 4.0;
    // initial handoff
    double init_Ip_MA = 0.5, init_T_keV = 0.3, init_n_e20 = 0.3;
    // rates (machine capability)
    double ip_ramp_max = 1.0, n_ramp_max = 0.15;
    // vessel (passive conductor — D-038 promoted these from vertical.h hard-codes)
    double b_over_a = 1.35, tau_wall_s = 0.025, kappa_shell = 1.5;
    int    n_passive = 24;
    double wall_over_a = 1.2;       // first-wall contour scale (limiter surface; D-039)
    // coil geometry (D-039: the 12 circuits, [coils] order; VS1 row = the anti-series
    // pair at (r, +-z), also consumed by vertical.h — one source)
    static constexpr int NCOIL = 12;
    double coil_r[NCOIL] = {0}, coil_z[NCOIL] = {0}, coil_hh[NCOIL] = {0};
    double coil_turns[NCOIL] = {0};
    double coil_imax_At[NCOIL] = {0};   // conductor i_max_kA x turns x 1e3 (D-039)
};

// The equilibrium→vertical bridge (the MERGE, D-038): what the vertical channel takes
// from the shape — DERIVED by gs_vertical_derive(), never configured. Pure function of
// the machine config at this slice; Monte-Carlo callers compute once, pass via SimInputs.
struct VertDerived {
    double k_dest_Npm = 0;      // rigid-displacement destabilizing force gradient [N/m]
    double n_decay = 0;         // external-field decay index at the axis (<0 = unstable)
    double Bz_ext_axis_T = 0;   // the maintaining (Shafranov) vertical field [T]
    double R_axis_m = 0;
    bool   set = false;
};

struct FloorsCfg {
    double greenwald_max_frac = 0.95;
    double nmin_e20 = 0.36;
    double ip_ramp = 0.85, n_ramp = 0.12;
};

struct GatesCfg {                    // contracts/spine_gates.toml (M0/M1-active subset)
    int    qp_dwell_ticks = 50;      // quench_precursor dwell
    double frad_trip = 1.0;          // f_rad threshold (predicate)
    double t_drop_frac = 0.5;        // T_e halving over tau_ms
    double t_drop_tau_ms = 2.0;
    double z_trip_m = 0.12;          // [vde_detected] (M1 slice 1)
    double zdot_trip = 2.0;
    int    vde_dwell_ticks = 20;
};

struct InnovCfg {                    // contracts/events.toml [innovation] (M1 slice 1)
    long   window_ticks = 2500;      // aggregation_window_ms / dt
    double sigma_token_vertical = 3.5;
    double sigma_alarm_vertical = 5.0;  // single-window fire, no dwell (D-035)
    int    dwell_windows = 2;
    long   refractory_ticks = 10000; // refractory_ms / dt
};
InnovCfg load_innov(const std::string& events_toml_path);

struct DispersionsCfg {              // contracts/dispersions.toml
    double ip_frac = 0.02, n_frac = 0.05, T_frac = 0.05, z0_mm = 3.0;
    double H98_sig = 0.05, tauw_frac = 0.05, coilR_frac = 0.03, actlag_frac = 0.10;
    uint32_t stream_plant = 0;
};

struct ScenarioCfg {                 // contracts/scenarios/<name>.toml
    std::string name; std::string cls;
    double duration_s = 30.0;
    double Ip_MA = 8.5, n_e20 = 3.0, T_keV = 8.0;   // [initial]
    // [control] (M0 addition, D-032): setpoints the null holds + reporting reference
    double T_set_keV = 8.0, n_set_e20 = 3.0, q_report_set = 1.7;
    // [pass]
    double q_min = 1.0, hold_s = 10.0; int seed_count = 1000; double pass_frac = 0.90;
    // impurity puff event (fixed-mag override per D-032 schema note)
    bool   puff = false; double puff_mag = 0.003; double puff_t_lo = 8.0, puff_t_hi = 20.0;
    // M1 slice 1: the vertical channel ([vertical] block; D-035 schema addition)
    bool   vert_on = false;          // enable the linearized vertical model
    bool   vs_on = true;             // VS feedback loop (false = open-loop = the enemy)
    bool   vde_kick = false; double kick_mm = 25.0, kick_t_lo = 5.0, kick_t_hi = 5.0;
    uint32_t scenario_id = 0;        // fnv1a32 of name — part of the Philox counter
};

struct GainsCfg {                    // control/gains_m0.toml (CEM output, committed)
    double Kp = 6.0, Ki = 3.0, Kd = 1.5;       // T-loop -> P_aux [MW per keV]
    double Kpn = 2.0, Kin = 1.0;               // n-loop -> S_gas [1e20 per 1e20-err]
    double P_ff_MW = 33.0, S_ff_e20 = 2.5;     // feedforwards at the reference point
    double Krad = 0.9;                          // D-033: radiation feedforward — MW of
    // P_aux per MW of measured P_rad excess over its 5 s average (bolometric burn
    // control, standard practice; pure PID plateaued at ~83% on train seeds, receipted)
    // M1 slice 1: the VS vertical loop (D-035)
    double Kpz = 5.0e4, Kdz = 200.0;            // V per m / V per (m/s) — sweep-found:
    // Kd through the 10 ms actuator lag destabilizes the screened mode above ~1e3
    double Kivs = 0.0;                          // V per A of I_vs — droop/actuator-state
    // feedback (D-039): speeds the VS lag pole (tau' = L/(R+Kivs)); the classical fix
    // for the phase crunch once the amended vessel put gamma^-1 at ~20 ms against the
    // 10 ms actuator. Still a classical null (PD + droop); LQ remains M2's baseline.
    bool   vs_truth = false;                    // DEV-ONLY calibration switch: PD on true
    // state instead of the EKF estimate. Never true in a committed scenario/gains file;
    // the M1-full statecheck fence will make the shipping build unable to compile it.
};

struct ObjectiveCfg {                // contracts/objective.toml (M0-consumed subset)
    double disrupt_cost = 10000.0, spine_cost = 6000.0, timeout_cost = 0.0;
    double q_weight = 1.0, effort_weight = 0.01;
    double gate_miss_cost = 500.0, minq_shortfall_w = 2000.0;   // D-032(h)
};

MachineCfg     load_machine(const std::string& path);
ObjectiveCfg   load_objective(const std::string& path);
FloorsCfg      load_floors(const std::string& path);
GatesCfg       load_gates(const std::string& path);
DispersionsCfg load_dispersions(const std::string& path);
ScenarioCfg    load_scenario(const std::string& path);
GainsCfg       load_gains(const std::string& path);

uint32_t fnv1a32(const std::string& s);

} // namespace fusion
