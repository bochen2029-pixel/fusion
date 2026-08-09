// core/philox.h — Philox4x32-10 counter RNG (Salmon et al. 2011), fusion house RNG.
// Deterministic by construction: value = f(key, counter). No state beyond the counter.
// Stream-domain law: contracts/dispersions.toml [streams] — plant=0, ghost=0(shared),
// teacher=1, train_env=2. M0 uses the plant stream only (scenario-init draws).
#pragma once
#include <cstdint>

namespace fusion {

struct Philox4 { uint32_t v[4]; };

inline void philox_round(uint32_t c[4], const uint32_t k[2]) {
    constexpr uint64_t M0 = 0xD2511F53ull, M1 = 0xCD9E8D57ull;
    const uint64_t p0 = M0 * c[0], p1 = M1 * c[2];
    const uint32_t hi0 = uint32_t(p0 >> 32), lo0 = uint32_t(p0);
    const uint32_t hi1 = uint32_t(p1 >> 32), lo1 = uint32_t(p1);
    c[0] = hi1 ^ c[1] ^ k[0];
    c[1] = lo1;
    c[2] = hi0 ^ c[3] ^ k[1];
    c[3] = lo0;
}

// key = {seed_lo, seed_hi ^ stream}; counter = {block, scenario_id, 0, 0}
inline Philox4 philox10(uint64_t seed, uint32_t stream, uint32_t scenario_id, uint32_t block) {
    uint32_t c[4] = { block, scenario_id, 0u, 0u };
    uint32_t k[2] = { uint32_t(seed & 0xFFFFFFFFu), uint32_t(seed >> 32) ^ stream };
    for (int r = 0; r < 10; ++r) {
        philox_round(c, k);
        k[0] += 0x9E3779B9u; k[1] += 0xBB67AE85u;
    }
    return Philox4{ { c[0], c[1], c[2], c[3] } };
}

inline double u01(uint32_t a, uint32_t b) {            // 53-bit uniform in [0,1)
    const uint64_t x = (uint64_t(a) << 21) ^ (uint64_t(b) >> 11) ^ (uint64_t(b) << 32 >> 32);
    return double((x ^ (uint64_t(a) << 40)) & ((1ull << 53) - 1)) * (1.0 / 9007199254740992.0);
}

// One block -> two independent U(0,1); gauss() -> two N(0,1) via Box-Muller.
struct Draw2 { double u0, u1; };
inline Draw2 draw2(uint64_t seed, uint32_t stream, uint32_t scen, uint32_t block) {
    const Philox4 p = philox10(seed, stream, scen, block);
    return Draw2{ u01(p.v[0], p.v[1]), u01(p.v[2], p.v[3]) };
}
struct Gauss2 { double g0, g1; };
inline Gauss2 gauss2(uint64_t seed, uint32_t stream, uint32_t scen, uint32_t block);

} // namespace fusion

#include <cmath>
namespace fusion {
inline Gauss2 gauss2(uint64_t seed, uint32_t stream, uint32_t scen, uint32_t block) {
    Draw2 d = draw2(seed, stream, scen, block);
    const double r = std::sqrt(-2.0 * std::log(d.u0 > 1e-300 ? d.u0 : 1e-300));
    const double th = 6.283185307179586 * d.u1;
    return Gauss2{ r * std::cos(th), r * std::sin(th) };
}
} // namespace fusion
