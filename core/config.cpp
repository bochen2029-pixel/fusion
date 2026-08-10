// core/config.cpp — the only TU that includes toml.hpp (tomlplusplus v3.4.0, vendored).
#include "config.h"
#include <toml.hpp>
#include <stdexcept>

namespace fusion {

uint32_t fnv1a32(const std::string& s) {
    uint32_t h = 2166136261u;
    for (unsigned char c : s) { h ^= c; h *= 16777619u; }
    return h;
}

static double num(const toml::table& t, const char* a, const char* b, double dflt) {
    if (auto n = t[a][b].value<double>()) return *n;
    if (auto i = t[a][b].value<int64_t>()) return double(*i);
    return dflt;
}

MachineCfg load_machine(const std::string& path) {
    toml::table t = toml::parse_file(path);
    MachineCfg m;
    m.R0 = num(t, "geometry", "R0_m", 1.85);
    m.a = num(t, "geometry", "a_m", 0.57);
    m.kappa_sep = num(t, "geometry", "kappa_sep", 1.85);
    m.kappa_a = num(t, "geometry", "kappa_area", 1.65);
    m.delta = num(t, "geometry", "delta", 0.45);
    m.V = num(t, "geometry", "volume_m3", 19.6);
    m.Sp = num(t, "geometry", "xsection_m2", 1.68);
    m.B0 = num(t, "field_current", "B0_T", 12.0);
    m.Ip_MA = num(t, "field_current", "Ip_MA", 8.5);
    m.nbar_e20 = num(t, "operating_point", "n_bar_e20", 3.0);
    m.T_avg_keV = num(t, "operating_point", "T_avg_keV", 8.0);
    m.H98 = num(t, "operating_point", "H98", 1.0);
    m.Zeff_ref = num(t, "operating_point", "Zeff", 1.8);
    m.M_amu = num(t, "operating_point", "M_amu", 2.5);
    m.troyon_C = num(t, "limits", "troyon_C", 2.8);
    m.nGW_e20 = num(t, "limits", "greenwald_e20", 8.33);
    m.q95_hard = num(t, "limits", "q95_hard", 2.0);
    m.nmin_e20 = num(t, "limits", "n_min_e20", 0.3);
    m.wall_refl = num(t, "radiation", "wall_reflectivity", 0.8);
    m.imp_seed_frac = num(t, "radiation", "impurity_seed_frac", 1e-4);
    m.E_alpha_MeV = num(t, "alphas", "E_alpha_MeV", 3.52);
    m.pfus_over_pa = num(t, "alphas", "P_fus_over_P_alpha", 5.03);
    m.f_conf = num(t, "alphas", "confined_fraction", 0.92);
    m.P_aux_max_MW = num(t, "heating", "P_aux_max_MW", 60.0);
    m.tau_act_s = num(t, "heating", "tau_act_s", 0.2);
    m.slew_MW_per_s = num(t, "heating", "slew_MW_per_s", 40.0);
    m.S_gas_max_e20 = num(t, "heating", "S_gas_max_e20_per_s", 8.0);
    m.tau_gas_s = num(t, "heating", "tau_gas_s", 0.1);
    m.T_postTQ_keV = num(t, "disruption", "T_post_TQ_keV", 0.015);
    m.tauTQ_ms = num(t, "disruption", "tau_TQ_ms", 0.5);
    m.tauCQ_floor_ms = num(t, "disruption", "tau_CQ_floor_ms", 3.0);
    m.Lp_uH = num(t, "disruption", "L_p_uH", 4.0);
    m.init_Ip_MA = num(t, "initial_state", "Ip_MA", 0.5);
    m.init_T_keV = num(t, "initial_state", "T_keV", 0.3);
    m.init_n_e20 = num(t, "initial_state", "n_e20", 0.3);
    m.ip_ramp_max = num(t, "rates", "ip_ramp_max_MA_per_s", 1.0);
    m.n_ramp_max = num(t, "rates", "n_ramp_max_e20_per_s", 0.15);
    m.b_over_a = num(t, "vessel", "b_over_a", 1.35);
    m.tau_wall_s = num(t, "vessel", "tau_wall_ms", 25.0) * 1e-3;
    m.kappa_shell = num(t, "vessel", "kappa_shell", 1.5);
    if (auto v = t["vessel"]["n_passive_filaments"].value<int64_t>()) m.n_passive = int(*v);
    m.wall_over_a = num(t, "vessel", "wall_over_a", 1.2);
    // coil geometry arrays (D-039; fixed [coils] order, all four same length)
    {
        auto arr = [&](const char* key, double* dst) {
            if (auto* a = t["coils"][key].as_array()) {
                int i = 0;
                for (auto& e : *a) {
                    if (i >= MachineCfg::NCOIL) break;
                    if (auto v = e.value<double>()) dst[i] = *v;
                    else if (auto w = e.value<int64_t>()) dst[i] = double(*w);
                    ++i;
                }
                if (i != MachineCfg::NCOIL)
                    throw std::runtime_error(std::string("machine.toml [coils].") + key +
                                             ": expected 12 entries");
            } else throw std::runtime_error(std::string("machine.toml [coils].") + key +
                                            " missing");
        };
        arr("r_m", m.coil_r); arr("z_m", m.coil_z); arr("half_h_m", m.coil_hh);
        arr("turns", m.coil_turns);
        arr("i_max_kA", m.coil_imax_At);               // conductor kA (D-039 correction)
        for (int i = 0; i < MachineCfg::NCOIL; ++i)
            m.coil_imax_At[i] *= 1e3 * m.coil_turns[i];   // -> circuit ampere-turns
        if (m.coil_r[11] <= 0 || m.coil_z[11] <= 0 || m.coil_turns[11] <= 0)
            throw std::runtime_error("machine.toml [coils]: VS1 geometry/turns must be positive");
    }
    if (m.V <= 0 || m.nGW_e20 <= 0) throw std::runtime_error("machine.toml: bad config");
    if (m.n_passive != 24)   // the tier-1 model's compile-time filament count (vertical.h)
        throw std::runtime_error("machine.toml: n_passive_filaments != 24 (model pin)");
    return m;
}

ObjectiveCfg load_objective(const std::string& path) {
    toml::table t = toml::parse_file(path);
    ObjectiveCfg o;
    o.disrupt_cost = num(t, "terminal", "disrupt_cost", 10000.0);
    o.spine_cost = num(t, "terminal", "spine_shutdown_cost", 6000.0);
    o.timeout_cost = num(t, "terminal", "timeout_cost", 0.0);
    o.q_weight = num(t, "tracking", "q_setpoint", 1.0);
    o.effort_weight = num(t, "actuators", "effort", 0.01);
    o.gate_miss_cost = num(t, "gate", "miss_cost", 500.0);
    o.minq_shortfall_w = num(t, "gate", "minq_shortfall_weight", 2000.0);
    return o;
}

FloorsCfg load_floors(const std::string& path) {
    toml::table t = toml::parse_file(path);
    FloorsCfg f;
    if (auto v = t["plasma"]["greenwald_max_frac"]["value"].value<double>()) f.greenwald_max_frac = *v;
    if (auto v = t["plasma"]["n_min_e20"]["value"].value<double>())          f.nmin_e20 = *v;
    if (auto v = t["rates"]["ip_ramp_max_MA_per_s"]["value"].value<double>()) f.ip_ramp = *v;
    if (auto v = t["rates"]["n_ramp_max_e20_per_s"]["value"].value<double>()) f.n_ramp = *v;
    return f;
}

GatesCfg load_gates(const std::string& path) {
    toml::table t = toml::parse_file(path);
    GatesCfg g;
    if (auto v = t["quench_precursor"]["dwell_ticks"].value<int64_t>()) g.qp_dwell_ticks = int(*v);
    g.z_trip_m = num(t, "vde_detected", "z_trip_m", 0.12);
    g.zdot_trip = num(t, "vde_detected", "zdot_trip_m_per_s", 2.0);
    if (auto v = t["vde_detected"]["dwell_ticks"].value<int64_t>()) g.vde_dwell_ticks = int(*v);
    // frad_trip 1.0 and the T-halving-in-2ms term mirror the [quench_precursor].predicate
    // prose in spine_gates.toml; M1 promotes them to typed keys.
    return g;
}

InnovCfg load_innov(const std::string& path) {
    toml::table t = toml::parse_file(path);
    InnovCfg c;
    const double win_ms = num(t, "innovation", "aggregation_window_ms", 250.0);
    c.window_ticks = (long)(win_ms * 1e-3 / 1e-4 + 0.5);
    if (auto v = t["innovation"]["sigma_token"]["vertical"].value<double>())
        c.sigma_token_vertical = *v;
    if (auto v = t["innovation"]["sigma_alarm"]["vertical"].value<double>())
        c.sigma_alarm_vertical = *v;
    if (auto v = t["innovation"]["dwell_windows"].value<int64_t>()) c.dwell_windows = int(*v);
    const double refr_ms = num(t, "innovation", "refractory_ms", 1000.0);
    c.refractory_ticks = (long)(refr_ms * 1e-3 / 1e-4 + 0.5);
    return c;
}

DispersionsCfg load_dispersions(const std::string& path) {
    toml::table t = toml::parse_file(path);
    DispersionsCfg d;
    d.ip_frac = num(t, "initial_jitter", "Ip_frac", 0.02);
    d.n_frac = num(t, "initial_jitter", "n_frac", 0.05);
    d.T_frac = num(t, "initial_jitter", "T_frac", 0.05);
    d.z0_mm = num(t, "initial_jitter", "z0_mm", 3.0);
    d.H98_sig = num(t, "plant_scatter", "H98", 0.05);
    d.tauw_frac = num(t, "plant_scatter", "tau_wall_frac", 0.05);
    d.coilR_frac = num(t, "plant_scatter", "coil_R_frac", 0.03);
    d.actlag_frac = num(t, "plant_scatter", "actuator_lag_frac", 0.10);
    if (auto v = t["streams"]["offsets"]["plant"].value<int64_t>()) d.stream_plant = uint32_t(*v);
    return d;
}

ScenarioCfg load_scenario(const std::string& path) {
    toml::table t = toml::parse_file(path);
    ScenarioCfg s;
    s.name = t["meta"]["name"].value_or(std::string("unnamed"));
    s.cls = t["meta"]["class"].value_or(std::string("curriculum"));
    s.duration_s = num(t, "phases", "duration_s", 30.0);
    s.Ip_MA = num(t, "initial", "Ip_MA", 8.5);
    s.n_e20 = num(t, "initial", "n_bar_e20", 3.0);
    s.T_keV = num(t, "initial", "T_keV", 8.0);
    s.T_set_keV = num(t, "control", "T_set_keV", 8.0);
    s.n_set_e20 = num(t, "control", "n_set_e20", 3.0);
    s.q_report_set = num(t, "control", "q_report_set", 1.7);
    s.q_min = num(t, "pass", "q_min", 1.0);
    s.hold_s = num(t, "pass", "hold_s", 10.0);
    if (auto v = t["pass"]["seed_count"].value<int64_t>()) s.seed_count = int(*v);
    s.pass_frac = num(t, "pass", "pass_frac", 0.90);
    if (auto arr = t["disturbances"]["events"].as_array()) {
        for (auto& e : *arr) {
            if (auto* et = e.as_table()) {
                const std::string ty = (*et)["type"].value_or(std::string(""));
                if (ty == "impurity_puff") {
                    s.puff = true;
                    s.puff_mag = (*et)["mag"].value_or(0.003);
                    s.puff_t_lo = (*et)["t_lo"].value_or(8.0);
                    s.puff_t_hi = (*et)["t_hi"].value_or(20.0);
                }
                if (ty == "vde_kick") {
                    s.vde_kick = true;
                    s.kick_mm = (*et)["mag_mm"].value_or(25.0);
                    s.kick_t_lo = (*et)["t_lo"].value_or(5.0);
                    s.kick_t_hi = (*et)["t_hi"].value_or(5.0);
                }
            }
        }
    }
    s.vert_on = t["vertical"]["enabled"].value_or(false);
    s.vs_on = t["vertical"]["vs_on"].value_or(true);
    if (t.contains("ramp")) {                        // D-041 schema addition
        s.ramp = true;
        s.ramp_ip_end_MA = num(t, "ramp", "ip_end_MA", s.Ip_MA);
        s.ramp_t0 = num(t, "ramp", "t_start_s", 0.0);
        s.ramp_t1 = num(t, "ramp", "t_end_s", 0.0);
        if (s.ramp_t1 <= s.ramp_t0)
            throw std::runtime_error("scenario [ramp]: t_end_s must exceed t_start_s");
    }
    s.scenario_id = fnv1a32(s.name);
    return s;
}

GainsCfg load_gains(const std::string& path) {
    toml::table t = toml::parse_file(path);
    GainsCfg g;
    g.Kp = num(t, "pid", "Kp", 6.0);   g.Ki = num(t, "pid", "Ki", 3.0);
    g.Kd = num(t, "pid", "Kd", 1.5);
    g.Kpn = num(t, "pid", "Kpn", 2.0); g.Kin = num(t, "pid", "Kin", 1.0);
    g.P_ff_MW = num(t, "pid", "P_ff_MW", 33.0);
    g.S_ff_e20 = num(t, "pid", "S_ff_e20", 2.5);
    g.Krad = num(t, "pid", "Krad", 0.9);
    g.Kpz = num(t, "vs", "Kpz", 5.0e4);
    g.Kdz = num(t, "vs", "Kdz", 200.0);
    g.Kivs = num(t, "vs", "Kivs", 0.0);
    g.vs_truth = t["vs"]["truth"].value_or(false);   // DEV-ONLY (CTL-11 forbids shipping)
    return g;
}

} // namespace fusion
