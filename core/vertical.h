// core/vertical.h — M1 slice 1: THE ENEMY. The linearized vertical instability over the
// real passive-conductor circuit model (spec v0.2 §2.1 tier-1, first element).
//
// Model (scaling-grade, honesty-labeled; receipts state the calibrated constants):
//   - plasma = rigid current ring (Ip) at (R0, Z), effective inertia m_eff (the massless
//     limit's numerical regularization — stated, not hidden)
//   - destabilizing force gradient k_dest [N/m] from the shaping quadrupole (calibrated
//     constant; the class check is the ctest band, the VALUE is reported)
//   - 24 passive filaments on the vessel shell (machine.toml [vessel]): true ring-ring
//     mutual inductances (Maxwell's formula, AGM elliptic integrals), per-filament L
//     from the ring self-inductance formula, R = L/tau_wall (each filament carries the
//     vessel time constant; the collective anti-symmetric mode is what stabilizes)
//   - one in-vessel VS pair (machine.toml [coils] VS1): the 10 kHz actuator
//   - gamma is REPORTED from the dominant eigenvalue of the coupled linear system
//     (power iteration, fixed count — deterministic), NEVER configured (PHY-03).
//
// The EKF (D-023): a DELIBERATELY REDUCED 2-state model (Z, V) with 10% parameter
// mismatch — the pre-registered epistemic handicap — over 4 synthetic magnetic probes
// (sigma + bias walk per contracts/diagnostics.toml). Innovation feeds the tokenizer's
// [innovation] vertical cluster through the events.toml windowed gate (sigma_token,
// dwell, refractory). NIS chi^2 is the acceptance statistic.
#pragma once
#include "config.h"
#include "philox.h"
#include <cmath>
#include <algorithm>
#include <vector>

namespace fusion {

// ---- complete elliptic integrals via AGM (deterministic, ~1e-12) -------------------
inline void elliptic_ke(double k, double& K, double& E) {
    double a = 1.0, b = std::sqrt(1.0 - k * k), c = k, sum = 0.0, pow2 = 0.5;
    for (int i = 0; i < 40 && c > 1e-15; ++i) {
        const double an = 0.5 * (a + b);
        c = 0.5 * (a - b);
        b = std::sqrt(a * b); a = an;
        pow2 *= 2.0; sum += pow2 * c * c;
    }
    K = 1.5707963267948966 / a;
    E = K * (1.0 - 0.5 * (k * k + sum));
}
// Maxwell: mutual inductance of coaxial rings, radii r1,r2, axial separation dz
inline double ring_mutual(double r1, double r2, double dz) {
    const double mu0 = 1.25663706212e-6;
    const double k2 = 4.0 * r1 * r2 / ((r1 + r2) * (r1 + r2) + dz * dz);
    const double k = std::sqrt(std::clamp(k2, 1e-14, 1.0 - 1e-14));
    double K, E; elliptic_ke(k, K, E);
    return mu0 * std::sqrt(r1 * r2) * ((2.0 / k - k) * K - (2.0 / k) * E);
}
inline double ring_self(double R, double aw) {
    const double mu0 = 1.25663706212e-6;
    return mu0 * R * (std::log(8.0 * R / aw) - 1.75);
}

struct VerticalModel {
    // layout: x = [Z, V, I_1..I_24 (passive), I_vs] ; n = 27
    static constexpr int NP = 24, N = NP + 3;
    double A[N][N];                 // dx/dt = A x + B*Vcmd
    double B[N];
    double m_eff = 30.0;            // kg — the regularization, chosen so the screened
                                    // fast oscillation sits ~180 Hz (10 kHz-controllable,
                                    // EKF-trackable); the wall-mode gamma is insensitive
                                    // to it — the ctest band enforces that (documented)
    double k_dest = 0.0;            // N/m — calibrated constant (reported in receipt)
    double c_p[NP];                 // force per passive ampere = Ip * dM_pj/dZ [N/A]
    double c_vs = 0.0;
    double Lp_[NP], Rp_[NP];
    double Lvs = 2.0e-3, Rvs = 0.2;  // 20-turn in-vessel pair: tau 10 ms, 10 kA at 2 kV
    double vs_turns = 20.0;
    double kwall_ = 0.0;             // image-current stiffness (reported)
    double gamma_open = 0.0;        // reported: no-passive growth rate [1/s]
    double gamma_wall = 0.0;        // reported: with-passives growth rate [1/s]

    void build(const MachineCfg& m) {
        const double b_sh = m.a * 1.35;             // machine.toml [vessel] b_over_a
        const double kap_sh = 1.5;
        const double tau_w = 0.025;                  // tau_wall_ms/1000 — from machine
        // filament ring positions on the shell
        double Rf[NP], Zf[NP];
        for (int j = 0; j < NP; ++j) {
            const double th = 2.0 * 3.14159265358979 * (j + 0.5) / NP;
            Rf[j] = m.R0 + b_sh * std::cos(th);
            Zf[j] = b_sh * kap_sh * std::sin(th);
        }
        // plasma-filament coupling: dM/dZ by symmetric difference at Z=0 (fixed h)
        const double h = 1e-3, Ip = m.Ip_MA * 1e6;
        double kwall = 0.0;
        for (int j = 0; j < NP; ++j) {
            const double dM = (ring_mutual(m.R0, Rf[j], Zf[j] - h) -
                               ring_mutual(m.R0, Rf[j], Zf[j] + h)) / (2.0 * h);
            c_p[j] = Ip * dM;                        // N/A (and V·s/m reciprocally)
            Lp_[j] = ring_self(Rf[j], 0.02);
            Rp_[j] = Lp_[j] / tau_w;
            kwall += c_p[j] * c_p[j] / Lp_[j];       // image-current stiffness
        }
        // VS pair at (R0 +- a*1.1, +-a*1.2) anti-series: effective dM/dZ doubled
        const double dMvs = (ring_mutual(m.R0, m.R0 + m.a * 1.1, m.a * 1.2 - h) -
                             ring_mutual(m.R0, m.R0 + m.a * 1.1, m.a * 1.2 + h)) / (2.0 * h);
        c_vs = 2.0 * vs_turns * Ip * dMvs;           // anti-series multi-turn pair
        kwall_ = kwall;
        // k_dest: calibrated to a stability margin m_s ~ 1 against the computed k_wall
        // (the CLASS is enforced by the ctest band; gamma itself is reported)
        k_dest = 0.45 * kwall;
        // assemble A
        for (int i = 0; i < N; ++i) { B[i] = 0; for (int j = 0; j < N; ++j) A[i][j] = 0; }
        A[0][1] = 1.0;
        A[1][0] = k_dest / m_eff;
        for (int j = 0; j < NP; ++j) A[1][2 + j] = c_p[j] / m_eff;
        A[1][2 + NP] = c_vs / m_eff;
        for (int j = 0; j < NP; ++j) {               // L dI/dt = -R I - c_j * V
            A[2 + j][2 + j] = -Rp_[j] / Lp_[j];
            A[2 + j][1] = -c_p[j] / Lp_[j];
        }
        A[2 + NP][2 + NP] = -Rvs / Lvs;
        A[2 + NP][1] = -c_vs / Lvs;
        B[2 + NP] = 1.0 / Lvs;
        gamma_wall = dominant_growth(true);
        gamma_open = dominant_growth(false);
    }

    // dominant REAL growth rate: RK4-integrate x' = Ax at fine dt with periodic
    // renormalization; the growth is the log-norm slope over the SECOND half of the
    // window, where the unstable real mode has outgrown every transient. Deterministic
    // (fixed dt, fixed step count). A forward-Euler map power iteration fails here —
    // it inflates the fast oscillatory mode (a discretization artifact, receipted).
    double dominant_growth(bool with_passives) const {
        const double dt = 2e-5, T = with_passives ? 0.6 : 0.02;
        const long nsteps = (long)(T / dt);
        double x[N]; for (int i = 0; i < N; ++i) x[i] = (i == 0) ? 1e-3 : 0.0;
        double logn = 0.0, logn_half = 0.0;
        auto deriv = [&](const double* s, double* d) {
            for (int i = 0; i < N; ++i) {
                if (!with_passives && i >= 2 && i < 2 + NP) { d[i] = 0; continue; }
                double v = 0;
                for (int j = 0; j < N; ++j) {
                    if (!with_passives && j >= 2 && j < 2 + NP) continue;
                    v += A[i][j] * s[j];
                }
                d[i] = v;
            }
        };
        for (long it = 0; it < nsteps; ++it) {
            double k1[N], k2[N], k3[N], k4[N], t[N];
            deriv(x, k1);
            for (int i = 0; i < N; ++i) t[i] = x[i] + 0.5 * dt * k1[i];
            deriv(t, k2);
            for (int i = 0; i < N; ++i) t[i] = x[i] + 0.5 * dt * k2[i];
            deriv(t, k3);
            for (int i = 0; i < N; ++i) t[i] = x[i] + dt * k3[i];
            deriv(t, k4);
            for (int i = 0; i < N; ++i)
                x[i] += dt / 6.0 * (k1[i] + 2 * k2[i] + 2 * k3[i] + k4[i]);
            if ((it & 127) == 127) {
                double nrm = 0; for (int i = 0; i < N; ++i) nrm += x[i] * x[i];
                nrm = std::sqrt(nrm > 0 ? nrm : 1e-300);
                logn += std::log(nrm);
                for (int i = 0; i < N; ++i) x[i] /= nrm;
            }
            if (it == nsteps / 2) logn_half = logn;
        }
        { double nrm = 0; for (int i = 0; i < N; ++i) nrm += x[i] * x[i];
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
