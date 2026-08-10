// tests/test_gs.cpp — M1 slice 2a acceptance: the Δ* solver is analytic-verified and
// the Picard equilibrium converges to the machine's current with a sane axis shift.
#include "core/gs.h"
#include "core/gs_free.h"
#include <cstdio>
#include <cmath>

using namespace fusion;

int main(int argc, char** argv) {
    const std::string root = argc > 1 ? argv[1] : ".";
    const MachineCfg m = load_machine(root + "/contracts/machine.toml");

    // (1) Solov'ev analytic verification — the classic GS-code acceptance test
    const double err = gs_solovev_check(m);
    std::printf("solovev: rel max error %.2e on 65x65 (SOR fixed sweeps)\n", err);
    if (!(err < 2e-3)) { std::puts("GS RED: analytic verification"); return 1; }

    // (2) Picard equilibrium: converges to Ip, axis shifts OUTWARD (Shafranov),
    //     l_i in a credible band for peaked-linear profiles
    GsGrid g;
    g.init(m);
    GsEquilibrium eq = gs_picard(m, g);
    std::printf("picard %d iters: Ip %.2f MA (target %.1f)  axis R %.3f m (R0 %.2f, "
                "shift %+.0f mm)  Z %.3f  li %.2f  beta_p(shape) %.2f\n",
                eq.picard_iters, eq.Ip_A / 1e6, m.Ip_MA, eq.R_axis, m.R0,
                (eq.R_axis - m.R0) * 1e3, eq.Z_axis, eq.li, eq.beta_p);
    if (std::fabs(eq.Ip_A / 1e6 - m.Ip_MA) > 0.05 * m.Ip_MA) {
        std::puts("GS RED: Ip normalization did not converge"); return 1; }
    // Slice-2a claims (D-036): interior axis on the midplane, converged normalization,
    // credible li band. The SHAFRANOV-SHIFT DIRECTION and q95 are slice-2b claims —
    // they need the shaped plasma boundary (the rectangle-Solov'ev-shell BC biases the
    // axis inboard; measured here, receipted, owned by 2b's free-boundary work).
    if (!(eq.R_axis > g.Rmin + 0.1 && eq.R_axis < g.Rmax - 0.1)) {
        std::puts("GS RED: axis not interior"); return 1; }
    if (!(std::fabs(eq.Z_axis) < 0.10)) { std::puts("GS RED: axis off midplane"); return 1; }
    if (!(eq.li > 0.1 && eq.li < 2.5)) { std::puts("GS RED: li band"); return 1; }

    // (3) slice 2b first piece: the SHAPED equilibrium — Shafranov shift outboard,
    //     q95 on the real FUSOR-1 D-shape vs the machine.toml design estimate (~3.0)
    GsGrid gs2;
    ShapedEq sh = gs_shaped(m, gs2);
    std::printf("shaped: Ip %.2f MA  axis shift %+.0f mm  q0 %.2f  q95 %.2f "
                "(design est 3.0; floor 2.2)\n",
                sh.eq.Ip_A / 1e6, sh.shift_mm, sh.q0, sh.q95);
    if (std::fabs(sh.eq.Ip_A / 1e6 - m.Ip_MA) > 0.05 * m.Ip_MA) {
        std::puts("GS RED: shaped Ip normalization"); return 1; }
    if (!(sh.shift_mm > 0 && sh.shift_mm < 200)) {
        std::puts("GS RED: Shafranov shift direction/magnitude"); return 1; }
    if (!(sh.q95 > 2.2 && sh.q95 < 4.5)) { std::puts("GS RED: q95 band"); return 1; }
    if (!(sh.q0 < sh.q95)) { std::puts("GS RED: q monotonicity"); return 1; }

    // (4) the MERGE (D-038): the external field's structure at the axis, derived.
    //     Physics-signed checks: the maintaining field is NEGATIVE for Ip>0 (inward
    //     force against the hoop), the elongating quadrupole makes n NEGATIVE, and the
    //     rigid-displacement gradient is therefore DESTABILIZING (k_dest > 0). The
    //     magnitude class check lives with gamma in test_vertical; here order-of-
    //     magnitude sanity only. psi_ext must be discrete-harmonic at the axis.
    std::printf("merge:  Bz_ext(axis) %.3f T  n_decay %.3f  k_dest %.3e N/m  "
                "ext_residual %.2e\n",
                sh.Bz_ext_axis_T, sh.n_decay, sh.k_dest_Npm, sh.ext_residual);
    if (!(sh.Bz_ext_axis_T < -0.2 && sh.Bz_ext_axis_T > -3.0)) {
        std::puts("GS RED: maintaining-field sign/magnitude"); return 1; }
    if (!(sh.n_decay < 0.0)) { std::puts("GS RED: decay index not destabilizing"); return 1; }
    if (!(sh.k_dest_Npm > 1e6 && sh.k_dest_Npm < 1e9)) {
        std::puts("GS RED: k_dest order-of-magnitude"); return 1; }
    if (!(sh.ext_residual < 0.02)) { std::puts("GS RED: psi_ext not harmonic at axis"); return 1; }

    // (5) FREE BOUNDARY (D-039): the 12-circuit coil set holds the shape by itself.
    //     The inverse isoflux fit finds the currents (feasibility = within i_max);
    //     the Picard finds the DN separatrix; the coil field IS the external field,
    //     so k_dest derives directly — cross-checked against the D-038 decomposition.
    FreeEq fe = gs_free_solve(m, true);
    std::printf("free:   Ip %.2f MA  axis R %.3f (shift %+.0f mm) Z %+.3f  %s\n",
                fe.eq.Ip_A / 1e6, fe.eq.R_axis, fe.shift_mm, fe.eq.Z_axis,
                fe.x_limited ? "X-LIMITED (diverted)" : "wall-limited");
    std::printf("        X (%.3f, +-%.3f)  kappa_ach %.2f  delta_ach %.2f  a_ach %.3f\n",
                fe.Rx, fe.Zx, fe.kappa_ach, fe.delta_ach, fe.a_ach);
    std::printf("        q0 %.2f  q95 %.2f  fit_rms %.2e  drift %.2e\n",
                fe.q0, fe.q95, fe.fit_rms, fe.drift_axis);
    std::printf("        I_At [CSe %.1f  CSm %.1f  PF1 %.1f  PF2 %.1f  PF3 %.1f  "
                "PF4 %.1f] kAt  imax_frac %.2f\n",
                fe.I_At[0] / 1e3, fe.I_At[1] / 1e3, fe.I_At[2] / 1e3, fe.I_At[3] / 1e3,
                fe.I_At[4] / 1e3, fe.I_At[5] / 1e3, fe.imax_frac);
    std::printf("        Bz_ext %.3f T  n_decay %.3f  k_dest %.3e  (fixed-bnd %.3e, "
                "ratio %.2f)\n", fe.Bz_ext_axis_T, fe.n_decay, fe.k_dest_Npm,
                sh.k_dest_Npm, fe.k_dest_Npm / sh.k_dest_Npm);
    if (std::fabs(fe.eq.Ip_A / 1e6 - m.Ip_MA) > 0.05 * m.Ip_MA) {
        std::puts("GS RED: free-boundary Ip"); return 1; }
    if (!fe.x_limited) { std::puts("GS RED: no diverted (X-limited) solution"); return 1; }
    if (!(fe.Rx > 1.35 && fe.Rx < 1.85 && fe.Zx > 0.85 && fe.Zx < 1.30)) {
        std::puts("GS RED: X-point position"); return 1; }
    if (!(std::fabs(fe.eq.Z_axis) < 0.06)) { std::puts("GS RED: axis off midplane (DN)"); return 1; }
    if (!(fe.shift_mm > 0 && fe.shift_mm < 200)) { std::puts("GS RED: free shift"); return 1; }
    if (!(fe.kappa_ach > 1.60 && fe.kappa_ach < 2.10)) { std::puts("GS RED: achieved kappa"); return 1; }
    if (!(fe.delta_ach > 0.20 && fe.delta_ach < 0.70)) { std::puts("GS RED: achieved delta"); return 1; }
    if (!(fe.q95 > 2.2 && fe.q95 < 4.5)) { std::puts("GS RED: free q95 band"); return 1; }
    if (!(fe.imax_frac <= 1.20)) { std::puts("GS RED: coil currents beyond i_max margin"); return 1; }
    if (!(fe.n_decay < 0.0 && fe.k_dest_Npm > 0.0)) { std::puts("GS RED: free k_dest sign"); return 1; }
    if (!(fe.k_dest_Npm / sh.k_dest_Npm > 0.55 && fe.k_dest_Npm / sh.k_dest_Npm < 1.80)) {
        std::puts("GS RED: k_dest cross-check (coil field vs D-038 decomposition)"); return 1; }

    std::puts("GS GREEN (2a verified; 2b shaped + q95; D-038 k_dest; D-039 free boundary + X-point)");
    return 0;
}
