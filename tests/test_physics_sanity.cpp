// tests/test_physics_sanity.cpp — the pre-registered expectations, checked in ctest.
// (a) Bosch-Hale at 10 keV must reproduce the canonical 1.13e-22 m^3/s.
// (b) FUSOR-1's reference point must yield Q in the machine.toml-stated band
//     (radiation-honest, conduction-convention — D-032).
#include "core/physics_tier0.h"
#include <cstdio>
#include <cmath>

using namespace fusion;

int main(int argc, char** argv) {
    const std::string root = argc > 1 ? argv[1] : ".";
    const MachineCfg m = load_machine(root + "/contracts/machine.toml");

    const double sv10 = bosch_hale_dt(10.0);
    std::printf("bosch_hale(10 keV) = %.4e m^3/s\n", sv10);
    if (sv10 < 1.05e-22 || sv10 > 1.22e-22) { std::puts("SANITY RED: Bosch-Hale"); return 1; }

    State s{};
    s.ne = m.nbar_e20 * 1e20;
    s.W = 3.0 * s.ne * m.V * m.T_avg_keV * E_KEV_J;
    s.nHe = 0.02 * s.ne;
    s.nimp = m.imp_seed_frac * s.ne;
    s.Ip = m.Ip_MA * 1e6;
    s.Paux = 0; s.Pal = 0; s.Sgas = 0; s.psi = 0;
    Derived d = derive(s, m, m.H98);
    s.Pal = d.Palpha_inst;                       // alpha lag at equilibrium
    d = derive(s, m, m.H98);
    const double P_aux_req = d.Pcond + d.Prad - s.Pal - d.Pohm;   // dW/dt = 0
    const double Q = d.Pfus / std::max(P_aux_req, 0.5e6);
    std::printf("reference: T=%.2f keV  Pfus=%.1f MW  Pcond=%.1f  Prad=%.1f (br %.1f ln %.1f sy %.1f)\n"
                "           Palpha=%.1f  Pohm=%.1f  -> Paux_req=%.1f MW  Q=%.3f  tauE=%.3f s\n",
                d.T, d.Pfus / 1e6, d.Pcond / 1e6, d.Prad / 1e6, d.Pbr / 1e6, d.Pline / 1e6,
                d.Psyn / 1e6, s.Pal / 1e6, d.Pohm / 1e6, P_aux_req / 1e6, Q, d.tauE_s);
    if (Q < 1.4 || Q > 2.8) { std::puts("SANITY RED: reference Q outside [1.4, 2.8]"); return 1; }
    if (d.tauE_s < 0.3 || d.tauE_s > 1.0) { std::puts("SANITY RED: tauE"); return 1; }
    std::puts("PHYSICS SANITY GREEN");
    return 0;
}
