// tests/test_budget.cpp — budget_events: the tick cadence must fit its own budget law
// (the M-B1/P-07 unit test the QC demanded). Stub-grade at M0, real check nonetheless.
#include "core/config.h"
#include <toml.hpp>
#include <cstdio>

int main(int argc, char** argv) {
    const std::string root = argc > 1 ? argv[1] : ".";
    toml::table t = toml::parse_file(root + "/contracts/events.toml");
    const double interval = t["ticks"]["nominal_interval_s"].value_or(300.0);
    const double cap = t["budget"]["max_tokens_per_hr_nominal"].value_or(150.0);
    const double tick_tokens = 8.0;                       // "[tick 300s Q=8.1 nominal]"
    const double per_hr = 3600.0 / interval * tick_tokens;
    std::printf("ticks: %.0f s interval -> %.1f tok/hr vs cap %.0f\n", interval, per_hr, cap);
    if (per_hr > cap) { std::puts("BUDGET RED: tick cadence busts its own cap"); return 1; }
    std::puts("BUDGET GREEN");
    return 0;
}
