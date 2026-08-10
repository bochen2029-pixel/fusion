// tests/test_gs.cpp — M1 slice 2a acceptance: the Δ* solver is analytic-verified and
// the Picard equilibrium converges to the machine's current with a sane axis shift.
#include "core/gs.h"
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

    std::puts("GS GREEN (2a solver verified; 2b shaped equilibrium + q95 live)");
    return 0;
}
