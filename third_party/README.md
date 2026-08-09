# third_party — vendored + pinned (no package managers in the product build)

| lib | version | files | source | sha256 |
|---|---|---|---|---|
| tomlplusplus | v3.4.0 | `tomlplusplus/toml.hpp` (single header, 485,931 B) | https://raw.githubusercontent.com/marzer/tomlplusplus/v3.4.0/toml.hpp (MIT) | `6b5172ad4dd6519aec67b919181fa7a38a2234131e5b2afa232dfe444819783e` |

Rules: downloads staged via `third_party/dl/` (gitignored); every vendored file is pinned
here with version + source + sha256; upgrades are a D-entry. Later milestones add
sherpa-onnx, piper, llama.cpp per their kickoffs.
