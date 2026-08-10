// core/rings.h — coaxial-ring inductance primitives (shared by the passive-conductor
// model and the GS free-boundary/self-field machinery). Deterministic: AGM to fixed
// tolerance, no branches on data beyond the convergence count cap.
#pragma once
#include <cmath>
#include <algorithm>

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

// Maxwell: mutual inductance of coaxial rings, radii r1,r2, axial separation dz.
// M·I = poloidal flux (full Wb) through the circle (r2, z2) from current I at (r1, z1);
// the GS per-radian flux convention divides by 2π at the call site.
inline double ring_mutual(double r1, double r2, double dz) {
    const double mu0 = 1.25663706212e-6;
    const double k2 = 4.0 * r1 * r2 / ((r1 + r2) * (r1 + r2) + dz * dz);
    const double k = std::sqrt(std::clamp(k2, 1e-14, 1.0 - 1e-14));
    double K, E; elliptic_ke(k, K, E);
    return mu0 * std::sqrt(r1 * r2) * ((2.0 / k - k) * K - (2.0 / k) * E);
}

// self-inductance of a ring of major radius R, wire (minor) radius aw — the −1.75 form
// includes the internal-inductance term (uniform current distribution)
inline double ring_self(double R, double aw) {
    const double mu0 = 1.25663706212e-6;
    return mu0 * R * (std::log(8.0 * R / aw) - 1.75);
}

} // namespace fusion
