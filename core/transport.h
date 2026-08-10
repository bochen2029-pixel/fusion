// core/transport.h — tier-1 1-D radial transport (spec §2.1: n(ρ), T(ρ), 20 nodes,
// prescribed χ CALIBRATED so the volume-integrated τ_E reproduces tier-0's IPB98 at
// the reference point) + the evolving current-profile family (PHY-14: α as the 0-D
// profile state — l_i and q₀ are REPORTED; neoclassical current-diffusion timescale;
// sawtooth crash resets the core). D-042.
//
// Closure (stated): χ(ρ,t) = χ0·(1+2ρ²)·(P_heat/P_ref)^0.69 / H98(t) — the power-
// degradation exponent is IPB98's, backed into the prescribed χ so the tier-0 macro
// behavior (W ∝ P^0.31 at fixed n) is reproduced globally, not just at the reference
// point; H98 enters as confinement luck / H-L back-transitions. χ acts on the TOTAL
// thermal energy density u = 3nT (the calibration absorbs species/convention factors
// — stated). D = 0.4·χ0·(1+2ρ²) (no power scaling; τ_p emerges). Geometry: cylinder
// of area-consistent minor radius a_eff = a·√κ_a (scaling grade, stated).
// Determinism: fixed node count, implicit Thomas steps, no data-dependent iteration.
#pragma once
#include "config.h"
#include "physics_tier0.h"
#include <cmath>
#include <cstdio>
#include <algorithm>

namespace fusion {

struct Tier1Burn {
    static constexpr int NRHO = 20;
    double T[NRHO];                 // keV, cell centers rho_j = (j+0.5)/NRHO
    double n[NRHO];                 // m^-3
    double alpha_prof = 1.0;        // the current-profile family exponent (0-D state,
                                    // PHY-14): w = (1-psibar)^alpha in the GS source
    double chi0 = 1.0;              // calibrated at init (receipted)
    double D0 = 0.2;
    double a_eff = 1.0, Vtot = 19.6, P_ref_W = 40e6;
    double li_1d = 0.85;            // REPORTED: cylindrical l_i of the 1-D j-profile
    double q0_rep = 1.1;            // REPORTED: cylindrical axis q (2B0/(mu0 R j0))
    long   sawteeth = 0;            // crash counter (events emitted by the caller)

    static double rho_c(int j) { return (j + 0.5) / double(NRHO); }

    // volume of shell j (rho^2 weighting; sums to Vtot exactly)
    double Vj(int j) const {
        const double r0 = j / double(NRHO), r1 = (j + 1) / double(NRHO);
        return Vtot * (r1 * r1 - r0 * r0);
    }

    void init(const MachineCfg& m, double Tbar_keV, double nbar) {
        a_eff = m.a * std::sqrt(m.kappa_a);
        Vtot = m.V;
        // reference profiles: mild peaking, volume averages hit the targets exactly
        double sT = 0, sn = 0;
        for (int j = 0; j < NRHO; ++j) {
            const double r = rho_c(j);
            T[j] = std::pow(1.0 - r * r * 0.96, 1.1);
            n[j] = std::sqrt(1.0 - 0.7 * r * r);
            sT += T[j] * Vj(j); sn += n[j] * Vj(j);
        }
        for (int j = 0; j < NRHO; ++j) {
            T[j] *= Tbar_keV * Vtot / sT;
            n[j] *= nbar * Vtot / sn;
        }
        // chi0 calibration (D-042, receipted): solve the STEADY-STATE of the exact
        // discrete operator at chi0=1 under the reference deposition (aux gaussian
        // 27 MW + alpha-shaped 13 MW, radiation excluded — tau_E covers CONDUCTION,
        // PHY-09), via the same solve_tridiag with dt -> infinity; the solution is
        // linear in 1/chi0 at fixed sources, so one division lands tau_E(ref) =
        // IPB98(ref) EXACTLY in the operator's own discretization. Same for D0
        // against tau_p_ref = 2*tau_E_ref (the M0 particle-time convention).
        const double n19 = (nbar / 1e19);
        const double tau_ref = ipb98_coeff(m, m.Ip_MA, n19) * std::pow(40.0, -0.69);
        P_ref_W = 40e6;
        {
            double Scal[NRHO], ax[NRHO], axn = 0, aln[NRHO], alnorm = 0;
            for (int j = 0; j < NRHO; ++j) {
                const double r = rho_c(j);
                ax[j] = std::exp(-0.5 * std::pow((r - 0.30) / 0.25, 2.0));
                axn += ax[j] * Vj(j);
                const double sv = bosch_hale_dt(T[j]);
                aln[j] = 0.25 * n[j] * n[j] * sv;
                alnorm += aln[j] * Vj(j);
            }
            for (int j = 0; j < NRHO; ++j)
                Scal[j] = 27e6 * ax[j] / axn + 13e6 * (alnorm > 0 ? aln[j] / alnorm : 0.0);
            double ucal[NRHO] = {0};
            solve_tridiag(ucal, 1.0, Scal, 1e6, 0.0);      // steady state at chi=1
            double W1 = 0;
            for (int j = 0; j < NRHO; ++j) W1 += ucal[j] * Vj(j);
            chi0 = W1 / std::max(tau_ref * P_ref_W, 1.0);
            // particles: steady content at D=1 under the edge gas shape vs the target
            double Sg[NRHO], gn = 0;
            for (int j = 0; j < NRHO; ++j) {
                const double r = rho_c(j);
                Sg[j] = std::exp(-0.5 * std::pow((r - 0.85) / 0.08, 2.0));
                gn += Sg[j] * Vj(j);
            }
            const double tau_p_ref = 2.0 * tau_ref;
            const double Sref = nbar * Vtot / tau_p_ref;   // source that holds nbar
            for (int j = 0; j < NRHO; ++j) Sg[j] = Sref * Sg[j] / gn;
            // steady content = (source part, ~1/D) + (BC part, D-INDEPENDENT — the
            // edge Dirichlet's harmonic fill). Calibrate on the source part only:
            //   D0 = (N1 - Nbc) / (N_target - Nbc)   [measured: conflating them left
            //   the realized nbar at ~0.44x target and the n-PID spiralled — receipted]
            double ncal[NRHO] = {0}, nbc[NRHO] = {0}, Sz[NRHO] = {0};
            solve_tridiag(ncal, 1.0, Sg, 1e6, 0.25 * nbar);
            solve_tridiag(nbc, 1.0, Sz, 1e6, 0.25 * nbar);
            double N1 = 0, Nbc = 0;
            for (int j = 0; j < NRHO; ++j) { N1 += ncal[j] * Vj(j); Nbc += nbc[j] * Vj(j); }
            const double Nt = nbar * Vtot;
            D0 = std::clamp((N1 - Nbc) / std::max(Nt - Nbc, 1.0), 0.005, 5.0);
        }
        alpha_prof = 1.0;
    }

    // the ONE tridiagonal solver (implicit step; dt=1e6 = steady-state) — runtime and
    // calibration share it so the calibration is exact in the operator's discretization
    void solve_tridiag(double* x, double coef_eff, const double* Ssrc, double dt,
                       double edge_ghost_val) {
        const double dr = a_eff / NRHO;
        double aL[NRHO], aD[NRHO], aU[NRHO], rhs[NRHO];
        for (int j = 0; j < NRHO; ++j) {
            const double rm = j / double(NRHO), rp = (j + 1) / double(NRHO);
            const double rc = rho_c(j);
            auto face = [&](double rf) { return coef_eff * (1.0 + 2.0 * rf * rf); };
            const double Fm = (j > 0) ? rm * face(rm) / (dr * dr) : 0.0;
            const double Fp = rp * face(rp) / (dr * dr);
            aL[j] = -dt * Fm / rc;
            aU[j] = -dt * Fp / rc;
            aD[j] = 1.0 - aL[j] - aU[j];
            rhs[j] = x[j] + dt * Ssrc[j];
            if (j == NRHO - 1) {                   // edge Dirichlet via ghost
                rhs[j] += -2.0 * aU[j] * edge_ghost_val;
                aD[j] += -aU[j];                   // (first-order ghost closure)
                aU[j] = 0.0;
            }
        }
        for (int j = 1; j < NRHO; ++j) {
            const double w = aL[j] / aD[j - 1];
            aD[j] -= w * aU[j - 1];
            rhs[j] -= w * rhs[j - 1];
        }
        x[NRHO - 1] = rhs[NRHO - 1] / aD[NRHO - 1];
        for (int j = NRHO - 2; j >= 0; --j)
            x[j] = (rhs[j] - aU[j] * x[j + 1]) / aD[j];
    }

    // one implicit sub-cycle step (dt ~ 1 ms). Inputs: volume-level 0-D quantities the
    // sources need; outputs applied in place. Returns total radiated power [W].
    struct StepIO {
        double Pal_W;               // lagged alpha heating (volume total, 0-D state)
        double Paux_W;              // aux actuator state
        double Pohm_W;
        double Sgas;                // particle source actuator [m^-3 s^-1 volume-avg]
        double fDT;                 // n_DT / n_e (from 0-D ash/impurity bookkeeping)
        double nimp_frac;           // n_imp / n_e
        double Zeff;
        double H98;                 // includes hl_backtransition timeline
        double B0, wall_refl, Vmach;
        // outputs
        double Prad_W = 0, Pheat_W = 0, W_J = 0, nbar = 0, Tvol = 0, T_core = 0;
        double Pfus_W = 0, Palpha_inst_W = 0;
    };

    void step(const MachineCfg& m, double dt, StepIO& io) {
        const double dr = a_eff / NRHO;
        // ---- sources per node (explicit; scalar kernels from physics_tier0) --------
        double S[NRHO], Sn[NRHO], prad_sum = 0, pfus = 0, palpha = 0;
        double alpha_shape[NRHO], alpha_norm = 0, aux_shape[NRHO], aux_norm = 0;
        double ohm_shape[NRHO], ohm_norm = 0, gas_shape[NRHO], gas_norm = 0;
        for (int j = 0; j < NRHO; ++j) {
            const double r = rho_c(j);
            const double sv = bosch_hale_dt(T[j]);
            const double nDT = io.fDT * n[j];
            const double R_dt = 0.25 * nDT * nDT * sv;
            alpha_shape[j] = R_dt; alpha_norm += R_dt * Vj(j);
            pfus += R_dt * (m.E_alpha_MeV * 1e3 * E_KEV_J) * m.pfus_over_pa * Vj(j);
            palpha += R_dt * (m.E_alpha_MeV * 1e3 * E_KEV_J) * m.f_conf * Vj(j);
            aux_shape[j] = std::exp(-0.5 * std::pow((r - 0.30) / 0.25, 2.0));
            aux_norm += aux_shape[j] * Vj(j);
            ohm_shape[j] = std::pow(std::max(T[j], 0.05), 1.5);   // ~ j^2/sigma ~ T^1.5
            ohm_norm += ohm_shape[j] * Vj(j);
            gas_shape[j] = std::exp(-0.5 * std::pow((r - 0.85) / 0.08, 2.0));
            gas_norm += gas_shape[j] * Vj(j);
            const double n20 = n[j] / 1e20;
            const double pbr = 5.35e3 * io.Zeff * n20 * n20 * std::sqrt(std::max(T[j], 1e-3));
            const double pline = n[j] * (io.nimp_frac * n[j]) * lz_ar(T[j]);
            const double psyn = 29.0 * std::pow(io.B0, 2.5) * std::sqrt(std::max(n20, 0.0))
                              * std::pow(std::max(T[j], 1e-3), 2.5)
                              * std::sqrt(std::max(1.0 - io.wall_refl, 0.0)) / 19.6;
            const double prad_j = pbr + pline + psyn;             // W/m^3
            prad_sum += prad_j * Vj(j);
            S[j] = -prad_j;
        }
        for (int j = 0; j < NRHO; ++j) {
            S[j] += io.Pal_W * (alpha_norm > 0 ? alpha_shape[j] / alpha_norm : 0.0)
                  + io.Paux_W * (aux_norm > 0 ? aux_shape[j] / aux_norm : 0.0)
                  + io.Pohm_W * (ohm_norm > 0 ? ohm_shape[j] / ohm_norm : 0.0);
            Sn[j] = io.Sgas * io.Vmach * (gas_norm > 0 ? gas_shape[j] / gas_norm : 0.0);
        }
        // ---- power-degraded chi (IPB98 closure, header note) -----------------------
        double pheat = io.Pal_W + io.Paux_W + io.Pohm_W;
        const double pfac = std::pow(std::clamp(pheat / P_ref_W, 0.2, 5.0), 0.69);
        const double hfac = std::max(io.H98, 0.3);
        // ---- implicit Thomas on u = 3nT (energy) and on n --------------------------
        double u[NRHO];
        for (int j = 0; j < NRHO; ++j) u[j] = 3.0 * n[j] * T[j] * E_KEV_J;
        double nbar_prev = 0;
        for (int j = 0; j < NRHO; ++j) nbar_prev += n[j] * Vj(j);
        nbar_prev /= Vtot;
        solve_tridiag(u, chi0 * pfac / hfac, S, dt, 3.0 * n[NRHO - 1] * 0.1 * E_KEV_J);
        solve_tridiag(n, D0, Sn, dt, 0.25 * nbar_prev);
        for (int j = 0; j < NRHO; ++j) {
            n[j] = std::max(n[j], 1e17);
            T[j] = std::clamp(u[j] / (3.0 * n[j] * E_KEV_J), 1e-3, 100.0);
        }
        // ---- outputs ---------------------------------------------------------------
        io.Prad_W = prad_sum; io.Pheat_W = pheat;
        io.Pfus_W = pfus; io.Palpha_inst_W = palpha;
        double W = 0, nb = 0, Tv = 0;
        for (int j = 0; j < NRHO; ++j) {
            W += 3.0 * n[j] * T[j] * E_KEV_J * Vj(j);
            nb += n[j] * Vj(j); Tv += T[j] * Vj(j);
        }
        io.W_J = W; io.nbar = nb / Vtot; io.Tvol = Tv / Vtot;
        io.T_core = 0.5 * (T[1] + T[2]);               // the ECE chord (~rho 0.1-0.2)
    }

    // current-profile family evolution (PHY-14): alpha relaxes toward the neoclassical
    // equilibrium peaking (j ~ sigma_Spitzer ~ T^1.5 at fixed loop field) on the
    // current-diffusion time; l_i and q0 REPORTED from the 1-D j-profile.
    // Sawtooth: q0_rep < 1 -> crash (energy-conserving core flatten + alpha drop);
    // timing EMERGES from state (dispersions law). Returns true when a crash fired.
    bool step_profile(const MachineCfg& m, double dt, double Zeff, double Ip_A) {
        double pk_num = 0, pk_den = 0;
        for (int j = 0; j < NRHO; ++j) {
            const double w15 = std::pow(std::max(T[j], 0.05), 1.5);
            pk_num += w15 * Vj(j); pk_den += Vj(j);
        }
        const double j0_over_avg = std::pow(std::max(T[0], 0.05), 1.5) / (pk_num / pk_den);
        const double alpha_eq = std::clamp(j0_over_avg - 1.0, 0.3, 2.5);
        const double Tbar = std::max(0.3, pk_num / pk_den > 0 ?
                                     std::pow(pk_num / pk_den, 2.0 / 3.0) : 1.0);
        const double eta = 2.0 * 2.8e-8 * Zeff / std::pow(Tbar, 1.5);
        const double tau_cr = std::clamp(0.14 * 1.25663706212e-6 * a_eff * a_eff / eta,
                                         0.5, 60.0);
        alpha_prof += dt * (alpha_eq - alpha_prof) / tau_cr;
        // cylindrical l_i and q0 of j ~ T^1.5 (REPORTED; the GS's own l_i is the
        // flux-geometry version — both receipted, conventions stated)
        double Icum = 0, bth2 = 0, jsum = 0;
        double jprof[NRHO];
        for (int j = 0; j < NRHO; ++j) { jprof[j] = std::pow(std::max(T[j], 0.05), 1.5); jsum += jprof[j] * Vj(j); }
        for (int j = 0; j < NRHO; ++j) jprof[j] *= Ip_A * Vtot / (jsum > 0 ? jsum : 1.0) / Vtot;
        double bth_a = 0;
        {
            double Ic = 0;
            for (int j = 0; j < NRHO; ++j) {
                const double r0 = j / double(NRHO), r1 = (j + 1) / double(NRHO);
                Ic += jprof[j] * (r1 * r1 - r0 * r0);   // ~ area fraction weighting
                const double rc = rho_c(j);
                const double bth = Ic / (rc > 0 ? rc : 1.0);
                bth2 += bth * bth * rc * (1.0 / NRHO);
                (void)r0;
            }
            bth_a = Ic;                                  // Btheta(a) ~ total
            Icum = Ic;
        }
        li_1d = (bth_a > 0) ? 2.0 * bth2 / (bth_a * bth_a) : 0.85;
        (void)Icum;
        const double j0_SI = jprof[0] * Ip_A > 0 ? jprof[0] : 0.0;
        // q0 from the cylindrical axis relation with the PHYSICAL on-axis density:
        // j0 [A/m^2] = peaking * Ip / (pi a_eff^2)
        const double jax = (1.0 + alpha_prof) * Ip_A / (3.14159265358979 * a_eff * a_eff);
        q0_rep = 2.0 * m.B0 / (1.25663706212e-6 * m.R0 * std::max(jax, 1.0));
        (void)j0_SI;
        if (q0_rep < 1.0) {                              // SAWTOOTH crash
            const int jmix = int(0.35 * NRHO);           // rho_mix 0.35 (pinned, stated)
            double Um = 0, Vm = 0, Nm = 0;
            for (int j = 0; j <= jmix; ++j) {
                Um += 3.0 * n[j] * T[j] * E_KEV_J * Vj(j);
                Nm += n[j] * Vj(j); Vm += Vj(j);
            }
            const double nf = Nm / Vm;
            const double Tf = Um / (3.0 * nf * E_KEV_J * Vm);
            for (int j = 0; j <= jmix; ++j) { n[j] = nf; T[j] = Tf; }
            alpha_prof = std::max(0.3, alpha_prof * 0.80);
            ++sawteeth;
            return true;
        }
        return false;
    }
};

} // namespace fusion
