// tests/test_feed.cpp — feed_render: the renderer must not SILENTLY DROP event kinds
// (M2 S17a/D-046; FORWARD_NOTES_M2 §5 — "the tokenizer's diet is the thesis, don't
// starve it"). Constructs a RunResult with every vocabulary event kind and asserts
// each renders a non-empty line on the right lane with the expected substring. The
// permanent guard against a new EvKind being added to the plant but not the renderer.
#include "membrane/feed.h"
#include <cstdio>
#include <cstring>
#include <string>

using namespace fusion;
using namespace fusion::membrane;

static int fail = 0;
static const std::vector<FeedLine>* g_lines = nullptr;

// assert some rendered line contains `needle` on lane `lane`
static void want(const char* label, const char* lane, const char* needle) {
    for (const auto& L : *g_lines)
        if (L.lane == lane && L.text.find(needle) != std::string::npos) {
            std::printf("  ok  %-10s [%s] \"%s\"\n", label, lane, needle);
            return;
        }
    std::fprintf(stderr, "  FEED RED: %s — no [%s] line containing \"%s\"\n", label, lane, needle);
    fail = 1;
}

int main() {
    RunResult r{};
    // a minimal golden series so the tick synthesizer has nominal ground (mode 0)
    for (uint64_t i = 0; i < 200; ++i)
        r.golden.push_back(GoldenRec{ i * 100, 5e7, 3e20, 6e18, 3e17, 8.0, 2.0, 4e7, 8.5e6, 0u, 0u });
    // one of every renderable event kind (args/values per core/sim.cpp ev() calls)
    r.events = {
        { 0,     EvKind::PhaseStart,     0, 8.5,   0.0    },
        { 1000,  EvKind::Innov,          0, 5.6,   0.058  },
        { 2000,  EvKind::GsLate,         0, 0.0582, -0.0014 },   // D-046
        { 3000,  EvKind::Sawtooth,       0, 0.98,  1.20   },     // D-046
        { 4000,  EvKind::HLBack,         0, 0.75,  4.5    },     // D-046
        { 5000,  EvKind::CtrlSatOn,      0, 60.0,  0.0    },
        { 6000,  EvKind::CtrlSatOff,     0, 1.3,   0.0    },
        { 7000,  EvKind::GateFire,       1, 0.19,  9.0    },
        { 8000,  EvKind::TerminalTQ,     2, 0.20,  8.0    },
        { 8100,  EvKind::TerminalCQ,     0, 10.0,  8.5    },
        { 8200,  EvKind::TerminalDisrupt,0, 0.82,  0.5    },
    };
    std::vector<FeedLine> lines = render_feed(r, 1.0, 0, 1.0, true);
    g_lines = &lines;
    std::puts("feed render: every event kind must produce a line —");
    want("phase",    "plant", "phase FLAT_TOP");
    want("innov",    "plant", "innov mag=");
    want("gs_late",  "sys",   "organ gs_late");        // the S17a additions
    want("sawtooth", "plant", "sawtooth crash");
    want("hlback",   "plant", "back-transition");
    want("saton",    "plant", "ctrl sat");
    want("gate",     "sys",   "gate vde_detected");
    want("tq",       "plant", "thermal_quench cause=vde");
    want("cq",       "plant", "current_quench");
    want("disrupt",  "plant", "terminal disrupt");
    if (fail) { std::fputs("FEED RED\n", stderr); return 1; }
    std::puts("FEED GREEN (no event kind is silently dropped)");
    return 0;
}
