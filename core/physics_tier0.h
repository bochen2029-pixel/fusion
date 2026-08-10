// core/physics_tier0.h — the 0-D burn, spec v0.2 §2.1 tier-0. Pure functions, no state.
// Conventions (PHY-09, D-032): tau_E(IPB98(y,2)) covers CONDUCTION; P_cond solves
// P = W/tau_E(P) in closed form (P^0.31 = W/(C*H98) with P in MW); radiation is a
// separate channel: dW/dt = P_heat - P_rad - P_cond. kappa = kappa_area. n in the fit
// is line-averaged in 1e19 m^-3. All receipts state this convention.
#pragma once
#include "config.h"
#include <cmath>
#include <algorithm>

namespace fusion {

constexpr double E_KEV_J = 1.602176634e-16;   // J per keV
constexpr double DT_TICK = 1.0e-4;            // the 100 us tick (compile-time law)
constexpr int    SUBCYCLE = 10;               // burn cadence 1 ms (compile-time law)

// Bosch & Hale, Nucl. Fusion 32 (1992) 611 — D-T <sigma v>, Maxwellian, T in keV,
// returns m^3/s. Validity 0.2–100 keV (asserted at init; clamped-to-zero below 0.2
// during quench transients, where it is physically negligible anyway).
inline double bosch_hale_dt(double T) {
    if (T < 0.2) return 0.0;
    const double BG = 34.3827, mrc2 = 1.124656e6;
    const double C1 = 1.17302e-9, C2 = 1.51361e-2, C3 = 7.51886e-2, C4 = 4.60643e-3,
                 C5 = 1.35e-2, C6 = -1.0675e-4, C7 = 1.366e-5;
    const double num = T * (C2 + T * (C4 + T * C6));
    const double den = 1.0 + T * (C3 + T * (C5 + T * C7));
    const double theta = T / (1.0 - num / den);
    const double xi = std::cbrt(BG * BG / (4.0 * theta));
    const double sv_cm3 = C1 * theta * std::sqrt(xi / (mrc2 * T * T * T)) * std::exp(-3.0 * xi);
    return sv_cm3 * 1.0e-6;
}

// Ar coronal cooling curve L_z(T_e) [W m^3] — scaling-grade log-log table
// (Mavrin-2018-flavored shape: peak near 0.2 keV, minimum near 10 keV). D-032 pins it;
// the receipt cites it as scaling-grade.
inline double lz_ar(double T) {
    static const double Tk[8] = { 0.1, 0.2, 0.5, 1.0, 2.0, 5.0, 10.0, 30.0 };
    static const double Lz[8] = { 8e-32, 1.8e-31, 1.0e-31, 3.5e-32, 1.4e-32, 6e-33, 4e-33, 3.5e-33 };
    const double t = std::clamp(T, 0.1, 30.0);
    int i = 0; while (i < 6 && Tk[i + 1] < t) ++i;
    const double f = (std::log(t) - std::log(Tk[i])) / (std::log(Tk[i + 1]) - std::log(Tk[i]));
    return std::exp(std::log(Lz[i]) + f * (std::log(Lz[i + 1]) - std::log(Lz[i])));
}

struct State {          // the integrator owns this; nothing else writes it (booster law)
    double W;           // thermal stored energy [J]
    double ne;          // electron density [m^-3]
    double nHe;         // helium ash [m^-3]
    double nimp;        // Ar impurity [m^-3]
    double psi;         // consumed volt-seconds [Wb]
    double Pal;         // slowing-down-lagged alpha heating [W]
    double Paux;        // actuator state: auxiliary heating [W]
    double Sgas;        // actuator state: particle source [m^-3 s^-1]
    double Ip;          // plasma current [A]
};

struct Cmds { double Paux_cmd; double Sgas_cmd; };

struct Derived {
    double T, nDT, Zeff, sv;
    double Pfus, Palpha_inst, Pbr, Pline, Psyn, Pohm, Prad, Pheat, Pcond, tauE_s, Q;
    double frad, loop_V;
};

inline double ipb98_coeff(const MachineCfg& m, double Ip_MA, double n19) {
    // 0.0562 * Ip^0.93 B^0.15 n19^0.41 R^1.97 kappa_a^0.78 eps^0.58 M^0.19  [s, P in MW]
    const double eps = m.a / m.R0;
    return 0.0562 * std::pow(Ip_MA, 0.93) * std::pow(m.B0, 0.15) * std::pow(n19, 0.41)
         * std::pow(m.R0, 1.97) * std::pow(m.kappa_a, 0.78) * std::pow(eps, 0.58)
         * std::pow(m.M_amu, 0.19);
}

inline Derived derive(const State& s, const MachineCfg& m, double H98) {
    Derived d{};
    const double ne = std::max(s.ne, 1e17);
    d.T = std::max(s.W / (3.0 * ne * m.V * E_KEV_J), 1e-3);
    const double Zimp = 16.0;                      // Ar effective charge at core T (D-032)
    d.nDT = std::max(ne - 2.0 * s.nHe - Zimp * s.nimp, 1e17);
    d.Zeff = std::clamp((d.nDT + 4.0 * s.nHe + Zimp * Zimp * s.nimp) / ne + 0.5, 1.0, 6.0);
    d.sv = bosch_hale_dt(d.T);
    const double Rdt = 0.25 * d.nDT * d.nDT * d.sv;            // reactions / m^3 s
    const double Ealpha_J = m.E_alpha_MeV * 1e3 * E_KEV_J;
    d.Palpha_inst = Rdt * Ealpha_J * m.V * m.f_conf;
    d.Pfus = Rdt * Ealpha_J * m.pfus_over_pa * m.V;
    const double n20 = ne / 1e20;
    d.Pbr = 5.35e3 * d.Zeff * n20 * n20 * std::sqrt(d.T) * m.V;
    d.Pline = ne * s.nimp * lz_ar(d.T) * m.V;
    d.Psyn = 29.0 * std::pow(m.B0, 2.5) * std::sqrt(n20) * std::pow(d.T, 2.5)
           * std::sqrt(std::max(1.0 - m.wall_refl, 0.0)) * (m.V / 19.6);
    const double eta = 2.0 * 2.8e-8 * d.Zeff / std::pow(std::max(d.T, 0.01), 1.5); // neo x2
    const double Rp = eta * 2.0 * m.R0 / (m.a * m.a * m.kappa_a);
    d.Pohm = s.Ip * s.Ip * Rp;
    d.loop_V = s.Ip * Rp;
    d.Prad = d.Pbr + d.Pline + d.Psyn;
    d.Pheat = s.Pal + s.Paux + d.Pohm;
    const double n19 = ne / 1e19;
    const double C = ipb98_coeff(m, std::max(s.Ip, 1e5) / 1e6, std::max(n19, 0.1)) * H98;
    const double W_MJ = std::max(s.W, 1e3) / 1e6;
    const double Pcond_MW = std::pow(W_MJ / C, 1.0 / 0.31);    // closed form (see header)
    d.Pcond = Pcond_MW * 1e6;
    d.tauE_s = std::max(s.W / std::max(d.Pcond, 1e5), 1e-3);
    d.frad = d.Prad / std::max(d.Pheat, 1e6);
    d.Q = d.Pfus / std::max(s.Paux, 0.5e6);                    // reporting floor 0.5 MW
    return d;
}

// RHS of the burn ODEs (actuator commands held constant across a step).
inline State rhs(const State& s, const Derived& d, const Cmds& c, const MachineCfg& m,
                 double dIp_dt,
                 double tau_act_eff) {
    State ds{};
    ds.W = d.Pheat - d.Prad - d.Pcond;
    const double tau_p = std::clamp(2.0 * d.tauE_s, 0.05, 10.0);
    ds.ne = s.Sgas - s.ne / tau_p;
    const double Rdt = 0.25 * d.nDT * d.nDT * d.sv;
    ds.nHe = Rdt - s.nHe / std::clamp(5.0 * d.tauE_s, 0.1, 30.0);
    const double nimp_seed = m.imp_seed_frac * s.ne;
    ds.nimp = -(s.nimp - nimp_seed) / std::clamp(3.0 * d.tauE_s, 0.1, 20.0);
    ds.psi = d.loop_V;
    const double tau_s = std::clamp(0.053 * std::pow(d.T, 1.5) / std::max(s.ne / 1e20, 0.05), 0.05, 2.0);
    ds.Pal = (d.Palpha_inst - s.Pal) / tau_s;
    ds.Paux = (c.Paux_cmd - s.Paux) / tau_act_eff;
    ds.Sgas = (c.Sgas_cmd - s.Sgas) / m.tau_gas_s;
    ds.Ip = dIp_dt;                                // scheduled [ramp] rate (D-041); 0 at
                                                   // flat-top — bit-identical to M0 there
    return ds;
}

} // namespace fusion
