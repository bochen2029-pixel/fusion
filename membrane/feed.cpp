// membrane/feed.cpp — the event tokenizer's text side (vocabulary per contracts/events.toml).
#include "membrane/feed.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>

namespace fusion::membrane {

static std::string fmt(const char* f, double a = 0, double b = 0, double c = 0) {
    char buf[224]; std::snprintf(buf, sizeof(buf), f, a, b, c);
    return std::string(buf);
}

std::vector<FeedLine> render_feed(const RunResult& r, double scale, long long t0_ms,
                                  double tick_interval_stream_s, bool events_on) {
    std::vector<FeedLine> out;
    auto ms_of = [&](uint64_t tick) -> long long {
        return t0_ms + (long long)(double(tick) * DT_TICK * 1000.0 * scale);
    };
    if (events_on) for (const EventRec& e : r.events) {
        FeedLine L; L.ms = ms_of(e.tick);
        switch (e.kind) {
            case EvKind::PhaseStart:
                L.lane = "plant"; L.text = fmt("phase FLAT_TOP begin Ip=%.1fMA", e.v0); break;
            case EvKind::CtrlSatOn:
                L.lane = "plant"; L.text = fmt("ctrl sat P_aux at cap %.1fMW 0.5s+", e.v0); break;
            case EvKind::CtrlSatOff:
                L.lane = "plant"; L.text = fmt("ctrl sat released after %.1fs", e.v0); break;
            case EvKind::GateFire:
                L.lane = "sys";
                L.text = (e.arg == 1)
                    ? fmt("gate vde_detected fired Z=%.2fm dZdt=%.1fm/s — heating killed, VDE outrunning", e.v0, e.v1)
                    : fmt("gate quench_precursor fired f_rad=%.2f T=%.1fkeV", e.v0, e.v1);
                break;
            case EvKind::SpineShutdown:
                L.lane = "sys"; L.text = fmt("shutdown spine gate=quench_precursor — SPINE_SHUTDOWN verdict f_rad=%.2f", e.v0); break;
            case EvKind::TerminalTQ:
                L.lane = "plant";
                L.text = (e.arg == 1)
                    ? fmt("terminal thermal_quench cause=density_limit f_GW=%.2f T=%.1fkeV", e.v1, e.v0)
                    : (e.arg == 2)
                    ? fmt("terminal thermal_quench cause=vde Z=%.2fm wall contact", e.v0)
                    : fmt("terminal thermal_quench cause=radiative_collapse T=%.2fkeV f_rad=%.2f", e.v0, e.v1);
                break;
            case EvKind::Innov:
                L.lane = "plant";
                L.text = fmt("innov mag=%.1fs cluster=vertical Z=%.0fmm", e.v0, e.v1 * 1000.0);
                break;
            case EvKind::TerminalCQ:
                L.lane = "plant"; L.text = fmt("terminal current_quench tau_cq=%.1fms Ip=%.1fMA", e.v0, e.v1); break;
            case EvKind::TerminalDisrupt:
                L.lane = "plant"; L.text = fmt("terminal disrupt t=%.2fs Ip->%.1fMA — the run is over", e.v0, e.v1); break;
            // M2 S17a (D-046): the tier-1 event kinds the renderer had been silently
            // dropping (FORWARD_NOTES_M2 §5 — the tokenizer's diet is the thesis, don't
            // starve it). v0/v1 per the plant's ev() calls in core/sim.cpp.
            case EvKind::GsLate:                          // sys lane (spec §4 bus lanes)
                L.lane = "sys";
                L.text = fmt("organ gs_late: revalidation unserved |dZ|=%.1fmm, solve in flight",
                             std::fabs(e.v0 - e.v1) * 1000.0);
                break;
            case EvKind::Sawtooth:
                L.lane = "plant"; L.text = fmt("sawtooth crash q0=%.2f alpha->%.2f", e.v0, e.v1); break;
            case EvKind::HLBack:
                L.lane = "plant"; L.text = fmt("H->L back-transition H98->%.2f at t=%.1fs — confinement drop", e.v0, e.v1); break;
        }
        if (!L.text.empty()) out.push_back(std::move(L));
    }
    // silence, priced: ticks synthesized from the golden at the budget cadence
    const double sim_interval = tick_interval_stream_s / scale;         // s of sim time
    for (double t = sim_interval; ; t += sim_interval) {
        const size_t gi = size_t(t / 0.01 + 0.5);
        if (gi >= r.golden.size()) break;
        const GoldenRec& g = r.golden[gi];
        if (g.mode != 0) break;                       // no nominal ticks after terminal
        FeedLine L; L.ms = t0_ms + (long long)(t * 1000.0 * scale); L.lane = "plant";
        L.text = fmt("tick +%.0fs nominal Q=", tick_interval_stream_s) +
                 fmt("%.2f T=%.1fkeV n=%.1fe20", g.Q, g.T, g.ne / 1e20);
        out.push_back(std::move(L));
    }
    std::stable_sort(out.begin(), out.end(),
                     [](const FeedLine& a, const FeedLine& b) { return a.ms < b.ms; });
    return out;
}

void write_truth_tsv(const std::string& path, const RunResult& r, double scale,
                     long long t0_ms) {
    std::ofstream f(path);
    f << "# ms\tQ\tT_keV\tne20   (ground truth for the F-VERACITY code grader)\n";
    for (size_t gi = 0; gi < r.golden.size(); gi += 100) {              // every 1 s sim
        const GoldenRec& g = r.golden[gi];
        const long long ms = t0_ms + (long long)(double(g.tick) * DT_TICK * 1000.0 * scale);
        char b[128];
        std::snprintf(b, sizeof(b), "%lld\t%.3f\t%.2f\t%.2f\n", ms, g.Q, g.T, g.ne / 1e20);
        f << b;
    }
}

bool write_feed_tsv(const std::string& path, const std::vector<FeedLine>& lines) {
    std::ofstream f(path);
    if (!f) return false;
    f << "# ms\tlane\ttext   (fusion shadow feed; replayed by fusor_shadow)\n";
    for (const auto& L : lines) f << L.ms << "\t" << L.lane << "\t" << L.text << "\n";
    return true;
}

bool read_feed_tsv(const std::string& path, std::vector<FeedLine>& out) {
    std::ifstream f(path);
    if (!f) return false;
    std::string ln;
    while (std::getline(f, ln)) {
        if (ln.empty() || ln[0] == '#') continue;
        const size_t a = ln.find('\t'); if (a == std::string::npos) continue;
        const size_t b = ln.find('\t', a + 1); if (b == std::string::npos) continue;
        FeedLine L;
        L.ms = atoll(ln.substr(0, a).c_str());
        L.lane = ln.substr(a + 1, b - a - 1);
        L.text = ln.substr(b + 1);
        if (!L.text.empty()) out.push_back(std::move(L));
    }
    return !out.empty();
}

std::vector<FeedLine> merge_feeds(const std::vector<std::vector<FeedLine>>& feeds) {
    std::vector<FeedLine> all;
    for (const auto& f : feeds) all.insert(all.end(), f.begin(), f.end());
    std::stable_sort(all.begin(), all.end(),
                     [](const FeedLine& a, const FeedLine& b) { return a.ms < b.ms; });
    return all;
}

} // namespace fusion::membrane
