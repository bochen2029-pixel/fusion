// control/net.h — THE POLICY NET RUNTIME (M2 S19, D-049). A flat-weight MLP, loaded
// mmap-style from a byte file the trainer exports (trainer/export_net.py), run on CPU at
// fixed precision with FIXED-ORDER dot products (/fp:strict) so the inference path is
// bit-reproducible and KAT-checkable (spec §3, CTL-32; relay Q18). No framework runtime.
//
// FENCE (CTL-14b): this header is FENCE-LEGAL — it includes ONLY the standard library and
// reads ONLY the float observation vector the caller hands it. It never sees true state;
// statecheck walks its closure (added to FENCE_TUS). The net is the residual on the LQG
// null: policy_vs computes u = LQG(x_hat) + clamp(net_residual), so zero weights => zero
// residual => bit-identical to the null (the ship-the-runtime-before-you-train pattern).
//
// Architecture is width-agnostic at load (the width lives in the file header — a trainer
// sweep, not a contract, CTL-36); the DEPTH is pinned at 2 hidden layers (spec §3). tanh
// hidden activations (bounded — a residual controller should not explode off-distribution),
// linear output. The output is scaled to the residual bound by the caller.
#pragma once
#include <cstdint>
#include <cstddef>
#include <cmath>
#include <vector>
#include <string>
#include <cstdio>

namespace fusion {

struct NetMLP {
    // flat file: [magic 'FNET'][i32 n_in][i32 n_hidden][i32 n_out] then float32:
    //   W0[n_hidden*n_in], b0[n_hidden], W1[n_hidden*n_hidden], b1[n_hidden],
    //   W2[n_out*n_hidden], b2[n_out]   (row-major; 2 hidden layers pinned)
    static constexpr uint32_t MAGIC = 0x54454E46u;   // 'FNET' little-endian
    int32_t n_in = 0, n_hidden = 0, n_out = 0;
    std::vector<float> W0, b0, W1, b1, W2, b2;
    bool loaded = false;

    bool load(const std::string& path) {
        std::FILE* f = std::fopen(path.c_str(), "rb");
        if (!f) return false;
        uint32_t magic = 0;
        bool ok = std::fread(&magic, sizeof(uint32_t), 1, f) == 1 && magic == MAGIC;
        ok = ok && std::fread(&n_in, sizeof(int32_t), 1, f) == 1;
        ok = ok && std::fread(&n_hidden, sizeof(int32_t), 1, f) == 1;
        ok = ok && std::fread(&n_out, sizeof(int32_t), 1, f) == 1;
        if (!ok || n_in <= 0 || n_hidden <= 0 || n_out <= 0) { std::fclose(f); return false; }
        auto rd = [&](std::vector<float>& v, size_t n) {
            v.resize(n);
            return n == 0 || std::fread(v.data(), sizeof(float), n, f) == n;
        };
        ok = rd(W0, size_t(n_hidden) * n_in) && rd(b0, n_hidden)
          && rd(W1, size_t(n_hidden) * n_hidden) && rd(b1, n_hidden)
          && rd(W2, size_t(n_out) * n_hidden) && rd(b2, n_out);
        std::fclose(f);
        loaded = ok;
        return ok;
    }

    // forward pass. `in` has n_in floats; writes n_out floats to `out`. FIXED loop order —
    // deterministic under /fp:strict (no unordered reductions; CTL-32). Single thread.
    void forward(const float* in, float* out) const {
        // hidden layer 0: h0 = tanh(W0 x + b0)
        std::vector<float> h0(n_hidden), h1(n_hidden);
        for (int i = 0; i < n_hidden; ++i) {
            float acc = b0[i];
            const float* w = &W0[size_t(i) * n_in];
            for (int j = 0; j < n_in; ++j) acc += w[j] * in[j];
            h0[i] = std::tanh(acc);
        }
        // hidden layer 1: h1 = tanh(W1 h0 + b1)
        for (int i = 0; i < n_hidden; ++i) {
            float acc = b1[i];
            const float* w = &W1[size_t(i) * n_hidden];
            for (int j = 0; j < n_hidden; ++j) acc += w[j] * h0[j];
            h1[i] = std::tanh(acc);
        }
        // output: linear
        for (int i = 0; i < n_out; ++i) {
            float acc = b2[i];
            const float* w = &W2[size_t(i) * n_hidden];
            for (int j = 0; j < n_hidden; ++j) acc += w[j] * h1[j];
            out[i] = acc;
        }
    }
};

} // namespace fusion
