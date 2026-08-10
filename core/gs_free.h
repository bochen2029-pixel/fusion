// core/gs_free.h — M1: the FREE-BOUNDARY Grad-Shafranov solver (spec §2.1 tier-1).
//
// The plasma boundary is NOT prescribed: the 12 actuated circuits (machine.toml
// [coils], D-039 geometry pins) produce the external field; the boundary is the first
// limiting surface found self-consistently — the separatrix through an X-point
// (FUSOR-1 is DOUBLE-NULL, D-039) or the first-wall contour (limiter fallback).
//
// Boundary coupling (the von Hagenow ROLE, D-039 technique ruling): the plasma field's
// rectangle-edge values come from a PRECOMPUTED Green response matrix (edge x interior,
// Maxwell ring mutuals — the exact discrete free-space Green operator, ~8 MB). Von
// Hagenow's surface-integral form was a memory optimization for machines that could
// not hold that matrix; the interface (J -> edge BC) is identical and the fast-solver
// slice may swap it. Verified against the D-038 direct decomposition (k_dest cross-
// check: the fixed-boundary IMPLIED external field vs the ACTUAL coil field).
//
// Inverse isoflux fit: DN symmetry -> 6 circuit unknowns (CS1=CS3, PFxU=PFxL, CS2;
// VS=0 at equilibrium). Residuals: isoflux at Miller control points + separatrix-flux
// + B=0 at the X target; Tikhonov-damped least squares; currents REPORTED vs i_max
// (the design-feasibility check for the synthetic coil set — receipted).
//
// Determinism: fixed sweep/iteration counts, fixed-order sums and flood fill, no
// wall-clock. DN symmetry is ENFORCED by J-symmetrization each iteration (the SOR
// sweep order would otherwise break it at FP rounding and let the neutral vertical
// position wander — stated, not hidden).
#pragma once
#include "config.h"
#include "gs.h"
#include "rings.h"
#include <cmath>
#include <cstdio>
#include <algorithm>
#include <memory>
#include <vector>

namespace fusion {

struct FreeEq {
    GsEquilibrium eq;                 // psi_axis/bnd, axis, Ip, iters
    double q0 = 0, q95 = 0;
    double shift_mm = 0;
    double Rx = 0, Zx = 0, psi_x = 0; // upper X-point (lower mirrored by DN symmetry)
    bool   x_limited = false;         // separatrix-bounded (vs wall-limited)
    double kappa_ach = 0, delta_ach = 0, a_ach = 0;   // achieved LCFS shape
    double I_At[6] = {0};             // fitted circuit ampere-turns
                                      // [CS1&CS3, CS2, PF1, PF2, PF3, PF4]
    double imax_frac = 0;             // max |I|/i_max over the six circuits
    double fit_rms = 0;               // isoflux residual rms [Wb/rad]
    double Bz_ext_axis_T = 0, n_decay = 0, k_dest_Npm = 0;  // from the COIL field
    double drift_axis = 0;            // |dpsi_axis| over the last Picard step (report)
};

struct GsFreeSolver {
    static constexpr int NR = GsGrid::NR, NZ = GsGrid::NZ;
    static constexpr int NCIRC = 6;
    GsGrid g;
    std::vector<double> Gc[NCIRC];    // circuit tables: psi_code per ampere-turn, NR*NZ
    std::vector<double> Gresp;        // edge response: [n_edge][n_int] (row-major)
    std::vector<int>    edge_ir, edge_iz;          // edge node coords (fixed order)
    std::vector<double> wallR, wallZ; // first-wall polygon (Miller * wall_over_a)
    double d0 = 0;                    // asin(delta)

    // ---- geometry helpers ----------------------------------------------------------
    static double PI() { return 3.14159265358979; }
    void miller_pt(const MachineCfg& m, double th, double scale, double& R, double& Z) const {
        R = m.R0 + scale * m.a * std::cos(th + d0 * std::sin(th));
        Z = m.kappa_sep * scale * m.a * std::sin(th);
    }
    bool in_wall(double R, double Z) const {
        bool in = false;
        const int n = int(wallR.size());
        for (int k = 0, j = n - 1; k < n; j = k++) {
            if (((wallZ[k] > Z) != (wallZ[j] > Z)) &&
                (R < (wallR[j] - wallR[k]) * (Z - wallZ[k]) / (wallZ[j] - wallZ[k]) + wallR[k]))
                in = !in;
        }
        return in;
    }
    // psi_code of one ampere(-turn) ring at (rc,zc), observed at (R,Z): the standard
    // Green flux / 2pi, sign-flipped into the code convention (D-038's derivation)
    static double ringpsi_code(double rc, double zc, double R, double Z) {
        return -ring_mutual(rc, R, Z - zc) / (2.0 * PI());
    }

    // ---- one-time build (deterministic) --------------------------------------------
    void init(const MachineCfg& m) {
        g.init(m);
        d0 = std::asin(std::clamp(m.delta, 0.0, 0.95));
        // first-wall polygon
        const int NW = 256;
        wallR.resize(NW); wallZ.resize(NW);
        for (int k = 0; k < NW; ++k)
            miller_pt(m, 2.0 * PI() * k / NW, m.wall_over_a, wallR[k], wallZ[k]);
        // circuit tables: rows of machine.toml [coils] grouped DN-symmetric
        static constexpr int GROUPS[NCIRC][2] = { {0,2},{1,-1},{3,4},{5,6},{7,8},{9,10} };
        for (int c = 0; c < NCIRC; ++c) {
            Gc[c].assign(size_t(NR) * NZ, 0.0);
            for (int gi = 0; gi < 2; ++gi) {
                const int row = GROUPS[c][gi];
                if (row < 0) continue;
                const double rc = m.coil_r[row], zc = m.coil_z[row], hh = m.coil_hh[row];
                const int nsub = hh > 0 ? 3 : 1;
                for (int s = 0; s < nsub; ++s) {
                    const double zs = zc + (nsub == 3 ? (s - 1) * hh : 0.0);
                    const double w = 1.0 / nsub;
                    for (int iz = 0; iz < NZ; ++iz)
                        for (int ir = 0; ir < NR; ++ir)
                            Gc[c][size_t(iz) * NR + ir] +=
                                w * ringpsi_code(rc, zs, g.R(ir), g.Z(iz));
                }
            }
        }
        // edge node list (fixed order: bottom row, top row, then left/right columns)
        edge_ir.clear(); edge_iz.clear();
        for (int ir = 0; ir < NR; ++ir) { edge_ir.push_back(ir); edge_iz.push_back(0); }
        for (int ir = 0; ir < NR; ++ir) { edge_ir.push_back(ir); edge_iz.push_back(NZ - 1); }
        for (int iz = 1; iz < NZ - 1; ++iz) { edge_ir.push_back(0); edge_iz.push_back(iz); }
        for (int iz = 1; iz < NZ - 1; ++iz) { edge_ir.push_back(NR - 1); edge_iz.push_back(iz); }
        // plasma->edge response matrix over interior cells (the von Hagenow role)
        const size_t ne = edge_ir.size(), ni = size_t(NR - 2) * (NZ - 2);
        const double dA = g.dR * g.dZ;
        Gresp.assign(ne * ni, 0.0);
        for (size_t e = 0; e < ne; ++e) {
            const double Re = g.R(edge_ir[e]), Ze = g.Z(edge_iz[e]);
            size_t q = 0;
            for (int iz = 1; iz < NZ - 1; ++iz)
                for (int ir = 1; ir < NR - 1; ++ir, ++q)
                    Gresp[e * ni + q] = ringpsi_code(g.R(ir), g.Z(iz), Re, Ze) * dA;
        }
    }

    // psi_p free-space value at an arbitrary point from the current density (direct sum)
    double psi_p_at(const std::vector<double>& J, double R, double Z) const {
        const double dA = g.dR * g.dZ;
        double s = 0.0;
        for (int iz = 1; iz < NZ - 1; ++iz)
            for (int ir = 1; ir < NR - 1; ++ir) {
                const double j = J[size_t(iz) * NR + ir];
                if (j != 0.0) s += ringpsi_code(g.R(ir), g.Z(iz), R, Z) * j * dA;
            }
        return s;
    }

    // ---- the inverse isoflux fit (6 unknowns; Tikhonov; deterministic) -------------
    // Unknowns are NORMALIZED by i_max (conditions the near-degenerate inboard
    // columns); ridge at the 1e-3 level in that space keeps currents physical —
    // the fit must succeed WITHOUT heroic cancellations or the coil set is infeasible.
    // Returns rms of the flux residuals at the solution.
    double fit_currents(const MachineCfg& m, const std::vector<double>& J, double I[NCIRC]) {
        // control points: upper-half Miller boundary + the X target (theta = 90 deg);
        // the 75/105 deg stations are deliberately absent — crowding control points
        // against the X vertex fights the B=0 condition (measured: shape twist)
        const double THS[] = { 0.0, 15.0, 30.0, 45.0, 60.0, 120.0,
                               135.0, 150.0, 165.0, 180.0 };
        const int NP_ = 10;
        double pR[NP_ + 2], pZ[NP_ + 2];
        for (int i = 0; i < NP_; ++i) miller_pt(m, THS[i] * PI() / 180.0, 1.0, pR[i], pZ[i]);
        double Rx, Zx; miller_pt(m, 0.5 * PI(), 1.0, Rx, Zx);
        // two flanking stations at the target-X height: flux-flatten the Z_x locus so
        // the self-consistent null cannot ride up (measured drift without them: +0.23 m)
        pR[NP_] = Rx - 0.15; pZ[NP_] = Zx;
        pR[NP_ + 1] = Rx + 0.15; pZ[NP_ + 1] = Zx;
        const int NPT = NP_ + 2;
        const double h = 0.5 * g.dR;                    // B stencil half-step
        // rows: NPT-1 isoflux diffs + X isoflux + BR(X) + BZ(X)   (weights below)
        const int NROW = NPT - 1 + 3;
        double A[NPT + 2][NCIRC], b[NPT + 2];
        const double w_B = 2.0;                          // meters: T -> Wb/rad scale
        auto psi_coil_pt = [&](int c, double R, double Z) {   // table-free (exact ring sum)
            static constexpr int GROUPS[NCIRC][2] = { {0,2},{1,-1},{3,4},{5,6},{7,8},{9,10} };
            double s = 0.0;
            for (int gi = 0; gi < 2; ++gi) {
                const int row = GROUPS[c][gi];
                if (row < 0) continue;
                const double rc = m.coil_r[row], zc = m.coil_z[row], hh = m.coil_hh[row];
                const int nsub = hh > 0 ? 3 : 1;
                for (int s2 = 0; s2 < nsub; ++s2)
                    s += ringpsi_code(rc, zc + (nsub == 3 ? (s2 - 1) * hh : 0.0), R, Z) / nsub;
            }
            return s;
        };
        for (int r = 0; r < NROW; ++r) { b[r] = 0; for (int c = 0; c < NCIRC; ++c) A[r][c] = 0; }
        for (int i = 1; i < NPT; ++i) {                  // isoflux vs point 0
            for (int c = 0; c < NCIRC; ++c)
                A[i - 1][c] = psi_coil_pt(c, pR[i], pZ[i]) - psi_coil_pt(c, pR[0], pZ[0]);
            b[i - 1] = psi_p_at(J, pR[i], pZ[i]) - psi_p_at(J, pR[0], pZ[0]);
        }
        for (int c = 0; c < NCIRC; ++c)                  // X on the same surface
            A[NPT - 1][c] = psi_coil_pt(c, Rx, Zx) - psi_coil_pt(c, pR[0], pZ[0]);
        b[NPT - 1] = psi_p_at(J, Rx, Zx) - psi_p_at(J, pR[0], pZ[0]);
        // B_R = -(1/R) dpsi_std/dZ = +(1/R) dpsi_code/dZ ; B_Z = -(1/R) dpsi_code/dR
        for (int c = 0; c < NCIRC; ++c) {
            A[NPT][c] = w_B * (psi_coil_pt(c, Rx, Zx + h) - psi_coil_pt(c, Rx, Zx - h)) / (2 * h * Rx);
            A[NPT + 1][c] = -w_B * (psi_coil_pt(c, Rx + h, Zx) - psi_coil_pt(c, Rx - h, Zx)) / (2 * h * Rx);
        }
        b[NPT] = w_B * (psi_p_at(J, Rx, Zx + h) - psi_p_at(J, Rx, Zx - h)) / (2 * h * Rx);
        b[NPT + 1] = -w_B * (psi_p_at(J, Rx + h, Zx) - psi_p_at(J, Rx - h, Zx)) / (2 * h * Rx);
        // normalize unknowns by i_max, then normal equations + ridge (1e-3 relative)
        const double inorm[NCIRC] = { m.coil_imax_At[0], m.coil_imax_At[1], m.coil_imax_At[3],
                                      m.coil_imax_At[5], m.coil_imax_At[7], m.coil_imax_At[9] };
        for (int r = 0; r < NROW; ++r)
            for (int c = 0; c < NCIRC; ++c) A[r][c] *= inorm[c];
        double N[NCIRC][NCIRC], rhs[NCIRC];
        for (int i = 0; i < NCIRC; ++i) {
            rhs[i] = 0;
            for (int j2 = 0; j2 < NCIRC; ++j2) N[i][j2] = 0;
        }
        for (int r = 0; r < NROW; ++r)
            for (int i = 0; i < NCIRC; ++i) {
                rhs[i] -= A[r][i] * b[r];
                for (int j2 = 0; j2 < NCIRC; ++j2) N[i][j2] += A[r][i] * A[r][j2];
            }
        double tr = 0; for (int i = 0; i < NCIRC; ++i) tr += N[i][i];
        const double lam2 = 1e-3 * tr / NCIRC;
        for (int i = 0; i < NCIRC; ++i) N[i][i] += lam2;
        // 6x6 Gauss with partial pivoting (fixed order — deterministic)
        int piv[NCIRC];
        for (int i = 0; i < NCIRC; ++i) piv[i] = i;
        for (int col = 0; col < NCIRC; ++col) {
            int best = col;
            for (int r = col + 1; r < NCIRC; ++r)
                if (std::fabs(N[r][col]) > std::fabs(N[best][col])) best = r;
            if (best != col) {
                for (int c2 = 0; c2 < NCIRC; ++c2) std::swap(N[col][c2], N[best][c2]);
                std::swap(rhs[col], rhs[best]);
            }
            const double d = N[col][col] != 0.0 ? N[col][col] : 1e-30;
            for (int r = col + 1; r < NCIRC; ++r) {
                const double f = N[r][col] / d;
                for (int c2 = col; c2 < NCIRC; ++c2) N[r][c2] -= f * N[col][c2];
                rhs[r] -= f * rhs[col];
            }
        }
        double x[NCIRC];
        for (int i = NCIRC - 1; i >= 0; --i) {
            double s = rhs[i];
            for (int j2 = i + 1; j2 < NCIRC; ++j2) s -= N[i][j2] * x[j2];
            x[i] = s / (N[i][i] != 0.0 ? N[i][i] : 1e-30);
        }
        (void)piv;
        double ss = 0;
        for (int r = 0; r < NROW; ++r) {
            double v = b[r];
            for (int c = 0; c < NCIRC; ++c) v += A[r][c] * x[c];
            ss += v * v;
        }
        for (int c = 0; c < NCIRC; ++c) I[c] = x[c] * inorm[c];   // back to ampere-turns
        return std::sqrt(ss / NROW);
    }

    // ---- the free-boundary Picard (frozen currents; fixed counts) ------------------
    struct PicardState {
        std::vector<double> J, psi_p, psi_coil, psi_tot;
        std::vector<char> mask;
        double psi_axis = 0, psi_bnd = 0;
        int ax_ir = NR / 2, ax_iz = NZ / 2;
        double Rx = 0, Zx = 0, psi_x = 0; bool have_x = false, x_limited = false;
        double drift = 0;
    };

    void load_coil_field(const double I[NCIRC], std::vector<double>& psi_coil) const {
        psi_coil.assign(size_t(NR) * NZ, 0.0);
        for (int c = 0; c < NCIRC; ++c)
            for (size_t k = 0; k < psi_coil.size(); ++k)
                psi_coil[k] += Gc[c][k] * I[c];
    }

    // saddle refine on bilinear psi_tot: fixed Newton iterations; returns success
    bool find_x(const std::vector<double>& psi, double R0g, double Z0g,
                double& Rx, double& Zx, double& px) const {
        auto psi_at = [&](double R, double Z) {
            const double fr = (R - g.Rmin) / g.dR, fz = (Z - g.Zmin) / g.dZ;
            const int ir = std::clamp(int(fr), 0, NR - 2);
            const int iz = std::clamp(int(fz), 0, NZ - 2);
            const double tr = fr - ir, tz = fz - iz;
            const size_t k = size_t(iz) * NR + ir;
            return (1 - tr) * (1 - tz) * psi[k] + tr * (1 - tz) * psi[k + 1]
                 + (1 - tr) * tz * psi[k + NR] + tr * tz * psi[k + NR + 1];
        };
        const double h = 0.35 * g.dR;
        double R = R0g, Z = Z0g;
        for (int it = 0; it < 14; ++it) {
            const double gR = (psi_at(R + h, Z) - psi_at(R - h, Z)) / (2 * h);
            const double gZ = (psi_at(R, Z + h) - psi_at(R, Z - h)) / (2 * h);
            const double RR = (psi_at(R + h, Z) - 2 * psi_at(R, Z) + psi_at(R - h, Z)) / (h * h);
            const double ZZ = (psi_at(R, Z + h) - 2 * psi_at(R, Z) + psi_at(R, Z - h)) / (h * h);
            const double RZ = (psi_at(R + h, Z + h) - psi_at(R + h, Z - h)
                             - psi_at(R - h, Z + h) + psi_at(R - h, Z - h)) / (4 * h * h);
            const double det = RR * ZZ - RZ * RZ;
            if (std::fabs(det) < 1e-14) return false;
            double dR = -(ZZ * gR - RZ * gZ) / det, dZ = -(-RZ * gR + RR * gZ) / det;
            dR = std::clamp(dR, -g.dR, g.dR); dZ = std::clamp(dZ, -g.dZ, g.dZ);
            R += dR; Z += dZ;
            if (R < g.Rmin + g.dR || R > g.Rmax - g.dR ||
                Z < g.Zmin + g.dZ || Z > g.Zmax - g.dZ) return false;
        }
        const double gR = (psi_at(R + h, Z) - psi_at(R - h, Z)) / (2 * h);
        const double gZ = (psi_at(R, Z + h) - psi_at(R, Z - h)) / (2 * h);
        const double RR = (psi_at(R + h, Z) - 2 * psi_at(R, Z) + psi_at(R - h, Z)) / (h * h);
        const double ZZ = (psi_at(R, Z + h) - 2 * psi_at(R, Z) + psi_at(R, Z - h)) / (h * h);
        const double RZ = (psi_at(R + h, Z + h) - psi_at(R + h, Z - h)
                         - psi_at(R - h, Z + h) + psi_at(R - h, Z - h)) / (4 * h * h);
        const double gnorm = std::sqrt(gR * gR + gZ * gZ);
        if (gnorm > 0.02 * std::fabs(psi_at(R, Z)) / g.dR) return false;   // not stationary
        if (RR * ZZ - RZ * RZ >= 0.0) return false;                        // not a saddle
        Rx = R; Zx = Z; px = psi_at(R, Z);
        return true;
    }

    // ONE Picard iteration on caller-owned state (gw = work grid). CONST: all tables
    // read-only — the same call serves the offline solve AND the D-041 runtime
    // tracking mode (one warm iteration per pipeline slot at the CURRENT Ip — the
    // real-time-GS pattern). Ip_target parameterizes ramps.
    void picard_iter(const MachineCfg& m, GsGrid& gw, PicardState& st, int sor_sweeps,
                     double Ip_target) const {
        const double mu0 = 1.25663706212e-6;
        const size_t ni = size_t(NR - 2) * (NZ - 2);
        // (a) plasma-field edge BC from the response matrix; SOR on the interior
        std::vector<double> rhs(size_t(NR) * NZ, 0.0);
        for (int iz = 1; iz < NZ - 1; ++iz)
            for (int ir = 1; ir < NR - 1; ++ir) {
                const size_t k = size_t(iz) * NR + ir;
                rhs[k] = mu0 * gw.R(ir) * st.J[k];
            }
        for (size_t e = 0; e < edge_ir.size(); ++e) {
            double s = 0.0;
            size_t q = 0;
            const double* row = &Gresp[e * ni];
            for (int iz = 1; iz < NZ - 1; ++iz)
                for (int ir = 1; ir < NR - 1; ++ir, ++q) {
                    const double j = st.J[size_t(iz) * NR + ir];
                    if (j != 0.0) s += row[q] * j;
                }
            st.psi_p[size_t(edge_iz[e]) * NR + edge_ir[e]] = s;
        }
        // warm-started SOR (edges hold the BC; interior carries the previous iterate)
        gw.psi = st.psi_p;
        gs_solve(gw, rhs, sor_sweeps);
        st.psi_p = gw.psi;
        // (b) total field
        for (size_t k = 0; k < st.psi_tot.size(); ++k)
            st.psi_tot[k] = st.psi_p[k] + st.psi_coil[k];
        // (c) axis: min psi_tot inside the wall
        const double prev_axis = st.psi_axis;
        double pmin = 1e300;
        for (int iz = 1; iz < NZ - 1; ++iz)
            for (int ir = 1; ir < NR - 1; ++ir) {
                const size_t k = size_t(iz) * NR + ir;
                if (!in_wall(gw.R(ir), gw.Z(iz))) continue;
                if (st.psi_tot[k] < pmin) { pmin = st.psi_tot[k]; st.ax_ir = ir; st.ax_iz = iz; }
            }
        st.psi_axis = pmin;
        st.drift = std::fabs(st.psi_axis - prev_axis);
        // (d) X-point: Newton from the DN target (upper); DN symmetry gives the lower
        double Rx0, Zx0; miller_pt(m, 0.5 * PI(), 1.0, Rx0, Zx0);
        st.have_x = find_x(st.psi_tot, Rx0, Zx0, st.Rx, st.Zx, st.psi_x);
        // (e/f/g) boundary + mask in one flood machinery. The limiter candidate must be
        // sensed ONLY where the wall blocks the AXIS-CONNECTED region — wall points
        // beyond the X sit in private flux where psi drops again and would falsely
        // undercut the separatrix (measured: the wall "min" chased the private zone).
        // Pass A floods below psi_x (or unbounded if no X) inside the wall; if the
        // flood is wall-blocked anywhere, the touch flux re-bounds it (pass B).
        auto flood = [&](double bound, double& wall_touch_psi) {
            std::fill(st.mask.begin(), st.mask.end(), char(0));
            wall_touch_psi = 1e300;
            const double eps = 1e-9 * std::fabs(bound - st.psi_axis);
            std::vector<int> stack;
            stack.push_back(st.ax_iz * NR + st.ax_ir);
            st.mask[size_t(st.ax_iz) * NR + st.ax_ir] = 1;
            while (!stack.empty()) {
                const int k = stack.back(); stack.pop_back();
                const int iz = k / NR, ir = k % NR;
                static constexpr int DIR[4][2] = { {1,0},{-1,0},{0,1},{0,-1} };
                for (auto& d : DIR) {
                    const int ir2 = ir + d[0], iz2 = iz + d[1];
                    if (ir2 < 1 || ir2 >= NR - 1 || iz2 < 1 || iz2 >= NZ - 1) continue;
                    const size_t k2 = size_t(iz2) * NR + ir2;
                    if (st.mask[k2]) continue;
                    if (st.psi_tot[k2] >= bound - eps) continue;
                    if (!in_wall(gw.R(ir2), gw.Z(iz2))) {          // wall-blocked frontier
                        wall_touch_psi = std::fmin(wall_touch_psi, st.psi_tot[k2]);
                        continue;
                    }
                    st.mask[k2] = 1;
                    stack.push_back(int(k2));
                }
            }
        };
        double touch;
        const double bound0 = st.have_x ? st.psi_x : 1e300;
        flood(bound0, touch);
        if (touch < bound0) {                          // the wall cuts a lower surface
            st.psi_bnd = touch; st.x_limited = false;
            flood(st.psi_bnd, touch);
        } else {
            st.psi_bnd = st.have_x ? st.psi_x : touch;
            st.x_limited = st.have_x;
        }
        // (h) profile update (same linear family as the fixed solver, p0:f1 = 1e4:1),
        //     Ip renormalization, 0.5 under-relaxation, DN symmetrization (stated)
        const double span = (st.psi_bnd - st.psi_axis) > 1e-30 ? st.psi_bnd - st.psi_axis : 1e-30;
        std::vector<double> Jn(st.J.size(), 0.0);
        double Ip_now = 0;
        const double dA = gw.dR * gw.dZ;
        for (int iz = 1; iz < NZ - 1; ++iz)
            for (int ir = 1; ir < NR - 1; ++ir) {
                const size_t k = size_t(iz) * NR + ir;
                if (!st.mask[k]) continue;
                const double pb = std::clamp((st.psi_tot[k] - st.psi_axis) / span, 0.0, 1.0);
                const double w = 1.0 - pb;
                const double R = gw.R(ir);
                Jn[k] = w * (R * 1.0e4 + 1.0 / (mu0 * R));
                Ip_now += Jn[k] * dA;
            }
        const double Ip_t = Ip_target;
        const double sc = Ip_t / (std::fabs(Ip_now) > 1.0 ? Ip_now : 1.0);
        for (auto& v : Jn) v *= sc;
        for (size_t k = 0; k < st.J.size(); ++k) st.J[k] = 0.5 * st.J[k] + 0.5 * Jn[k];
        for (int iz = 1; iz < NZ / 2 + 1; ++iz)             // DN symmetrization
            for (int ir = 1; ir < NR - 1; ++ir) {
                const size_t ka = size_t(iz) * NR + ir, kb = size_t(NZ - 1 - iz) * NR + ir;
                const double v = 0.5 * (st.J[ka] + st.J[kb]);
                st.J[ka] = st.J[kb] = v;
            }
        double Ip2 = 0;
        for (int iz = 1; iz < NZ - 1; ++iz)
            for (int ir = 1; ir < NR - 1; ++ir) Ip2 += st.J[size_t(iz) * NR + ir] * dA;
        const double sc2 = Ip_t / (std::fabs(Ip2) > 1.0 ? Ip2 : 1.0);
        for (auto& v : st.J) v *= sc2;
    }

    // ---- the full solve: warm start -> (fit -> Picard) x n_outer -> outputs --------
    // Outer alternation: refit against the evolved J, 0.5-under-relaxed on currents
    // (fit chasing plasma chasing fit oscillates otherwise — measured).
    // the axis-local external-field derive (shared by solve() and the tracking mode)
    VertDerived derive_at_axis(const PicardState& st, double Ip_A) const {
        VertDerived v;
        auto Bz_coil = [&](int ir) {
            const size_t k = size_t(st.ax_iz) * NR + ir;
            return -(st.psi_coil[k + 1] - st.psi_coil[k - 1]) / (2.0 * g.dR * g.R(ir));
        };
        const double Bz0 = Bz_coil(st.ax_ir);
        const double dBzdR = (Bz_coil(st.ax_ir + 1) - Bz_coil(st.ax_ir - 1)) / (2.0 * g.dR);
        v.Bz_ext_axis_T = Bz0;
        v.R_axis_m = g.R(st.ax_ir);
        v.n_decay = (std::fabs(Bz0) > 1e-12) ? -(v.R_axis_m / Bz0) * dBzdR : 0.0;
        v.k_dest_Npm = -2.0 * PI() * v.R_axis_m * Ip_A * dBzdR;
        v.set = true;
        return v;
    }

    // ---- D-041: the runtime tracking mode ------------------------------------------
    // Per-run state seeded from the converged full solve; track_step runs ONE warm
    // Picard iteration at the CURRENT Ip and re-derives the vertical inputs. Fixed
    // coil currents (no shape controller until M2) — the shape drift under ramps is
    // the physics, and the lethal-legal lever (relative elongation grows as Ip falls).
    struct FreeTrack {
        GsGrid gw;
        PicardState st;
    };
    void seed_track(const PicardState& converged, FreeTrack& tr) const {
        tr.gw = g;                       // geometry copy (psi overwritten per solve)
        tr.st = converged;
    }
    VertDerived track_step(const MachineCfg& m, FreeTrack& tr, double Ip_A) const {
        picard_iter(m, tr.gw, tr.st, 0, Ip_A);
        return derive_at_axis(tr.st, Ip_A);
    }

    FreeEq solve(const MachineCfg& m, int n_outer = 4, int n_picard = 30,
                 bool verbose = false, PicardState* keep = nullptr) {
        const double mu0 = 1.25663706212e-6;
        FreeEq out;
        // warm start: the verified fixed-boundary equilibrium (exports its J — D-039)
        GsGrid gtmp;
        const ShapedEq sh = gs_shaped(m, gtmp);
        PicardState st;
        st.J = sh.Jphi;
        st.psi_p.assign(size_t(NR) * NZ, 0.0);
        st.psi_tot.assign(size_t(NR) * NZ, 0.0);
        st.mask.assign(size_t(NR) * NZ, 0);
        double Ifit[NCIRC] = {0}, Inew[NCIRC];
        for (int outer = 0; outer < n_outer; ++outer) {
            out.fit_rms = fit_currents(m, st.J, Inew);
            for (int c = 0; c < NCIRC; ++c)
                Ifit[c] = (outer == 0) ? Inew[c] : 0.5 * Ifit[c] + 0.5 * Inew[c];
            load_coil_field(Ifit, st.psi_coil);
            for (int it = 0; it < n_picard; ++it)
                picard_iter(m, g, st, 0, m.Ip_MA * 1e6);
            if (verbose)
                std::printf("  outer %d: fit_rms %.2e  I[kAt] %.0f %.0f %.0f %.0f %.0f %.0f"
                            "  axis R %.3f Z %+.4f  X(%.3f,%.3f)%s  psi_x-pw %+.3e\n",
                            outer, out.fit_rms, Ifit[0] / 1e3, Ifit[1] / 1e3, Ifit[2] / 1e3,
                            Ifit[3] / 1e3, Ifit[4] / 1e3, Ifit[5] / 1e3,
                            g.R(st.ax_ir), g.Z(st.ax_iz), st.Rx, st.Zx,
                            st.have_x ? "" : "!", st.psi_x - st.psi_bnd);
        }
        // outputs
        for (int c = 0; c < NCIRC; ++c) out.I_At[c] = Ifit[c];
        const double imax[NCIRC] = { m.coil_imax_At[0], m.coil_imax_At[1], m.coil_imax_At[3],
                                     m.coil_imax_At[5], m.coil_imax_At[7], m.coil_imax_At[9] };
        out.imax_frac = 0;
        for (int c = 0; c < NCIRC; ++c)
            out.imax_frac = std::fmax(out.imax_frac, std::fabs(Ifit[c]) / imax[c]);
        out.eq.psi_axis = st.psi_axis; out.eq.psi_bnd = st.psi_bnd;
        out.eq.R_axis = g.R(st.ax_ir); out.eq.Z_axis = g.Z(st.ax_iz);
        out.eq.picard_iters = n_outer * n_picard;
        out.Rx = st.Rx; out.Zx = std::fabs(st.Zx); out.psi_x = st.psi_x;
        out.x_limited = st.x_limited;
        out.shift_mm = (out.eq.R_axis - m.R0) * 1e3;
        out.drift_axis = st.drift;
        {
            const double dA = g.dR * g.dZ;
            double ip = 0;
            for (int iz = 1; iz < NZ - 1; ++iz)
                for (int ir = 1; ir < NR - 1; ++ir) ip += st.J[size_t(iz) * NR + ir] * dA;
            out.eq.Ip_A = ip;
        }
        // achieved LCFS shape by ray bisection (axis -> wall) on psi_tot
        auto psi_at = [&](double R, double Z) {
            const double fr = (R - g.Rmin) / g.dR, fz = (Z - g.Zmin) / g.dZ;
            const int ir = std::clamp(int(fr), 0, NR - 2);
            const int iz = std::clamp(int(fz), 0, NZ - 2);
            const double tr = fr - ir, tz = fz - iz;
            const size_t k = size_t(iz) * NR + ir;
            return (1 - tr) * (1 - tz) * st.psi_tot[k] + tr * (1 - tz) * st.psi_tot[k + 1]
                 + (1 - tr) * tz * st.psi_tot[k + NR] + tr * tz * st.psi_tot[k + NR + 1];
        };
        auto lcfs_r = [&](double th, double target) {
            double bR, bZ;
            miller_pt(m, th, m.wall_over_a, bR, bZ);       // ray endpoint on the wall
            // FIRST crossing walk (psi is non-monotonic past the X — bisection can
            // land in private flux; measured), then a short local refine
            const int NS = 400;
            double prevf = 0.0, prevp = st.psi_axis;
            for (int s = 1; s <= NS; ++s) {
                const double f = double(s) / NS;
                const double R = out.eq.R_axis + f * (bR - out.eq.R_axis);
                const double Z = out.eq.Z_axis + f * (bZ - out.eq.Z_axis);
                const double p = psi_at(R, Z);
                if (p >= target) {
                    const double t = (target - prevp) / (p - prevp > 1e-30 ? p - prevp : 1e-30);
                    return prevf + (f - prevf) * std::clamp(t, 0.0, 1.0);
                }
                prevf = f; prevp = p;
            }
            return 1.0;
        };
        {
            const double t = st.psi_bnd;
            const double fo = lcfs_r(0.0, t), fi = lcfs_r(PI(), t);
            double RoR, RoZ, RiR, RiZ;
            miller_pt(m, 0.0, m.wall_over_a, RoR, RoZ);
            miller_pt(m, PI(), m.wall_over_a, RiR, RiZ);
            const double Rout = out.eq.R_axis + fo * (RoR - out.eq.R_axis);
            const double Rin = out.eq.R_axis + fi * (RiR - out.eq.R_axis);
            out.a_ach = 0.5 * (Rout - Rin);
            const double Rgeo = 0.5 * (Rout + Rin);
            out.kappa_ach = (out.a_ach > 0 && st.have_x) ? out.Zx / out.a_ach : 0.0;
            out.delta_ach = (out.a_ach > 0 && st.have_x) ? (Rgeo - out.Rx) / out.a_ach : 0.0;
        }
        // q by theta-ray contours (F = R0*B0 stated, as the fixed solver)
        auto grad_at = [&](double R, double Z, double& gR, double& gZ) {
            const double h = 0.5 * g.dR;
            gR = (psi_at(R + h, Z) - psi_at(R - h, Z)) / (2 * h);
            gZ = (psi_at(R, Z + h) - psi_at(R, Z - h)) / (2 * h);
        };
        auto q_of = [&](double pbar) {
            const double target = st.psi_axis + pbar * (st.psi_bnd - st.psi_axis);
            const int NTH = 128;
            double integ = 0, prevR = 0, prevZ = 0;
            for (int k = 0; k <= NTH; ++k) {
                const double th = 2.0 * PI() * k / NTH;
                const double f = lcfs_r(th, target);
                double bR, bZ; miller_pt(m, th, m.wall_over_a, bR, bZ);
                const double R = out.eq.R_axis + f * (bR - out.eq.R_axis);
                const double Z = out.eq.Z_axis + f * (bZ - out.eq.Z_axis);
                if (k == 0) { prevR = R; prevZ = Z; continue; }
                const double dl = std::sqrt((R - prevR) * (R - prevR) + (Z - prevZ) * (Z - prevZ));
                double gR, gZ; grad_at(0.5 * (R + prevR), 0.5 * (Z + prevZ), gR, gZ);
                const double gp = std::sqrt(gR * gR + gZ * gZ);
                integ += dl / (0.5 * (R + prevR) * (gp > 1e-9 ? gp : 1e-9));
                prevR = R; prevZ = Z;
            }
            return (m.R0 * m.B0) / (2.0 * PI()) * integ;
        };
        out.q95 = q_of(0.95);
        out.q0 = q_of(0.10);
        // k_dest from the COIL field (the true external field at equilibrium)
        {
            const VertDerived v = derive_at_axis(st, out.eq.Ip_A);
            out.Bz_ext_axis_T = v.Bz_ext_axis_T;
            out.n_decay = v.n_decay;
            out.k_dest_Npm = v.k_dest_Npm;
        }
        if (keep) *keep = st;
        (void)mu0;
        return out;
    }
};

inline FreeEq gs_free_solve(const MachineCfg& m, bool verbose = false) {
    GsFreeSolver s;
    s.init(m);
    return s.solve(m, 4, 30, verbose);
}

// D-041: the shared free-boundary runtime context — tables + the converged seed state,
// built ONCE per process by Monte-Carlo callers and shared read-only across runs
// (run_sim copies the seed state per run; ~170 KB). Pure function of the machine.
struct FreeContext {
    GsFreeSolver solver;               // tables const after init (read-only use)
    GsFreeSolver::PicardState seed;    // the converged flat-top state
    FreeEq ref;                        // the reference equilibrium (reported numbers)
};
inline std::shared_ptr<const FreeContext> gs_free_context(const MachineCfg& m) {
    auto ctx = std::make_shared<FreeContext>();
    ctx->solver.init(m);
    ctx->ref = ctx->solver.solve(m, 4, 30, false, &ctx->seed);
    return ctx;
}

} // namespace fusion
