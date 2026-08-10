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
    static constexpr double F_SCREEN_HZ = 180.0, M_EFF_FLOOR = 5.0;
    double k_dest = 0.0;            // N/m — DERIVED input (gs_vertical_derive; D-038)
    double c_p[NP];                 // force per passive ampere = Ip * dM_pj/dZ [N/A]
    double c_vs = 0.0;
    double Mfull[NC][NC];           // circuit inductance matrix [filaments..., VS]
    double Minv[NC][NC];
    double Rcirc[NC];               // circuit resistances (vessel scalar calibrated)
    double Lvs = 2.0e-3, Rvs = 0.2;  // 20-turn in-vessel pair: tau 10 ms, 10 kA at 2 kV
    double vs_turns = 20.0;
    double kwall_ = 0.0;             // instantaneous screening stiffness c^T Minv c (rep.)
    double rho_shell = 0.0;          // the calibrated surface-resistivity scalar (rep.)
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

    void build(const MachineCfg& m, double k_dest_derived_Npm) {
        const double PI = 3.14159265358979;
        const double b_sh = m.a * m.b_over_a;               // machine.toml [vessel]
        const double kap_sh = m.kappa_shell;
        const double tau_w = m.tau_wall_s;
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
        // VS pair at (R0 +- a*1.1, +-a*1.2) anti-series: effective dM/dZ doubled
        const double Rvs_r = m.R0 + m.a * 1.1, Zvs = m.a * 1.2;
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
        Rcirc[NP] = Rvs;
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
        // the regularization: pin the screened-oscillation artifact near F_SCREEN_HZ
        // (documented; if the shell cannot hold the shape at all — kwall <= k_dest —
        // the mode is beyond passive stabilization and the class test must catch it)
        const double w_t = 2.0 * PI * F_SCREEN_HZ;
        m_eff = (kwall_ > k_dest) ? std::max(M_EFF_FLOOR, (kwall_ - k_dest) / (w_t * w_t))
                                  : 30.0;
        // ---- assemble A: M dI/dt = -R I - c dZ/dt + e_vs Vcmd ----------------------
        for (int i = 0; i < N; ++i) { B[i] = 0; for (int j = 0; j < N; ++j) A[i][j] = 0; }
        A[0][1] = 1.0;
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
            } else {                                  // [Z, V, I_vs] alone
                d[0] = s[1];
                d[1] = (k_dest * s[0] + c_vs * s[2]) / m_eff;
                d[2] = (-Rvs * s[2] - c_vs * s[1]) / Lvs;
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

// ---- the reduced EKF (D-023: the pre-registered epistemic handicap) ----------------
struct VerticalEKF {
    // model: Z'' = (k/m) Z + (c/m) I_vs_cmd_proxy - d V   with 10% param mismatch and
    // NO passive-current states — the reduction IS the handicap (documented).
    double xz = 0, xv = 0;
    double P[2][2] = { {1e-6, 0}, {0, 1e-4} };
    double k_over_m, c_over_m, damp;
    double sigma_probe;             // effective probe noise (averaged channels)
    double nis_sum = 0; long nis_n = 0;
    double innov_last = 0, S_last = 1;

    // Reduced model = the SCREENED oscillator (what the probes actually see at control
    // timescales), with 10% parameter mismatch — the pre-registered handicap (D-023).
    // The slow wall-mode drift is carried by process noise, not by the model: that
    // omission IS the reduction. omega^2 = (k_wall - k_dest)/m, zeta guessed low.
    void init(const VerticalModel& vm, double probe_sigma_m) {
        const double w2_true = (vm.kwall_ - vm.k_dest) / vm.m_eff;
        k_over_m = -0.90 * (w2_true > 0 ? w2_true : 1e4);   // restoring, mismatched
        c_over_m = 0.90 * vm.c_vs / vm.m_eff;
        const double w = std::sqrt(w2_true > 0 ? w2_true : 1e4);
        damp = 2.0 * 0.70 * w;      // heavily damped INTERNAL model: the filter tracks,
                                    // it does not resonate (the 10 kHz alpha correction
                                    // supplies the bandwidth; measured fix, receipted)
        sigma_probe = probe_sigma_m;
    }
    // SLICE-1 REDUCTION (documented, D-035): a FIXED-GAIN steady-state filter (alpha-
    // beta tracker = the steady-state Kalman of this measurement geometry). The full
    // adaptive EKF with covariance propagation + chi^2 NIS acceptance is the M1-full
    // deliverable, built on the GS-linearized model (slice 2). The hand-rolled 2x2
    // covariance propagation was Euler-unstable at the screened-mode frequency —
    // measured, receipted, replaced.
    // KINEMATIC predict (constant velocity — no plant model at all): the maximal
    // pre-registered handicap. The stiffness model whipped xv on the slow wall-mode
    // manifold (true quasi-static accel ~0 vs modeled -k*Z — measured, receipted);
    // slice 2's EKF gets the GS-linearized model + the D-023 information-set doctrine.
    void predict(double Ivs, double dt) {
        xz += dt * xv;
        (void)Ivs;
    }
    void update(double z_meas) {
        const double alpha = 0.45, beta = 0.08, dt = 1e-4;
        const double nu = z_meas - xz;
        const double S = sigma_probe * sigma_probe * 2.5;   // fixed consistency scale
        innov_last = nu; S_last = S;
        nis_sum += nu * nu / S; ++nis_n;
        xz += alpha * nu;
        xv += 50.0 * nu;            // velocity correction (beta_canon ~ 5e-3 at 10 kHz)
        (void)beta; (void)dt;
    }
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
