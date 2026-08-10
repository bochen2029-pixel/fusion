// core/vertical.h — M1: THE ENEMY. The linearized vertical instability over the real
// passive-conductor circuit model (spec v0.2 §2.1 tier-1, first element).
//
// Model (scaling-grade, honesty-labeled; receipts state every derived constant):
//   - plasma = rigid current ring (Ip) at (R0, Z), effective inertia m_eff (the massless
//     limit's numerical regularization — computed to pin the screened-oscillation
//     artifact near a stated frequency; the wall-mode gamma is insensitive to it — the
//     ctest band enforces that)
//   - destabilizing force gradient k_dest [N/m] — DERIVED from the shaped equilibrium's
//     external-field decay index (gs_vertical_derive, D-038) and passed in; this file
//     never invents it. Report-don't-configure, end-to-end (PHY-03).
//   - 24 passive filaments on the vessel shell (machine.toml [vessel]) with the FULL
//     mutual-inductance matrix (Maxwell ring mutuals; diagonal = ring self-inductance at
//     an effective wire radius of half the filament spacing — the standard shell
//     discretization). One resistive scalar (a uniform surface resistivity) calibrated
//     so the vertical-SCREENING eigenmode carries the pinned vessel tau_wall; the mode
//     spectrum, k_wall, and gamma are then derived and REPORTED. (Slice-1's diagonal
//     approximation under-carried the collective screening ~20x — measured, receipted.)
//   - one in-vessel VS pair (machine.toml [coils] VS1): the 10 kHz actuator, coupled to
//     the shell through the same matrix (it transformer-drives vessel eddies — physical)
//   - gamma is REPORTED from the dominant eigenvalue of the coupled linear system
//     (RK4 log-slope, fixed count — deterministic), NEVER configured (PHY-03).
//
// The EKF (D-023): a DELIBERATELY REDUCED 2-state model (Z, V) — the pre-registered
// epistemic handicap — over 4 synthetic magnetic probes (sigma + bias walk per
// contracts/diagnostics.toml). Innovation feeds the tokenizer's [innovation] vertical
// cluster through the events.toml windowed gate. NIS chi^2 is the acceptance statistic.
#pragma once
#include "config.h"
#include "rings.h"
#include "philox.h"
#include <cmath>
#include <algorithm>
#include <vector>

namespace fusion {

struct VerticalModel {
    // layout: x = [Z, V, I_1..I_24 (passive), I_vs] ; n = 27; circuits = 25
    static constexpr int NP = 24, NC = NP + 1, N = NP + 3;
    double A[N][N];                 // dx/dt = A x + B*Vcmd
    double B[N];
    double m_eff = 30.0;            // kg — the regularization; COMPUTED in build() so the
                                    // screened fast oscillation sits near F_SCREEN_HZ
                                    // (10 kHz-controllable, EKF-trackable), floored at
                                    // M_EFF_FLOOR; documented, gamma-insensitive (ctest)
    static constexpr double F_SCREEN_HZ = 180.0, M_EFF_FLOOR = 5.0, ZETA_SCREEN = 0.7;
    double d_eff = 0.0;             // N·s/m — the regularization's matching damping
                                    // (D-040): fake inertia with NO fake damping left
                                    // the screened artifact RESONANT — the noisy loop
                                    // pumped it into a 179 Hz, ±60-90 mm standing
                                    // limit cycle (traced, receipted; S11's "hold"
                                    // contained the smaller version). ZETA_SCREEN
                                    // critically damps the artifact BY CONSTRUCTION;
                                    // the slow mode's gamma shifts ~7% and is, as
                                    // always, REPORTED from the eigenproblem.
    double k_dest = 0.0;            // N/m — DERIVED input (gs_vertical_derive; D-038)
    double c_p[NP];                 // force per passive ampere = Ip * dM_pj/dZ [N/A]
    double c_vs = 0.0;
    double Mfull[NC][NC];           // circuit inductance matrix [filaments..., VS]
    double Minv[NC][NC];
    double Rcirc[NC];               // circuit resistances (vessel scalar calibrated)
    double Lvs = 2.0e-3, Rvs = 0.2;  // in-vessel pair: tau 10 ms, 10 kA at 2 kV
    double vs_turns = 20.0;          // overwritten from machine.toml [coils] turns (D-039)
    double kwall_ = 0.0;             // instantaneous screening stiffness c^T Minv c (rep.)
    double rho_shell = 0.0;          // the calibrated surface-resistivity scalar (rep.)
    double screen_[NC] = {0};        // flux-conserving current pattern per meter (D-039)
    double tau_wall_used = 0.025;    // the (possibly scattered) vessel tau this build used
    double gamma_open = 0.0;        // reported: no-shell growth rate [1/s] (VS passive)
    double gamma_wall = 0.0;        // reported: with-shell growth rate [1/s]

    // ---- 25x25 SPD Cholesky (fixed order — deterministic) --------------------------
    static bool chol(int n, const double* Min, double* L) {   // row-major n x n
        for (int i = 0; i < n * n; ++i) L[i] = 0.0;
        for (int i = 0; i < n; ++i)
            for (int j = 0; j <= i; ++j) {
                double s = Min[i * n + j];
                for (int k = 0; k < j; ++k) s -= L[i * n + k] * L[j * n + k];
                if (i == j) {
                    if (s <= 0.0) return false;               // not PD — hard model error
                    L[i * n + i] = std::sqrt(s);
                } else L[i * n + j] = s / L[j * n + j];
            }
        return true;
    }
    static void chol_solve(int n, const double* L, const double* b, double* x) {
        std::vector<double> y(n);
        for (int i = 0; i < n; ++i) {
            double s = b[i];
            for (int k = 0; k < i; ++k) s -= L[i * n + k] * y[k];
            y[i] = s / L[i * n + i];
        }
        for (int i = n - 1; i >= 0; --i) {
            double s = y[i];
            for (int k = i + 1; k < n; ++k) s -= L[k * n + i] * x[k];
            x[i] = s / L[i * n + i];
        }
    }

    // tw_scale / crs_scale: per-seed plant_scatter truths (dispersions.toml — vessel
    // time-constant and actuated-circuit-resistance tolerances). The plant is built
    // SCATTERED; the EKF inits from a NOMINAL build (scales 1) — that gap IS D-023's
    // pre-registered information-set mismatch, finally consumed (D-040).
    void build(const MachineCfg& m, double k_dest_derived_Npm,
               double tw_scale = 1.0, double crs_scale = 1.0) {
        const double PI = 3.14159265358979;
        const double b_sh = m.a * m.b_over_a;               // machine.toml [vessel]
        const double kap_sh = m.kappa_shell;
        const double tau_w = m.tau_wall_s * tw_scale;
        tau_wall_used = tau_w;
        k_dest = k_dest_derived_Npm;
        // filament ring positions on the shell
        double Rf[NP], Zf[NP];
        for (int j = 0; j < NP; ++j) {
            const double th = 2.0 * PI * (j + 0.5) / NP;
            Rf[j] = m.R0 + b_sh * std::cos(th);
            Zf[j] = b_sh * kap_sh * std::sin(th);
        }
        // local filament spacing along the shell (aw = spacing/2 — the standard shell
        // discretization; slice-1's thin-wire 0.02 m overstated the diagonal)
        double aw[NP], ds[NP];
        for (int j = 0; j < NP; ++j) {
            const int jp = (j + 1) % NP, jm = (j + NP - 1) % NP;
            const double dp = std::hypot(Rf[jp] - Rf[j], Zf[jp] - Zf[j]);
            const double dm = std::hypot(Rf[j] - Rf[jm], Zf[j] - Zf[jm]);
            ds[j] = 0.5 * (dp + dm);
            aw[j] = 0.5 * ds[j];
        }
        // plasma-circuit coupling: dM/dZ by symmetric difference at Z=0 (fixed h)
        const double h = 1e-3, Ip = m.Ip_MA * 1e6;
        for (int j = 0; j < NP; ++j) {
            const double dM = (ring_mutual(m.R0, Rf[j], Zf[j] - h) -
                               ring_mutual(m.R0, Rf[j], Zf[j] + h)) / (2.0 * h);
            c_p[j] = Ip * dM;                        // N/A (and V·s/m reciprocally)
        }
        // VS pair (machine.toml [coils] VS1 row — one source, D-039), anti-series:
        // effective dM/dZ doubled
        const double Rvs_r = m.coil_r[11], Zvs = m.coil_z[11];
        vs_turns = m.coil_turns[11];
        const double dMvs = (ring_mutual(m.R0, Rvs_r, Zvs - h) -
                             ring_mutual(m.R0, Rvs_r, Zvs + h)) / (2.0 * h);
        c_vs = 2.0 * vs_turns * Ip * dMvs;           // anti-series multi-turn pair
        // ---- the FULL circuit inductance matrix (filaments + VS; D-038) ------------
        for (int i = 0; i < NP; ++i)
            for (int j = 0; j < NP; ++j)
                Mfull[i][j] = (i == j) ? ring_self(Rf[i], aw[i])
                                       : ring_mutual(Rf[i], Rf[j], Zf[i] - Zf[j]);
        for (int j = 0; j < NP; ++j) {               // VS circuit <-> filament (anti-series)
            const double mv = vs_turns * (ring_mutual(Rvs_r, Rf[j], Zvs - Zf[j]) -
                                          ring_mutual(Rvs_r, Rf[j], -Zvs - Zf[j]));
            Mfull[NP][j] = Mfull[j][NP] = mv;
        }
        Mfull[NP][NP] = Lvs;                         // pinned pair inductance
        // ---- vessel resistances: ONE surface-resistivity scalar, calibrated so the
        // vertical-screening eigenmode (pattern Mff^-1 c_f) carries the PINNED tau_wall.
        // tau_wall is machine.toml's physical input; everything downstream is derived.
        {
            std::vector<double> Mff(NP * NP), Lf(NP * NP), u(NP), cf(NP);
            for (int i = 0; i < NP; ++i)
                for (int j = 0; j < NP; ++j) Mff[i * NP + j] = Mfull[i][j];
            for (int j = 0; j < NP; ++j) cf[j] = c_p[j];
            const bool ok = chol(NP, Mff.data(), Lf.data());
            if (ok) chol_solve(NP, Lf.data(), cf.data(), u.data());
            else    for (int j = 0; j < NP; ++j) u[j] = c_p[j];   // degenerate fallback
            double uMu = 0, uRu = 0;
            for (int i = 0; i < NP; ++i) {
                for (int j = 0; j < NP; ++j) uMu += u[i] * Mfull[i][j] * u[j];
                uRu += u[i] * (2.0 * PI * Rf[i] / ds[i]) * u[i];  // geometry factor
            }
            rho_shell = (uRu > 0) ? uMu / (uRu * tau_w) : 1e-6;   // [ohm per square]
            for (int j = 0; j < NP; ++j) Rcirc[j] = rho_shell * 2.0 * PI * Rf[j] / ds[j];
        }
        Rcirc[NP] = Rvs * crs_scale;
        // ---- invert the full matrix (Cholesky columns; deterministic) --------------
        {
            std::vector<double> Mlin(NC * NC), Llin(NC * NC), e(NC), col(NC);
            for (int i = 0; i < NC; ++i)
                for (int j = 0; j < NC; ++j) Mlin[i * NC + j] = Mfull[i][j];
            const bool ok = chol(NC, Mlin.data(), Llin.data());
            for (int j = 0; j < NC; ++j) {
                for (int i = 0; i < NC; ++i) e[i] = (i == j) ? 1.0 : 0.0;
                if (ok) chol_solve(NC, Llin.data(), e.data(), col.data());
                else    for (int i = 0; i < NC; ++i) col[i] = (i == j) ? 1.0 / Mfull[j][j] : 0.0;
                for (int i = 0; i < NC; ++i) Minv[i][j] = col[i];
            }
        }
        // instantaneous screening stiffness (superconducting-circuits limit), REPORTED
        double cc[NC];
        for (int j = 0; j < NP; ++j) cc[j] = c_p[j];
        cc[NP] = c_vs;
        kwall_ = 0.0;
        for (int i = 0; i < NC; ++i)
            for (int j = 0; j < NC; ++j) kwall_ += cc[i] * Minv[i][j] * cc[j];
        // the flux-conserving screening pattern (per meter of displacement): a physical
        // sudden-displacement disturbance co-moves the circuit currents by -screen_*dz
        // (D-039: a bare z teleport stores (1/2)(kwall-k_dest)dz^2 of ARTIFICIAL energy
        // in the regularization's screened oscillator and rings it at ~omega*dz — the
        // artifact killed curriculum seeds once the amended vessel weakened the screen;
        // measured, receipted. On the slow manifold the excursion grows at gamma_wall —
        // the true VDE onset.)
        for (int i = 0; i < NC; ++i) {
            double s = 0.0;
            for (int j = 0; j < NC; ++j) s += Minv[i][j] * cc[j];
            screen_[i] = s;
        }
        // the regularization: pin the screened-oscillation artifact near F_SCREEN_HZ
        // (documented; if the shell cannot hold the shape at all — kwall <= k_dest —
        // the mode is beyond passive stabilization and the class test must catch it)
        const double w_t = 2.0 * PI * F_SCREEN_HZ;
        m_eff = (kwall_ > k_dest) ? std::max(M_EFF_FLOOR, (kwall_ - k_dest) / (w_t * w_t))
                                  : 30.0;
        d_eff = 2.0 * ZETA_SCREEN * std::sqrt(m_eff * std::max(1e3, kwall_ - k_dest));
        // ---- assemble A: M dI/dt = -R I - c dZ/dt + e_vs Vcmd ----------------------
        for (int i = 0; i < N; ++i) { B[i] = 0; for (int j = 0; j < N; ++j) A[i][j] = 0; }
        A[0][1] = 1.0;
        A[1][1] = -d_eff / m_eff;
        A[1][0] = k_dest / m_eff;
        for (int j = 0; j < NP; ++j) A[1][2 + j] = c_p[j] / m_eff;
        A[1][2 + NP] = c_vs / m_eff;
        for (int i = 0; i < NC; ++i) {
            double mc = 0.0;
            for (int j = 0; j < NC; ++j) {
                A[2 + i][2 + j] = -Minv[i][j] * Rcirc[j];
                mc += Minv[i][j] * cc[j];
            }
            A[2 + i][1] = -mc;
            B[2 + i] = Minv[i][NP];                  // VS drive leaks into the shell (phys.)
        }
        gamma_wall = dominant_growth(true);
        gamma_open = dominant_growth(false);
    }

    // dominant REAL growth rate: RK4-integrate at fine dt with periodic renormalization;
    // the growth is the log-norm slope over the SECOND half of the window, where the
    // unstable real mode has outgrown every transient. Deterministic (fixed dt, fixed
    // step count). with_passives=false integrates the explicit 3-state [Z,V,I_vs]
    // system (shell removed; the VS pair still screens passively — reported as such).
    // A forward-Euler map power iteration fails here — it inflates the fast oscillatory
    // mode (a discretization artifact, receipted at slice 1).
    double dominant_growth(bool with_passives) const {
        const double dt = 2e-5, T = with_passives ? 0.6 : 0.02;
        const long nsteps = (long)(T / dt);
        const int n = with_passives ? N : 3;
        double x[N]; for (int i = 0; i < n; ++i) x[i] = (i == 0) ? 1e-3 : 0.0;
        double logn = 0.0, logn_half = 0.0;
        auto deriv = [&](const double* s, double* d) {
            if (with_passives) {
                for (int i = 0; i < N; ++i) {
                    double v = 0;
                    for (int j = 0; j < N; ++j) v += A[i][j] * s[j];
                    d[i] = v;
                }
            } else {                                  // [Z, V, I_vs] alone — the UNDAMPED
                d[0] = s[1];                          // inertial reference (the artifact
                d[1] = (k_dest * s[0] + c_vs * s[2]) / m_eff;   // damper is a closed-loop
                d[2] = (-Rcirc[NP] * s[2] - c_vs * s[1]) / Lvs; // device; excluded here)
            }
        };
        for (long it = 0; it < nsteps; ++it) {
            double k1[N], k2[N], k3[N], k4[N], t[N];
            deriv(x, k1);
            for (int i = 0; i < n; ++i) t[i] = x[i] + 0.5 * dt * k1[i];
            deriv(t, k2);
            for (int i = 0; i < n; ++i) t[i] = x[i] + 0.5 * dt * k2[i];
            deriv(t, k3);
            for (int i = 0; i < n; ++i) t[i] = x[i] + dt * k3[i];
            deriv(t, k4);
            for (int i = 0; i < n; ++i)
                x[i] += dt / 6.0 * (k1[i] + 2 * k2[i] + 2 * k3[i] + k4[i]);
            if ((it & 127) == 127) {
                double nrm = 0; for (int i = 0; i < n; ++i) nrm += x[i] * x[i];
                nrm = std::sqrt(nrm > 0 ? nrm : 1e-300);
                logn += std::log(nrm);
                for (int i = 0; i < n; ++i) x[i] /= nrm;
            }
            if (it == nsteps / 2) logn_half = logn;
        }
        { double nrm = 0; for (int i = 0; i < n; ++i) nrm += x[i] * x[i];
          logn += 0.5 * std::log(nrm > 0 ? nrm : 1e-300); }
        return (logn - logn_half) / (T * 0.5);
    }

    // physical sudden-displacement disturbance: slow-manifold state jump (see build())
    void kick_state(double x[N], double dz) const {
        x[0] += dz;
        for (int i = 0; i < NC; ++i) x[2 + i] -= screen_[i] * dz;
    }

    void step_rk4(double x[N], double Vcmd, double dt) const {
        auto rhs = [&](const double* s, double* d) {
            for (int i = 0; i < N; ++i) {
                double v = B[i] * Vcmd;
                for (int j = 0; j < N; ++j) v += A[i][j] * s[j];
                d[i] = v;
            }
        };
        double k1[N], k2[N], k3[N], k4[N], t[N];
        rhs(x, k1);
        for (int i = 0; i < N; ++i) t[i] = x[i] + 0.5 * dt * k1[i];
        rhs(t, k2);
        for (int i = 0; i < N; ++i) t[i] = x[i] + 0.5 * dt * k2[i];
        rhs(t, k3);
        for (int i = 0; i < N; ++i) t[i] = x[i] + dt * k3[i];
        rhs(t, k4);
        for (int i = 0; i < N; ++i)
            x[i] += dt / 6.0 * (k1[i] + 2 * k2[i] + 2 * k3[i] + k4[i]);
    }
};

// ---- the model-based EKF (D-023 full; D-040 — replaces the slice-1 α-β stand-in) ---
// A 4-state linear Kalman on the REDUCED vertical model, built from the same DERIVED
// constants the plant reports (k_dest, kwall, m_eff, tau_wall, c_vs, Lvs, Rvs):
//   x = [z, v, q_s, I_vs];  q_s = the collective screening mode (internal scale
//   L_s = 1 H; c_s = sqrt(kwall - c_vs^2/Lvs) so the instantaneous stiffness and the
//   pinned tau_wall are carried exactly; the 24-filament spectrum's higher harmonics
//   are the reduction — covered by pre-registered process noise, per the S9 handoff
//   law: the reduced model must NOT carry raw unstable stiffness on the slow manifold
//   — and it does not: the slow manifold is IN the model, the tail is in Q).
// Information set (D-023): NOMINAL constants (the plant is scattered — that gap is
// the mismatch), the controller's own Vcmd, and ONLY diagnostics.toml channels:
// z (magnetics: sigma + 2-tick latency, unmodeled bias walk — the filter follows it,
// stated) and I_vs ([coil_sensors]: 0.2% FS, 1-tick latency). Sequential scalar
// updates. Discretization: 4th-order series in 4 substeps (Euler was unstable at the
// screened mode — slice-1's measured lesson; series error ~1e-9 at these eigenvalues).
// NIS chi^2 on the z channel is the acceptance statistic; the [innovation] gate feeds
// on the same (nu, S) — the seam's units are finally calibrated sigma.
struct VerticalEKF {
    static constexpr int NS = 4;
    double x[NS] = {0};
    double P[NS][NS] = {0};
    double Phi[NS][NS] = {0}, Bd[NS] = {0};
    double Qd[NS] = {0};            // diagonal discrete process noise (pre-registered)
    double Rz = 0, Ri = 0;
    double xz = 0, xv = 0;          // mirrors of x[0], x[1] (controller taps)
    double nis_sum = 0; long nis_n = 0;
    double innov_last = 0, S_last = 1;

    void init(const VerticalModel& nom, double sigma_z_m, double sigma_i_A) {
        const double m_ = nom.m_eff, kd = nom.k_dest, cvs = nom.c_vs;
        const double Lv = nom.Lvs, Rv = nom.Rvs;            // nominal actuated circuit
        const double kwf = std::max(1e3, nom.kwall_ - cvs * cvs / Lv);
        const double cs = std::sqrt(kwf);                   // L_s = 1 convention
        // continuous A, B. The single collective mode is MOMENT-MATCHED: it carries
        // the exact instantaneous stiffness (cs^2 above), and its decay tau_s is
        // CALIBRATED so the reduced 4-state's slow pole equals the plant's REPORTED
        // gamma_wall — the multi-mode spectrum leaks ~1.5x faster than the pinned
        // tau_wall alone suggests (measured: single-tau gave gamma 34 vs 50 /s and
        // the filter lagged the growth systematically, NIS ~120 in-loop). Both
        // numbers are derived from the same NOMINAL model — the information set is
        // unchanged (D-023). Bisection below is deterministic, fixed count.
        double A[NS][NS] = {0}, B[NS] = {0};
        A[0][1] = 1.0;
        A[1][0] = kd / m_; A[1][1] = -nom.d_eff / m_;
        A[1][2] = cs / m_; A[1][3] = cvs / m_;
        A[2][1] = -cs;
        A[3][1] = -cvs / Lv; A[3][3] = -Rv / Lv;
        B[3] = 1.0 / Lv;
        {
            const double gam_t = nom.gamma_wall > 1.0 ? nom.gamma_wall : 1.0;
            auto slow_pole = [&](double tau_s) {           // dominant growth of the 4x4
                A[2][2] = -1.0 / tau_s;
                double xs[NS] = { 1e-3, 0, 0, 0 };
                const double dts = 2e-5; const int nst = 15000;   // 0.3 s window
                double logn = 0, logn_half = 0;
                for (int it = 0; it < nst; ++it) {
                    double k1[NS], k2[NS], k3[NS], k4[NS], tt[NS];
                    auto der = [&](const double* s2, double* d2) {
                        for (int i = 0; i < NS; ++i) {
                            double v2 = 0;
                            for (int j2 = 0; j2 < NS; ++j2) v2 += A[i][j2] * s2[j2];
                            d2[i] = v2;
                        }
                    };
                    der(xs, k1);
                    for (int i = 0; i < NS; ++i) tt[i] = xs[i] + 0.5 * dts * k1[i];
                    der(tt, k2);
                    for (int i = 0; i < NS; ++i) tt[i] = xs[i] + 0.5 * dts * k2[i];
                    der(tt, k3);
                    for (int i = 0; i < NS; ++i) tt[i] = xs[i] + dts * k3[i];
                    der(tt, k4);
                    for (int i = 0; i < NS; ++i)
                        xs[i] += dts / 6.0 * (k1[i] + 2 * k2[i] + 2 * k3[i] + k4[i]);
                    if ((it & 127) == 127) {
                        double nr = 0; for (int i = 0; i < NS; ++i) nr += xs[i] * xs[i];
                        nr = std::sqrt(nr > 0 ? nr : 1e-300);
                        logn += std::log(nr);
                        for (int i = 0; i < NS; ++i) xs[i] /= nr;
                    }
                    if (it == nst / 2) logn_half = logn;
                }
                double nr = 0; for (int i = 0; i < NS; ++i) nr += xs[i] * xs[i];
                logn += 0.5 * std::log(nr > 0 ? nr : 1e-300);
                return (logn - logn_half) / (nst * dts * 0.5);
            };
            double lo = 0.004, hi = 0.040;                 // gamma decreases with tau_s
            for (int b = 0; b < 18; ++b) {
                const double mid = 0.5 * (lo + hi);
                (slow_pole(mid) > gam_t) ? lo = mid : hi = mid;
            }
            A[2][2] = -1.0 / (0.5 * (lo + hi));            // the calibrated mode decay
        }
        // discrete Phi, Bd: 4 substeps of the 4th-order series (deterministic)
        const double dt = 1e-4, h = dt / 4.0;
        double Ph[NS][NS] = {0}, Bh[NS] = {0};
        {   // series at h
            double A2[NS][NS] = {0}, A3[NS][NS] = {0};
            for (int i = 0; i < NS; ++i)
                for (int j2 = 0; j2 < NS; ++j2)
                    for (int k = 0; k < NS; ++k) A2[i][j2] += A[i][k] * A[k][j2];
            for (int i = 0; i < NS; ++i)
                for (int j2 = 0; j2 < NS; ++j2)
                    for (int k = 0; k < NS; ++k) A3[i][j2] += A2[i][k] * A[k][j2];
            for (int i = 0; i < NS; ++i)
                for (int j2 = 0; j2 < NS; ++j2)
                    Ph[i][j2] = (i == j2 ? 1.0 : 0.0) + h * A[i][j2]
                              + h * h / 2.0 * A2[i][j2] + h * h * h / 6.0 * A3[i][j2];
            for (int i = 0; i < NS; ++i)
                Bh[i] = h * B[i] + h * h / 2.0 * (A[i][3] * B[3])
                      + h * h * h / 6.0 * (A2[i][3] * B[3]);
        }
        // compose 4 substeps: Phi = Ph^4, Bd = (Ph^3 + Ph^2 + Ph + I) Bh
        double P2[NS][NS] = {0}, P3[NS][NS] = {0};
        for (int i = 0; i < NS; ++i)
            for (int j2 = 0; j2 < NS; ++j2)
                for (int k = 0; k < NS; ++k) P2[i][j2] += Ph[i][k] * Ph[k][j2];
        for (int i = 0; i < NS; ++i)
            for (int j2 = 0; j2 < NS; ++j2)
                for (int k = 0; k < NS; ++k) P3[i][j2] += P2[i][k] * Ph[k][j2];
        for (int i = 0; i < NS; ++i)
            for (int j2 = 0; j2 < NS; ++j2) {
                Phi[i][j2] = 0;
                for (int k = 0; k < NS; ++k) Phi[i][j2] += P2[i][k] * P2[k][j2];
            }
        for (int i = 0; i < NS; ++i) {
            Bd[i] = Bh[i];
            for (int j2 = 0; j2 < NS; ++j2)
                Bd[i] += (Ph[i][j2] + P2[i][j2] + P3[i][j2]) * Bh[j2];
        }
        // pre-registered noise (calibrated ONCE on nominal seeds, frozen — receipted):
        // q_v covers the mode-spectrum reduction as force noise — the single collective
        // mode mislocates the FAST (artifact) frequency by O(10-30%), and in closed
        // loop the noise-pumped artifact makes that the dominant innovation source
        // (measured: NIS 2 as observer, ~200 in-loop at sigma_a 300); sigma_a is set
        // to the unmodeled-artifact acceleration scale ~ 0.2*omega^2*z_dance. q_qs
        // covers the mode amplitude, q_i the drive/resistance tolerance.
        Qd[0] = 1e-12;
        Qd[1] = (3000.0 * dt) * (3000.0 * dt);
        Qd[2] = (0.2 * cs * 0.3 * dt) * (0.2 * cs * 0.3 * dt);
        Qd[3] = (1.5e4 * dt) * (1.5e4 * dt);
        Rz = sigma_z_m * sigma_z_m;
        Ri = sigma_i_A * sigma_i_A;
        for (int i = 0; i < NS; ++i)
            for (int j2 = 0; j2 < NS; ++j2) P[i][j2] = 0;
        P[0][0] = 9e-6; P[1][1] = 1e-2; P[2][2] = 1e2; P[3][3] = 1e4;
        for (int i = 0; i < NS; ++i) x[i] = 0;
        xz = xv = 0;
    }

    void predict(double Vcmd, double dt_unused) {
        (void)dt_unused;                                    // Phi is pinned at 1e-4
        double xn[NS] = {0};
        for (int i = 0; i < NS; ++i) {
            for (int j2 = 0; j2 < NS; ++j2) xn[i] += Phi[i][j2] * x[j2];
            xn[i] += Bd[i] * Vcmd;
        }
        for (int i = 0; i < NS; ++i) x[i] = xn[i];
        double PP[NS][NS] = {0};
        for (int i = 0; i < NS; ++i)                        // P = Phi P Phi^T + Q
            for (int j2 = 0; j2 < NS; ++j2) {
                double s = 0;
                for (int k = 0; k < NS; ++k) s += Phi[i][k] * P[k][j2];
                PP[i][j2] = s;
            }
        for (int i = 0; i < NS; ++i)
            for (int j2 = 0; j2 < NS; ++j2) {
                double s = 0;
                for (int k = 0; k < NS; ++k) s += PP[i][k] * Phi[j2][k];
                P[i][j2] = s + (i == j2 ? Qd[i] : 0.0);
            }
        xz = x[0]; xv = x[1];
    }
    void scalar_update(int row, double meas, double Rm, bool count_nis) {
        const double nu = meas - x[row];
        const double S = P[row][row] + Rm;
        if (count_nis) {
            innov_last = nu; S_last = S;
            nis_sum += nu * nu / S; ++nis_n;
        }
        double K[NS];
        for (int i = 0; i < NS; ++i) K[i] = P[i][row] / S;
        for (int i = 0; i < NS; ++i) x[i] += K[i] * nu;
        double Pn[NS][NS];
        for (int i = 0; i < NS; ++i)                        // (I - K H) P, symmetrized
            for (int j2 = 0; j2 < NS; ++j2)
                Pn[i][j2] = P[i][j2] - K[i] * P[row][j2];
        for (int i = 0; i < NS; ++i)
            for (int j2 = 0; j2 < NS; ++j2)
                P[i][j2] = 0.5 * (Pn[i][j2] + Pn[j2][i]);
        xz = x[0]; xv = x[1];
    }
    void update(double z_meas) { scalar_update(0, z_meas, Rz, true); }   // magnetics
    void update_i(double i_meas) { scalar_update(3, i_meas, Ri, false); } // coil sensor
    double nis_mean() const { return nis_n ? nis_sum / double(nis_n) : 0.0; }
};

// ---- the [innovation] gate (contracts/events.toml [innovation], vertical cluster) --
struct InnovGate {
    double win_sum = 0, win_max = 0; long win_n = 0; long win_ticks = 0;
    int dwell = 0; long refractory_ticks = 0;
    double floor_ = 0;              // self-calibrating nominal floor (D-023's
                                    // NIS-calibration clause, minimal slice-1 form:
                                    // sigma-units are RATIOS to this EMA floor)
    // returns |cluster residual| in sigma when an event should fire, else 0.
    // Two tiers (events.toml [innovation], D-035): sigma_alarm fires single-window
    // (fast transients — a kick lives in ONE window); sigma_token needs the dwell.
    // The window statistic is mean |nu|/sqrt(S) vs its noise expectation (~0.8), so an
    // oscillatory transient cannot cancel itself out of the window.
    double feed(double nu, double S, long window_ticks, double sigma_token,
                double sigma_alarm, int dwell_windows, long refr_ticks) {
        const double a = std::fabs(nu) / std::sqrt(S > 0 ? S : 1);
        win_sum += a; win_max = std::fmax(win_max, a); ++win_n; ++win_ticks;
        if (refractory_ticks > 0) --refractory_ticks;
        if (win_ticks < window_ticks) return 0.0;
        const double mean_abs = win_sum / (double)(win_n ? win_n : 1);
        const double wmax = win_max;
        win_sum = 0; win_max = 0; win_n = 0; win_ticks = 0;
        if (floor_ <= 0) { floor_ = mean_abs; return 0.0; }    // first window seeds it
        const double mag_mean = mean_abs / floor_;             // sigma-units = ratio to
        const double mag_max = wmax / (1.8 * floor_);          // the calibrated floor
        // (1.8: nominal window peaks sit ~3.4x the mean floor -> ~1.9 in these units;
        //  a 25 mm kick peaks ~10x -> ~5.6, clearing sigma_alarm=5. Receipted.)
        // fast tier: one window's PEAK stands sigma_alarm-fold over the (3x) floor —
        // a kick lives here. slow tier: sustained mean elevation with dwell.
        double fired = 0.0;
        if (refractory_ticks == 0 && mag_max >= sigma_alarm) {
            dwell = 0; refractory_ticks = refr_ticks; fired = mag_max;
        } else if (mag_mean >= sigma_token) {
            if (++dwell >= dwell_windows && refractory_ticks == 0) {
                dwell = 0; refractory_ticks = refr_ticks; fired = mag_mean;
            }
        } else dwell = 0;
        if (fired == 0.0) floor_ += 0.05 * (mean_abs - floor_);   // adapt on quiet only
        return fired;
    }
};

} // namespace fusion
