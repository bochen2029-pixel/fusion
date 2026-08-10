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
#include "rings.h"
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

// ---- slice 2b (first piece): the SHAPED fixed-boundary equilibrium + q-profile -----
// The plasma boundary is the Miller D-shape from machine.toml (kappa_sep, delta);
// the SOR solves only inside it (nearest-node mask, first-order at the boundary —
// scaling grade, stated). F(psi) = R0*B0 held constant (paramagnetic correction few
// percent — stated). q(psi) by contour integration on theta-rays from the axis.
struct ShapedEq {
    GsEquilibrium eq;
    double q0 = 0, q95 = 0;
    double shift_mm = 0;                     // axis R - R0 (Shafranov, should be > 0)
    // ---- the MERGE (D-038): the external field's structure at the axis, REPORTED ----
    double Bz_ext_axis_T = 0;                // maintaining (Shafranov) vertical field
    double n_decay = 0;                      // decay index n = -(R/Bz) dBz/dR (<0: the
                                             //   elongating quadrupole — vert. unstable)
    double k_dest_Npm = 0;                   // rigid-displacement destabilizing gradient
                                             //   = -2π R_axis Ip (dBz_ext/dR)|axis
    double ext_residual = 0;                 // |Δ*ψ_ext|/Σ|terms| at the axis (diagnostic:
                                             //   ψ_ext must be discrete-harmonic there)
    std::vector<double> Jphi;                // A/m² on the grid, final Picard iterate
                                             //   (0 outside the plasma; free-boundary
                                             //   solver's warm start + fit input, D-039)
};

inline ShapedEq gs_shaped(const MachineCfg& m, GsGrid& g, int n_picard = 18) {
    const double mu0 = 1.25663706212e-6, PI = 3.14159265358979;
    g.init(m);
    // Miller boundary polygon + point-in-polygon mask
    const int NB = 256;
    std::vector<double> Rb(NB), Zb(NB);
    const double d0 = std::asin(std::clamp(m.delta, 0.0, 0.95));
    for (int k = 0; k < NB; ++k) {
        const double th = 2.0 * PI * k / NB;
        Rb[k] = m.R0 + m.a * std::cos(th + d0 * std::sin(th));
        Zb[k] = m.kappa_sep * m.a * std::sin(th);
    }
    auto inside = [&](double R, double Z) {
        bool in = false;
        for (int k = 0, j = NB - 1; k < NB; j = k++) {
            if (((Zb[k] > Z) != (Zb[j] > Z)) &&
                (R < (Rb[j] - Rb[k]) * (Z - Zb[k]) / (Zb[j] - Zb[k]) + Rb[k]))
                in = !in;
        }
        return in;
    };
    std::vector<char> mask(g.psi.size(), 0);
    for (int iz = 1; iz < GsGrid::NZ - 1; ++iz)
        for (int ir = 1; ir < GsGrid::NR - 1; ++ir)
            mask[size_t(iz) * GsGrid::NR + ir] = inside(g.R(ir), g.Z(iz)) ? 1 : 0;

    // masked SOR (psi = 0 outside/boundary; the plasma is a psi<0 well)
    auto solve_masked = [&](const std::vector<double>& rhs, int sweeps) {
        const double idR2 = 1.0 / (g.dR * g.dR), idZ2 = 1.0 / (g.dZ * g.dZ);
        for (int s = 0; s < sweeps; ++s)
            for (int iz = 1; iz < GsGrid::NZ - 1; ++iz)
                for (int ir = 1; ir < GsGrid::NR - 1; ++ir) {
                    if (!mask[size_t(iz) * GsGrid::NR + ir]) { g.at(ir, iz) = 0.0; continue; }
                    const double R = g.R(ir);
                    const double aE = idR2 - 1.0 / (2.0 * R * g.dR);
                    const double aW = idR2 + 1.0 / (2.0 * R * g.dR);
                    const double diag = -2.0 * idR2 - 2.0 * idZ2;
                    const double res = rhs[size_t(iz) * GsGrid::NR + ir]
                        - aE * g.at(ir + 1, iz) - aW * g.at(ir - 1, iz)
                        - idZ2 * (g.at(ir, iz + 1) + g.at(ir, iz - 1))
                        - diag * g.at(ir, iz);
                    g.at(ir, iz) += 1.85 * res / diag;
                }
    };

    for (auto& v : g.psi) v = 0.0;
    std::vector<double> rhs(g.psi.size(), 0.0);
    double p0 = 1.0e4, f1 = 1.0;
    ShapedEq out;
    const double Ip_target = m.Ip_MA * 1e6;
    int ax_ir = GsGrid::NR / 2, ax_iz = GsGrid::NZ / 2;      // axis node (kept post-loop)
    for (int it = 0; it < n_picard; ++it) {
        double pmin = 0.0; ax_ir = GsGrid::NR / 2; ax_iz = GsGrid::NZ / 2;
        for (int iz = 1; iz < GsGrid::NZ - 1; ++iz)
            for (int ir = 1; ir < GsGrid::NR - 1; ++ir)
                if (mask[size_t(iz) * GsGrid::NR + ir] && g.at(ir, iz) < pmin) {
                    pmin = g.at(ir, iz); ax_ir = ir; ax_iz = iz;
                }
        const double span = (0.0 - pmin) > 1e-30 ? -pmin : 1e-30;
        double Ip_now = 0;
        for (int iz = 1; iz < GsGrid::NZ - 1; ++iz)
            for (int ir = 1; ir < GsGrid::NR - 1; ++ir) {
                const size_t k = size_t(iz) * GsGrid::NR + ir;
                if (!mask[k]) { rhs[k] = 0; continue; }
                const double R = g.R(ir);
                const double pb = std::clamp((g.at(ir, iz) - pmin) / span, 0.0, 1.0);
                const double w = (it == 0) ? 1.0 : (1.0 - pb);
                const double pprime = -p0 * w, ffprime = -f1 * w;
                const double Jphi = -(R * pprime + ffprime / (mu0 * R));
                rhs[k] = -mu0 * R * R * pprime - ffprime;
                Ip_now += Jphi * g.dR * g.dZ;
            }
        const double scale = Ip_target / (std::fabs(Ip_now) > 1.0 ? Ip_now : 1.0);
        p0 *= scale; f1 *= scale;
        for (auto& v : rhs) v *= scale;
        solve_masked(rhs, 2500);
        out.eq.psi_axis = pmin; out.eq.psi_bnd = 0.0;
        out.eq.R_axis = g.R(ax_ir); out.eq.Z_axis = g.Z(ax_iz);
        out.eq.Ip_A = Ip_now * scale; out.eq.picard_iters = it + 1;
    }
    out.shift_mm = (out.eq.R_axis - m.R0) * 1e3;

    // q(psi_bar) via theta-ray contours from the axis; F = R0*B0 (stated approximation)
    auto psi_at = [&](double R, double Z) {                 // bilinear
        const double fr = (R - g.Rmin) / g.dR, fz = (Z - g.Zmin) / g.dZ;
        const int ir = std::clamp(int(fr), 0, GsGrid::NR - 2);
        const int iz = std::clamp(int(fz), 0, GsGrid::NZ - 2);
        const double tr = fr - ir, tz = fz - iz;
        return (1 - tr) * (1 - tz) * g.at(ir, iz) + tr * (1 - tz) * g.at(ir + 1, iz)
             + (1 - tr) * tz * g.at(ir, iz + 1) + tr * tz * g.at(ir + 1, iz + 1);
    };
    auto grad_at = [&](double R, double Z, double& gR, double& gZ) {
        const double h = 0.5 * g.dR;
        gR = (psi_at(R + h, Z) - psi_at(R - h, Z)) / (2 * h);
        gZ = (psi_at(R, Z + h) - psi_at(R, Z - h)) / (2 * h);
    };
    auto q_of = [&](double pbar) {
        const double target = out.eq.psi_axis * (1.0 - pbar);   // psi<0 well, bnd=0
        const int NTH = 128;
        double integ = 0, prevR = 0, prevZ = 0, firstR = 0, firstZ = 0;
        for (int k = 0; k <= NTH; ++k) {
            const double th = 2.0 * PI * k / NTH;
            double lo = 0.0, hi = 1.0;                       // ray: axis -> boundary poly
            const double bR = m.R0 + m.a * std::cos(th + d0 * std::sin(th));
            const double bZ = m.kappa_sep * m.a * std::sin(th);
            for (int b = 0; b < 40; ++b) {                    // bisection on psi
                const double mid = 0.5 * (lo + hi);
                const double R = out.eq.R_axis + mid * (bR - out.eq.R_axis);
                const double Z = out.eq.Z_axis + mid * (bZ - out.eq.Z_axis);
                (psi_at(R, Z) < target) ? lo = mid : hi = mid;
            }
            const double R = out.eq.R_axis + lo * (bR - out.eq.R_axis);
            const double Z = out.eq.Z_axis + lo * (bZ - out.eq.Z_axis);
            if (k == 0) { firstR = prevR = R; firstZ = prevZ = Z; continue; }
            const double dl = std::sqrt((R - prevR) * (R - prevR) + (Z - prevZ) * (Z - prevZ));
            double gR, gZ; grad_at(0.5 * (R + prevR), 0.5 * (Z + prevZ), gR, gZ);
            const double gp = std::sqrt(gR * gR + gZ * gZ);
            integ += dl / (0.5 * (R + prevR) * (gp > 1e-9 ? gp : 1e-9));
            prevR = R; prevZ = Z;
        }
        (void)firstR; (void)firstZ;
        return (m.R0 * m.B0) / (2.0 * PI) * integ;
    };
    out.q95 = q_of(0.95);
    out.q0 = q_of(0.10);                      // near-axis proxy (grid resolution floor)

    out.Jphi.assign(g.psi.size(), 0.0);       // final-iterate current density (D-039)
    for (int iz = 1; iz < GsGrid::NZ - 1; ++iz)
        for (int ir = 1; ir < GsGrid::NR - 1; ++ir) {
            const size_t k = size_t(iz) * GsGrid::NR + ir;
            if (mask[k]) out.Jphi[k] = rhs[k] / (mu0 * g.R(ir));
        }

    // ---- the MERGE (D-038): external-field decay index → k_dest, REPORTED ----------
    // ψ_total = ψ_self (the plasma current in free space) + ψ_ext (the vacuum field the
    // fixed boundary implies — the shaping/maintaining field). ψ_self: the same discrete
    // Δ* operator on the FULL rectangle with Dirichlet edges from the Maxwell-mutual
    // Green sum over J_φ (edge points are far from the current — no kernel singularity).
    // ψ_ext = ψ_total − ψ_self is then discrete-harmonic inside the mask; its midplane
    // Bz structure at the axis gives the decay index and the rigid-ring destabilizing
    // gradient. This is the free-boundary machinery's first physical use (Green tables
    // arrive for coils next slice — same primitives).
    {
        const double dA = g.dR * g.dZ;
        GsGrid gself; gself.init(m);                       // same rectangle, ψ = 0
        // Dirichlet edges: full-flux Maxwell sum / 2π, code-sign (Δ*ψ_code = +μ0 R J)
        auto green_edge = [&](double Re, double Ze) {
            double s = 0.0;
            for (int iz = 1; iz < GsGrid::NZ - 1; ++iz)
                for (int ir = 1; ir < GsGrid::NR - 1; ++ir) {
                    const size_t k = size_t(iz) * GsGrid::NR + ir;
                    if (!mask[k] || rhs[k] == 0.0) continue;
                    const double Ic = rhs[k] / (mu0 * g.R(ir)) * dA;   // cell current
                    s += ring_mutual(g.R(ir), Re, Ze - g.Z(iz)) * Ic;
                }
            return -s / (2.0 * PI);                        // standard→code sign flip
        };
        for (int ir = 0; ir < GsGrid::NR; ++ir) {
            gself.at(ir, 0) = green_edge(g.R(ir), g.Z(0));
            gself.at(ir, GsGrid::NZ - 1) = green_edge(g.R(ir), g.Z(GsGrid::NZ - 1));
        }
        for (int iz = 1; iz < GsGrid::NZ - 1; ++iz) {
            gself.at(0, iz) = green_edge(g.R(0), g.Z(iz));
            gself.at(GsGrid::NR - 1, iz) = green_edge(g.R(GsGrid::NR - 1), g.Z(iz));
        }
        gs_solve(gself, rhs, 8000);                        // free-space plasma field
        // ψ_ext in the STANDARD sign (ψ_std = −ψ_code; Bz = (1/R)∂ψ_std/∂R)
        auto psi_ext = [&](int ir, int iz) { return -(g.at(ir, iz) - gself.at(ir, iz)); };
        auto Bz_at = [&](int ir) {
            return (psi_ext(ir + 1, ax_iz) - psi_ext(ir - 1, ax_iz)) / (2.0 * g.dR * g.R(ir));
        };
        const double Bz0 = Bz_at(ax_ir);
        const double dBzdR = (Bz_at(ax_ir + 1) - Bz_at(ax_ir - 1)) / (2.0 * g.dR);
        out.Bz_ext_axis_T = Bz0;
        out.n_decay = (std::fabs(Bz0) > 1e-12) ? -(out.eq.R_axis / Bz0) * dBzdR : 0.0;
        // rigid ring: F_Z(δZ) = −2πR Ip B_R,ext(δZ);  ∂B_R/∂Z = ∂B_z/∂R (vacuum)
        out.k_dest_Npm = -2.0 * PI * out.eq.R_axis * out.eq.Ip_A * dBzdR;
        // diagnostic: ψ_ext discrete-harmonic at the axis (relative residual)
        {
            const double idR2 = 1.0 / (g.dR * g.dR), idZ2 = 1.0 / (g.dZ * g.dZ);
            const double R = g.R(ax_ir);
            const double aE = idR2 - 1.0 / (2.0 * R * g.dR);
            const double aW = idR2 + 1.0 / (2.0 * R * g.dR);
            const double tE = aE * psi_ext(ax_ir + 1, ax_iz), tW = aW * psi_ext(ax_ir - 1, ax_iz);
            const double tN = idZ2 * psi_ext(ax_ir, ax_iz + 1), tS = idZ2 * psi_ext(ax_ir, ax_iz - 1);
            const double tC = (-2.0 * idR2 - 2.0 * idZ2) * psi_ext(ax_ir, ax_iz);
            const double denom = std::fabs(tE) + std::fabs(tW) + std::fabs(tN)
                               + std::fabs(tS) + std::fabs(tC);
            out.ext_residual = std::fabs(tE + tW + tN + tS + tC) / (denom > 0 ? denom : 1.0);
        }
    }
    return out;
}

// The equilibrium→vertical bridge (the MERGE, D-038): everything the vertical channel
// needs from the shape, derived — never configured. Pure function of the machine config
// at this slice (profiles fixed); callers that Monte-Carlo compute it ONCE and pass it
// through SimInputs.
inline VertDerived gs_vertical_derive(const MachineCfg& m) {
    GsGrid g;
    const ShapedEq sh = gs_shaped(m, g);
    VertDerived v;
    v.k_dest_Npm = sh.k_dest_Npm;
    v.n_decay = sh.n_decay;
    v.Bz_ext_axis_T = sh.Bz_ext_axis_T;
    v.R_axis_m = sh.eq.R_axis;
    v.set = true;
    return v;
}

} // namespace fusion
