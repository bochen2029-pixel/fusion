// core/gs.h — M1 slice 2a: the Grad-Shafranov solver CORE (spec v0.2 §2.1 tier-1).
//
// Scope of this slice (D-036, honesty-labeled):
//   - the Δ* elliptic solver on the 65×65 grid (SOR, fixed sweep count — deterministic;
//     the named DST/cyclic-reduction fast solver is the slice-2b upgrade, same interface)
//   - ANALYTIC VERIFICATION against the Solov'ev solution (Δ*ψ = 8A·R² + 2B has exact
//     ψ = A·R⁴ + B·Z² + homogeneous) — the classic GS-code acceptance test
//   - the Picard loop with linear Solov'ev-class profiles p'(ψ̄), FF'(ψ̄), normalized to
//     the machine's Ip each iteration; reports ψ_axis, magnetic-axis position
//     (the Shafranov-shift-flavored displacement), l_i and β_p integrals
// NOT in this slice (slice 2b): plasma-shaped boundary (κ, δ via free-boundary coils +
// von Hagenow), X-point, the q-profile/q95 (needs flux-surface contours), transport
// coupling. The boundary here is the rectangle with a Solov'ev Dirichlet shell —
// stated wherever the numbers are reported.
#pragma once
#include "config.h"
#include <cmath>
#include <algorithm>
#include <vector>

namespace fusion {

struct GsGrid {
    static constexpr int NR = 65, NZ = 65;
    double Rmin, Rmax, Zmin, Zmax, dR, dZ;
    std::vector<double> psi;                 // NR*NZ, row-major [iz*NR + ir]
    double R(int ir) const { return Rmin + dR * ir; }
    double Z(int iz) const { return Zmin + dZ * iz; }
    double& at(int ir, int iz) { return psi[size_t(iz) * NR + ir]; }
    double  at(int ir, int iz) const { return psi[size_t(iz) * NR + ir]; }

    void init(const MachineCfg& m) {
        Rmin = m.R0 - 1.5 * m.a; Rmax = m.R0 + 1.5 * m.a;
        Zmin = -1.5 * m.a * m.kappa_sep; Zmax = +1.5 * m.a * m.kappa_sep;
        dR = (Rmax - Rmin) / (NR - 1); dZ = (Zmax - Zmin) / (NZ - 1);
        psi.assign(size_t(NR) * NZ, 0.0);
    }
};

// SOR solve of  Δ*ψ = rhs  with Dirichlet boundary already loaded in psi's edge cells.
// Fixed sweep count (deterministic); omega tuned for the 65² operator.
inline void gs_solve(GsGrid& g, const std::vector<double>& rhs, int sweeps = 8000,
                     double omega = 1.85) {
    const double idR2 = 1.0 / (g.dR * g.dR), idZ2 = 1.0 / (g.dZ * g.dZ);
    for (int s = 0; s < sweeps; ++s) {
        for (int iz = 1; iz < GsGrid::NZ - 1; ++iz) {
            for (int ir = 1; ir < GsGrid::NR - 1; ++ir) {
                const double R = g.R(ir);
                const double aE = idR2 - 1.0 / (2.0 * R * g.dR);   // psi[ir+1]
                const double aW = idR2 + 1.0 / (2.0 * R * g.dR);   // psi[ir-1]
                const double diag = -2.0 * idR2 - 2.0 * idZ2;
                const double res = rhs[size_t(iz) * GsGrid::NR + ir]
                    - aE * g.at(ir + 1, iz) - aW * g.at(ir - 1, iz)
                    - idZ2 * (g.at(ir, iz + 1) + g.at(ir, iz - 1))
                    - diag * g.at(ir, iz);
                g.at(ir, iz) += omega * res / diag;
            }
        }
    }
}

// ---- the Solov'ev analytic acceptance test -----------------------------------------
// psi_exact = A R^4 + B Z^2  solves  Δ*psi = 8A R^2 + 2B  (R^2 is homogeneous).
inline double gs_solovev_check(const MachineCfg& m) {
    GsGrid g; g.init(m);
    const double A = 0.02, B = 0.5;
    auto exact = [&](double R, double Z) { return A * R * R * R * R + B * Z * Z; };
    std::vector<double> rhs(g.psi.size());
    for (int iz = 0; iz < GsGrid::NZ; ++iz)
        for (int ir = 0; ir < GsGrid::NR; ++ir) {
            const double R = g.R(ir);
            rhs[size_t(iz) * GsGrid::NR + ir] = 8.0 * A * R * R + 2.0 * B;
            if (ir == 0 || iz == 0 || ir == GsGrid::NR - 1 || iz == GsGrid::NZ - 1)
                g.at(ir, iz) = exact(R, g.Z(iz));
        }
    gs_solve(g, rhs);
    double emax = 0, scale = 0;
    for (int iz = 1; iz < GsGrid::NZ - 1; ++iz)
        for (int ir = 1; ir < GsGrid::NR - 1; ++ir) {
            const double e = std::fabs(g.at(ir, iz) - exact(g.R(ir), g.Z(iz)));
            emax = std::fmax(emax, e);
            scale = std::fmax(scale, std::fabs(exact(g.R(ir), g.Z(iz))));
        }
    return emax / (scale > 0 ? scale : 1.0);         // relative max error
}

// ---- the Picard equilibrium (slice-2a form) ----------------------------------------
struct GsEquilibrium {
    double psi_axis = 0, psi_bnd = 0;
    double R_axis = 0, Z_axis = 0;           // magnetic axis (Shafranov-shifted)
    double Ip_A = 0;                          // achieved plasma current
    double li = 0, beta_p = 0;                // standard integrals (rectangle-shell BC)
    int    picard_iters = 0;
};

inline GsEquilibrium gs_picard(const MachineCfg& m, GsGrid& g, int n_picard = 15) {
    const double mu0 = 1.25663706212e-6;
    const double Ip_target = m.Ip_MA * 1e6;
    // Dirichlet shell: the Solov'ev surface through (R0 ± a, 0) — stated in reports.
    const double A0 = 0.02, B0 = 0.5;
    auto shell = [&](double R, double Z) { return A0 * R * R * R * R + B0 * Z * Z; };
    std::vector<double> rhs(g.psi.size(), 0.0);
    for (int iz = 0; iz < GsGrid::NZ; ++iz)
        for (int ir = 0; ir < GsGrid::NR; ++ir) {
            g.at(ir, iz) = shell(g.R(ir), g.Z(iz));   // initial guess = shell field
        }
    double p0 = 1.0e4, f1 = 1.0;                       // profile scales (re-normalized)
    GsEquilibrium eq;
    for (int it = 0; it < n_picard; ++it) {
        // axis/boundary flux for normalization
        double pmin = 1e300, pmax = -1e300; int ax_ir = 1, ax_iz = 1;
        for (int iz = 1; iz < GsGrid::NZ - 1; ++iz)
            for (int ir = 1; ir < GsGrid::NR - 1; ++ir) {
                const double v = g.at(ir, iz);
                if (v < pmin) { pmin = v; ax_ir = ir; ax_iz = iz; }
                if (v > pmax) pmax = v;
            }
        const double span = (pmax - pmin) > 1e-30 ? (pmax - pmin) : 1e-30;
        // source + current integral with CURRENT profile scales
        double Ip_now = 0;
        for (int iz = 1; iz < GsGrid::NZ - 1; ++iz)
            for (int ir = 1; ir < GsGrid::NR - 1; ++ir) {
                const double R = g.R(ir);
                const double pb = std::clamp((g.at(ir, iz) - pmin) / span, 0.0, 1.0);
                const double w = 1.0 - pb;             // linear Solov'ev-class profiles
                const double pprime = -p0 * w, ffprime = -f1 * w;
                const double Jphi = -(R * pprime + ffprime / (mu0 * R));
                rhs[size_t(iz) * GsGrid::NR + ir] = -mu0 * R * R * pprime - ffprime;
                Ip_now += Jphi * g.dR * g.dZ;
            }
        // rescale both profile amplitudes to hit Ip_target (Picard-stable)
        const double scale = Ip_target / (std::fabs(Ip_now) > 1.0 ? Ip_now : 1.0);
        p0 *= scale; f1 *= scale;
        for (auto& v : rhs) v *= scale;
        gs_solve(g, rhs, 3000);
        eq.psi_axis = pmin; eq.psi_bnd = pmax;
        eq.R_axis = g.R(ax_ir); eq.Z_axis = g.Z(ax_iz);
        eq.Ip_A = Ip_now * scale;
        eq.picard_iters = it + 1;
    }
    // integrals (over the rectangle-shell plasma region; labeled as such)
    const double span = (eq.psi_bnd - eq.psi_axis) > 1e-30 ? eq.psi_bnd - eq.psi_axis : 1e-30;
    double bp2_int = 0, p_int = 0, vol = 0;
    for (int iz = 1; iz < GsGrid::NZ - 1; ++iz)
        for (int ir = 1; ir < GsGrid::NR - 1; ++ir) {
            const double R = g.R(ir);
            const double dpsidR = (g.at(ir + 1, iz) - g.at(ir - 1, iz)) / (2 * g.dR);
            const double dpsidZ = (g.at(ir, iz + 1) - g.at(ir, iz - 1)) / (2 * g.dZ);
            const double Bp2 = (dpsidR * dpsidR + dpsidZ * dpsidZ) / (R * R);
            const double pb = std::clamp((g.at(ir, iz) - eq.psi_axis) / span, 0.0, 1.0);
            const double pres = 0.5 * 1.0 * (1.0 - pb) * (1.0 - pb);   // shape only
            const double dV = 2.0 * 3.14159265358979 * R * g.dR * g.dZ;
            bp2_int += Bp2 * dV; p_int += pres * dV; vol += dV;
        }
    const double mu0_ = 1.25663706212e-6;
    const double Bpa = mu0_ * eq.Ip_A / (2 * 3.14159265358979 * m.a *
                                         std::sqrt((1 + m.kappa_a * m.kappa_a) / 2));
    eq.li = (bp2_int / vol) / (Bpa * Bpa);
    eq.beta_p = 2 * mu0_ * (p_int / vol) / (Bpa * Bpa);   // shape-normalized (reported)
    return eq;
}

} // namespace fusion
