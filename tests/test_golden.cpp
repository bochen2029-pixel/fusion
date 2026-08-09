// tests/test_golden.cpp — golden_check: re-run the three gate seeds and memcmp against
// the COMMITTED goldens (DoD clause 2). Cross-build determinism on this box/arch.
#include "core/sim.h"
#include <cstdio>
#include <cstring>
#include <vector>

using namespace fusion;

static std::vector<unsigned char> slurp(const std::string& p) {
    std::vector<unsigned char> v;
    FILE* f = std::fopen(p.c_str(), "rb");
    if (!f) return v;
    std::fseek(f, 0, SEEK_END); long n = std::ftell(f); std::fseek(f, 0, SEEK_SET);
    v.resize(size_t(n));
    if (n > 0 && std::fread(v.data(), 1, size_t(n), f) != size_t(n)) v.clear();
    std::fclose(f);
    return v;
}

int main(int argc, char** argv) {
    const std::string root = argc > 1 ? argv[1] : ".";
    SimInputs in;
    in.m = load_machine(root + "/contracts/machine.toml");
    in.f = load_floors(root + "/contracts/floors.toml");
    in.g = load_gates(root + "/contracts/spine_gates.toml");
    in.d = load_dispersions(root + "/contracts/dispersions.toml");
    in.s = load_scenario(root + "/contracts/scenarios/easy.toml");
    in.k = load_gains(root + "/control/gains_m0.toml");
    for (uint64_t seed : { 900042ull, 900107ull, 900999ull }) {
        const std::string path = root + "/goldens/m0-easy-" + std::to_string(seed) + ".golden";
        auto want = slurp(path);
        if (want.empty()) { std::fprintf(stderr, "GOLDEN RED: missing %s\n", path.c_str()); return 1; }
        RunResult r = run_sim(in, seed, true);
        const size_t got = r.golden.size() * sizeof(GoldenRec);
        if (got != want.size() || std::memcmp(r.golden.data(), want.data(), got) != 0) {
            std::fprintf(stderr, "GOLDEN RED: seed %llu differs (%zu vs %zu bytes)\n",
                         (unsigned long long)seed, got, want.size());
            return 1;
        }
        std::printf("golden seed %llu: %zu bytes bit-identical\n",
                    (unsigned long long)seed, got);
    }
    std::puts("GOLDEN GREEN");
    return 0;
}
