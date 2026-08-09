# third_party — vendored + pinned (no package managers in the product build)

| lib | version | files | source | sha256 |
|---|---|---|---|---|
| tomlplusplus | v3.4.0 | `tomlplusplus/toml.hpp` (single header, 485,931 B) | https://raw.githubusercontent.com/marzer/tomlplusplus/v3.4.0/toml.hpp (MIT) | `6b5172ad4dd6519aec67b919181fa7a38a2234131e5b2afa232dfe444819783e` |
| llama.cpp import set | auricle's pin (matches the C:/llama.cpp runtime DLLs) | `llama.cpp/include/*.h` + `llama.cpp/lib/{llama,ggml,ggml-base}.{lib,def,exp}` | copied verbatim from `C:/auricle/third_party/llama.cpp` (MIT); DLLs delay-loaded from `C:/llama.cpp` at runtime — the import libs and headers MUST stay in lockstep with that directory | llama.lib `9a3a9d3a3b9213f341df559e51e2018aaa5bc6a0df2ea636f7ccd687dcb01ecb` |

Rules: downloads staged via `third_party/dl/` (gitignored); every vendored file is pinned
here with version + source + sha256; upgrades are a D-entry. Later milestones add
sherpa-onnx, piper, llama.cpp per their kickoffs.
