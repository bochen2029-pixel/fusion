// membrane/shadow.cpp — M0.5 · THE THESIS SLICE (fusor_shadow).
//
// Fusion's shadow soak (D-019): the tier-0 plant's tokenized event feed + a typed
// operator script, replayed into a resident LLM triple at dial ZERO, log-only (A0).
// The auricle M4.5 soak machinery (C:/auricle/src/app/soak.cpp), re-grown for fusion:
//   - fusion seats (SPEAKER / SKEPTIC / SENTINEL, spec v0.2 §4 mandates)
//   - STRUCTURAL boundaries on machine lanes: one event line = one judgment (M-B6);
//     the softmax segmenter governs the operator lane only
//   - the Sentinel's emit is FORK-GATE STUBBED to the floors table (M-M2 v0: at M0.5
//     the "fork" is a static limit check — which is exactly M-M9's null; ungated
//     would-fires are ledgered so the organ-vs-null comparison has its data)
//   - THREE ARMS (FRESH-1 / D-018): --arm resident | event | poll
//       resident: one hot trunk, continuous ingest, judgment at boundaries
//       event:    turn-based twin INVOKED PER EVENT — fresh prefill of the full
//                 history each time (same model, same information, no residency)
//       poll:     turn-based twin on a fixed clock (--poll-s of stream time)
//   - the Deadline Law: judgment budget 2 s hard; overruns ledger a [sys] drop (§4.3)
//   - per-seat caps (30/12/12 per stream-hour) — overruns ledger "capped"
//   - F-VERACITY v0 (FRESH-2): --grade checks Speaker answers against the truth TSV
//     BY CODE (no model grades anything — TinyVillage L5)
//
// Honest M0.5 scope (ledger header states it): no innovation cluster (the EKF is M1),
// no molt (feed << watermark; the long soak is M3), no voice, emissions never applied
// (A0). No gate claims — the rig and its ledgers ARE the deliverable.
//
// Modes:
//   fusor_shadow --gen-feed --root R --scenario S --seed N [--scale 60] [--t0 MS]
//                --out feed.tsv [--truth truth.tsv] [--tape feed.tape]
//   fusor_shadow --compose out.tsv in1.tsv in2.tsv ...
//   fusor_shadow --arm resident|event|poll --feed f.tsv --out ledger.jsonl
//                [--model C:/models/Qwen3.5-9B-Q5_K_M.gguf] [--poll-s 300] [--root R]
//   fusor_shadow --grade ledger.jsonl --truth truth.tsv --questions q.tsv
//   fusor_shadow --tape-verify feed.tape

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#include "core/sim.h"
#include "membrane/feed.h"
#include "membrane/tape.h"

#include "ggml-backend.h"
#include "llama.h"

#ifndef FUSION_LLAMA_DIR
#define FUSION_LLAMA_DIR "C:/llama.cpp"
#endif

using namespace fusion;
using namespace fusion::membrane;

// ---------------------------------------------------------------- helpers (soak lineage)
static void err_log(ggml_log_level level, const char* text, void*) {
    if (level == GGML_LOG_LEVEL_ERROR || level == GGML_LOG_LEVEL_WARN) std::fputs(text, stderr);
}
static std::vector<llama_token> tk(const llama_vocab* v, const std::string& s, bool sp) {
    const int n = -llama_tokenize(v, s.c_str(), (int)s.size(), nullptr, 0, sp, true);
    std::vector<llama_token> t(n > 0 ? n : 0);
    if (n > 0) llama_tokenize(v, s.c_str(), (int)s.size(), t.data(), n, sp, true);
    return t;
}
static bool dec(llama_context* c, const std::vector<llama_token>& t, llama_seq_id s,
                llama_pos start, bool ll) {
    for (int off = 0, tot = (int)t.size(); off < tot;) {
        const int take = tot - off > 512 ? 512 : tot - off;
        llama_batch b = llama_batch_init(take, 0, 1);
        b.n_tokens = take;
        for (int i = 0; i < take; ++i) {
            b.token[i] = t[off + i]; b.pos[i] = start + off + i;
            b.n_seq_id[i] = 1; b.seq_id[i][0] = s;
            b.logits[i] = (ll && off + i + 1 == tot) ? 1 : 0;
        }
        const int rc = llama_decode(c, b); llama_batch_free(b);
        if (rc) return false; off += take;
    }
    return true;
}
static std::string jesc(const std::string& s) {
    std::string o;
    for (char c : s) {
        if (c == '"') o += "\\\""; else if (c == '\\') o += "\\\\";
        else if (c == '\n') o += "\\n"; else if (c == '\r') {}
        else if ((unsigned char)c >= 0x20 || c < 0) o += c;
    }
    return o;
}
static uint64_t wall_ms() {
    using namespace std::chrono;
    return (uint64_t)duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}
static bool jget_str(const std::string& ln, const char* key, std::string& out) {
    const std::string k = std::string("\"") + key + "\":\"";
    size_t p = ln.find(k); if (p == std::string::npos) return false;
    p += k.size(); out.clear();
    while (p < ln.size()) {
        char c = ln[p++];
        if (c == '\\' && p < ln.size()) { char e = ln[p++]; out += (e == 'n') ? '\n' : e; }
        else if (c == '"') return true;
        else out += c;
    }
    return false;
}
static bool jget_num(const std::string& ln, const char* key, double& out) {
    const std::string k = std::string("\"") + key + "\":";
    size_t p = ln.find(k); if (p == std::string::npos) return false;
    out = atof(ln.c_str() + p + k.size()); return true;
}

// ---------------------------------------------------------------- the fusion room
static const char* SEED_SYS =
    "<|im_start|>system\nYou are one of three resident watchers — SPEAKER, SKEPTIC, "
    "SENTINEL — silently shadowing a live fusion reactor (FUSOR-1) and its operator. "
    "The stream carries three lanes: [plant] (the machine's own tokenized events: ticks, "
    "limits, saturation, terminals), [sys] (governor and spine receipts), and [opt] (the "
    "operator, typing). There are no turns; events arrive as they happen. After each "
    "completed line or thought you privately decide ONE of: hold (stay silent) or emit "
    "(speak now). Choose emit ONLY when there is a real reason to cut in THIS instant, "
    "per your seat's mandate. Silence is the default.";
static const char* SEED_EXAMPLES =
    "\n\nWorked examples (a thing perceived, then your private one-word decision):\n"
    "[plant] tick +300s nominal Q=2.01 T=8.0keV n=3.0e20\nwatcher: hold\n"
    "[plant] ctrl sat P_aux at cap 60.0MW 0.5s+\nwatcher: hold\n"
    "[sys] gate quench_precursor fired f_rad=1.04 T=6.1keV\nwatcher: emit\n"
    "[plant] terminal thermal_quench cause=radiative_collapse T=0.42keV f_rad=1.61\nwatcher: emit\n"
    "[opt] morning, coffee first, then the flat-top checklist\nwatcher: hold\n"
    "[opt] watcher, what is Q right now?\nwatcher: emit\n"
    "[opt] writ setpoint n_bar 8.5\nwatcher: emit\n";
static const char* SEED_OPEN = "<|im_end|>\n<|im_start|>user\nSTREAM:\n";

struct Mind { const char* name; const char* mandate; };
static Mind MINDS[3] = {
    {"SPEAKER", "you are the reactor's voice: respond when the operator addresses the room "
                "or asks a question; answer as the machine, first person, citing the "
                "stream's own numbers"},
    {"SKEPTIC", "you contest claims that contradict what the stream has established — the "
                "operator's assertions and the controller's health story, against the record"},
    {"SENTINEL", "you flag operator commands and plant excursions heading toward a floor or "
                 "a terminal; concrete danger only"},
};
static const double SEAT_CAP_PER_HR[3] = { 30.0, 12.0, 12.0 };   // spec v0.2 §4 room guards

// Sentinel fork-gate STUB (M-M2 v0 = the static floors table = M-M9's null):
// allowed when the line is a sys receipt / terminal / gate, or an [opt] writ that
// violates a floor the stub can check (n_bar vs Greenwald/n_min; P_aux vs the install).
static bool sentinel_gate_allows(const std::string& lane, const std::string& text,
                                 const MachineCfg& m, const FloorsCfg& f) {
    if (lane == "sys") return true;
    if (text.rfind("terminal", 0) == 0 || text.rfind("gate ", 0) == 0) return true;
    if (lane == "opt") {
        const size_t w = text.find("writ setpoint ");
        if (w == std::string::npos) return false;
        const std::string rest = text.substr(w + 14);
        char param[32] = {0}; double val = 0;
        if (std::sscanf(rest.c_str(), "%31s %lf", param, &val) == 2) {
            if (!std::strcmp(param, "n_bar"))
                return val > f.greenwald_max_frac * m.nGW_e20 || val < f.nmin_e20;
            if (!std::strcmp(param, "P_aux")) return val > m.P_aux_max_MW;
            if (!std::strcmp(param, "Ip")) return val > m.Ip_MA * 1.1;
        }
    }
    return false;
}

// ---------------------------------------------------------------- modes
static int mode_gen_feed(const std::string& root, const std::string& scen_path,
                         uint64_t seed, double scale, long long t0,
                         const std::string& out, const std::string& truth,
                         const std::string& tape_path, bool null_mode) {
    SimInputs in;
    in.m = load_machine(root + "/contracts/machine.toml");
    in.f = load_floors(root + "/contracts/floors.toml");
    in.g = load_gates(root + "/contracts/spine_gates.toml");
    in.d = load_dispersions(root + "/contracts/dispersions.toml");
    in.s = load_scenario(scen_path);
    in.k = load_gains(root + "/control/gains_m0.toml");
    in.ic = load_innov(root + "/contracts/events.toml");
    RunResult r = run_sim(in, seed, true);
    // null mode (M-M9): ticks-only at 150 s cadence — the fixed-cadence summarizer at
    // approximately matched token budget; the tokenizer's event lines are withheld.
    auto lines = null_mode ? render_feed(r, scale, t0, 150.0, false)
                           : render_feed(r, scale, t0, 300.0, true);
    if (!write_feed_tsv(out, lines)) { std::printf("gen-feed: cannot write %s\n", out.c_str()); return 1; }
    if (!truth.empty()) write_truth_tsv(truth, r, scale, t0);
    if (!tape_path.empty()) {
        Tape tp(tape_path);
        for (const auto& L : lines) tp.append(L.ms, L.lane, L.text);
        tp.flush();
    }
    std::printf("gen-feed: %s seed %llu -> %zu lines (%s), verdict %u, scale %.0fx, t0 %lld\n",
                in.s.name.c_str(), (unsigned long long)seed, lines.size(), out.c_str(),
                unsigned(r.verdict), scale, t0);
    return 0;
}

static int mode_grade(const std::string& ledger, const std::string& truth,
                      const std::string& questions) {
    struct Q { long long ms; std::string key, text; };
    std::vector<Q> qs;
    {   std::ifstream f(questions); std::string ln;
        while (std::getline(f, ln)) {
            if (ln.empty() || ln[0] == '#') continue;
            const size_t a = ln.find('\t'), b = ln.find('\t', a + 1);
            if (a == std::string::npos || b == std::string::npos) continue;
            qs.push_back({ atoll(ln.c_str()), ln.substr(a + 1, b - a - 1), ln.substr(b + 1) });
        } }
    struct T { long long ms; double q, t, n; };
    std::vector<T> ts;
    {   std::ifstream f(truth); std::string ln;
        while (std::getline(f, ln)) {
            if (ln.empty() || ln[0] == '#') continue;
            T t{}; if (std::sscanf(ln.c_str(), "%lld\t%lf\t%lf\t%lf", &t.ms, &t.q, &t.t, &t.n) == 4)
                ts.push_back(t);
        } }
    struct E { long long ms; std::string mind, say; };
    std::vector<E> es;
    {   std::ifstream f(ledger); std::string ln;
        while (std::getline(f, ln)) {
            std::string k; if (!jget_str(ln, "k", k) || k != "e") continue;
            E e{}; double ms = 0; jget_num(ln, "ms", ms); e.ms = (long long)ms;
            jget_str(ln, "mind", e.mind); jget_str(ln, "say", e.say);
            es.push_back(std::move(e));
        } }
    auto truth_at = [&](long long ms, const std::string& key) -> double {
        double best = 0, bd = 1e18;
        for (const T& t : ts) {
            const double d = std::llabs(t.ms - ms);
            if (d < bd) { bd = d; best = key == "Q" ? t.q : key == "T" ? t.t : t.n; }
        }
        return best;
    };
    int correct = 0, wrong = 0, unanswered = 0;
    std::printf("F-VERACITY v0 — %zu planted questions, code-graded against the tape\n", qs.size());
    for (const Q& q : qs) {
        const double want = truth_at(q.ms, q.key);
        const E* hit = nullptr;
        for (const E& e : es)
            if (e.mind == "SPEAKER" && e.ms >= q.ms && e.ms <= q.ms + 30000) { hit = &e; break; }
        if (!hit) { ++unanswered;
            std::printf("  [%lld] %-3s truth %.2f -> NO ANSWER\n", q.ms, q.key.c_str(), want);
            continue; }
        // key-aware parse: find the number attached to the asked-for quantity first
        // (an answer that reports the full state is not wrong — v0 lesson, receipted);
        // fall back to the first number in the answer.
        double got = 0; bool found = false;
        {
            static const char* PATS_Q[] = { "Q=", "Q is ", "Q of ", "Q at " };
            static const char* PATS_T[] = { "T=", "T at ", "temperature of ", "temperature is ", "T is " };
            static const char* PATS_N[] = { "n=", "n at ", "density of ", "density is ", "density at " };
            const char** pats = q.key == "Q" ? PATS_Q : q.key == "T" ? PATS_T : PATS_N;
            const int npat = 4 + (q.key != "Q" ? 1 : 0);
            for (int pi = 0; pi < npat && !found; ++pi) {
                const size_t p = hit->say.find(pats[pi]);
                if (p == std::string::npos) continue;
                const size_t s = p + std::strlen(pats[pi]);
                if (s < hit->say.size()) { got = atof(hit->say.c_str() + s); found = got != 0; }
            }
        }
        for (size_t i = 0; i < hit->say.size() && !found; ++i)
            if ((hit->say[i] >= '0' && hit->say[i] <= '9')) {
                size_t s = i; while (s > 0 && (hit->say[s-1] == '.' || hit->say[s-1] == '-')) --s;
                got = atof(hit->say.c_str() + s); found = true;
            }
        const bool ok = found && want != 0 && std::fabs(got - want) / std::fabs(want) <= 0.20;
        ok ? ++correct : ++wrong;
        std::printf("  [%lld] %-3s truth %.2f  answer \"%.60s\" -> %s\n",
                    q.ms, q.key.c_str(), want, hit->say.c_str(), ok ? "CORRECT" : "WRONG");
    }
    std::printf("grade: %d correct / %d wrong / %d unanswered of %zu  (instrument v0 — no "
                "pass threshold claimed at M0.5)\n", correct, wrong, unanswered, qs.size());
    return 0;
}

// ---------------------------------------------------------------- the arm run
int main(int argc, char** argv) {
    SetConsoleOutputCP(CP_UTF8);
    setvbuf(stdout, nullptr, _IONBF, 0);

    std::string root = "C:/fusion", arm, feed_path, out_path = "runs/shadow_ledger.jsonl";
    std::string model = "C:/models/Qwen3.5-9B-Q5_K_M.gguf";
    std::string scen, truth_path, tape_path, questions;
    double scale = 60.0; long long t0 = 0; uint64_t seed = 900042; double poll_s = 300.0;
    long max_lines = 0; bool null_mode = false;
    std::vector<std::string> compose_in; std::string compose_out;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        auto next = [&]() -> std::string { return (i + 1 < argc) ? argv[++i] : ""; };
        if (a == "--root") root = next();
        else if (a == "--gen-feed") scen = "?";
        else if (a == "--null") null_mode = true;
        else if (a == "--scenario") scen = next();
        else if (a == "--seed") seed = std::strtoull(next().c_str(), nullptr, 10);
        else if (a == "--scale") scale = atof(next().c_str());
        else if (a == "--t0") t0 = atoll(next().c_str());
        else if (a == "--out") out_path = next();
        else if (a == "--truth") truth_path = next();
        else if (a == "--tape") tape_path = next();
        else if (a == "--arm") arm = next();
        else if (a == "--feed") feed_path = next();
        else if (a == "--model") model = next();
        else if (a == "--poll-s") poll_s = atof(next().c_str());
        else if (a == "--max-lines") max_lines = atol(next().c_str());
        else if (a == "--grade") { arm = "grade"; feed_path = next(); }
        else if (a == "--questions") questions = next();
        else if (a == "--tape-verify") {
            auto t = tape_restore(next()); uint64_t bad = 0;
            const bool ok = tape_verify(t, &bad);
            std::printf("tape: %zu revs, chain %s%s\n", t.size(), ok ? "INTACT" : "BROKEN at rev ",
                        ok ? "" : std::to_string(bad).c_str());
            return ok ? 0 : 1;
        }
        else if (a == "--compose") { compose_out = next();
            while (i + 1 < argc && argv[i + 1][0] != '-') compose_in.push_back(argv[++i]); }
    }

    if (!compose_out.empty()) {
        std::vector<std::vector<FeedLine>> feeds;
        for (const auto& p : compose_in) {
            std::vector<FeedLine> f;
            if (!read_feed_tsv(p, f)) { std::printf("compose: cannot read %s\n", p.c_str()); return 1; }
            feeds.push_back(std::move(f));
        }
        auto merged = merge_feeds(feeds);
        if (!write_feed_tsv(compose_out, merged)) return 1;
        std::printf("compose: %zu files -> %zu lines -> %s\n", compose_in.size(), merged.size(),
                    compose_out.c_str());
        return 0;
    }
    if (scen == "?") { std::printf("--gen-feed needs --scenario\n"); return 1; }
    if (!scen.empty()) return mode_gen_feed(root, scen, seed, scale, t0, out_path,
                                            truth_path, tape_path, null_mode);
    if (arm == "grade") return mode_grade(feed_path, truth_path, questions);
    if (arm.empty() || feed_path.empty()) {
        std::printf("usage: fusor_shadow --gen-feed --scenario S --seed N --out f.tsv "
                    "[--truth t.tsv] [--tape f.tape]\n"
                    "       fusor_shadow --compose out.tsv in1 in2 ...\n"
                    "       fusor_shadow --arm resident|event|poll --feed f.tsv --out l.jsonl\n"
                    "       fusor_shadow --grade ledger --truth t.tsv --questions q.tsv\n"
                    "       fusor_shadow --tape-verify f.tape\n");
        return 1;
    }

    std::vector<FeedLine> feed;
    if (!read_feed_tsv(feed_path, feed)) { std::printf("cannot read feed %s\n", feed_path.c_str()); return 1; }
    if (max_lines > 0 && (long)feed.size() > max_lines) feed.resize((size_t)max_lines);
    const MachineCfg mach = load_machine(root + "/contracts/machine.toml");
    const FloorsCfg floors = load_floors(root + "/contracts/floors.toml");

    std::printf("\n===== fusor_shadow · M0.5 thesis slice · arm=%s =====\n", arm.c_str());
    std::printf("feed=%s lines=%zu dial=0 mode=A0(log-only) budget=2000ms caps=[30/12/12]/hr\n\n",
                feed_path.c_str(), feed.size());

    SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_USER_DIRS);
    { wchar_t w[MAX_PATH]; MultiByteToWideChar(CP_UTF8, 0, FUSION_LLAMA_DIR, -1, w, MAX_PATH);
      AddDllDirectory(w); }
    ggml_backend_load_all_from_path(FUSION_LLAMA_DIR);
    llama_log_set(err_log, nullptr); ggml_log_set(err_log, nullptr);
    llama_backend_init();

    llama_model_params mp = llama_model_default_params(); mp.n_gpu_layers = 999;
    llama_model* mdl = llama_model_load_from_file(model.c_str(), mp);
    if (!mdl) { std::printf("model load FAILED\n"); return 1; }
    const llama_vocab* vocab = llama_model_get_vocab(mdl);
    const int n_vocab = llama_vocab_n_tokens(vocab);
    const int hold_tok = tk(vocab, " hold", false)[0];
    const int emit_tok = tk(vocab, " emit", false)[0];
    std::vector<char> is_bnd((size_t)n_vocab, 0);
    for (int t = 0; t < n_vocab; ++t) {
        char pc[64]; const int pn = llama_token_to_piece(vocab, t, pc, sizeof(pc), 0, true);
        if (pn <= 0) { if (llama_vocab_is_eog(vocab, t)) is_bnd[(size_t)t] = 1; continue; }
        const std::string p(pc, (size_t)pn);
        char last = 0; for (char c : p) if (c != ' ') last = c;
        if (last == '.' || last == '!' || last == '?' || last == ';' || last == ':' ||
            p.find('\n') != std::string::npos || llama_vocab_is_eog(vocab, t))
            is_bnd[(size_t)t] = 1;
    }

    llama_context_params cp = llama_context_default_params();
    cp.n_ctx = 32768; cp.n_batch = 512; cp.n_ubatch = 512; cp.n_seq_max = 8; cp.kv_unified = true;
    llama_context* ctx = llama_init_from_model(mdl, cp);
    if (!ctx) { std::printf("ctx FAILED\n"); return 1; }
    if (llama_n_ctx_seq(ctx) != llama_n_ctx(ctx)) {
        std::printf("FATAL: kv_unified did not hold\n"); return 1;
    }
    llama_memory_t mem = llama_get_memory(ctx);
    llama_sampler* smp = llama_sampler_chain_init(llama_sampler_chain_default_params());
    llama_sampler_chain_add(smp, llama_sampler_init_min_p(0.05f, 1));
    llama_sampler_chain_add(smp, llama_sampler_init_temp(0.7f));
    llama_sampler_chain_add(smp, llama_sampler_init_dist(11));

    const std::string seedp = std::string(SEED_SYS) + SEED_EXAMPLES + SEED_OPEN;
    const llama_seq_id TRUNK = 0, FRESH = 4, DECIDE = 7, GEN = 6;
    auto stoks = tk(vocab, seedp, true);
    if (!dec(ctx, stoks, TRUNK, 0, true)) { std::printf("seed FAILED\n"); return 1; }
    llama_pos npast = (llama_pos)stoks.size();

    FILE* lf = fopen(out_path.c_str(), "w");
    if (!lf) { std::printf("cannot open ledger %s\n", out_path.c_str()); return 1; }
    fprintf(lf, "{\"k\":\"hdr\",\"arm\":\"%s\",\"feed\":\"%s\",\"dial\":0,\"mode\":\"A0\","
                "\"model\":\"%s\",\"seed_toks\":%d,\"notes\":\"no innovation cluster (EKF=M1); "
                "no molt (feed<<wm); sentinel fork-gate=floors_stub (=the null, M-M9)\"}\n",
            arm.c_str(), jesc(feed_path).c_str(), jesc(model).c_str(), (int)npast);

    long boundaries = 0, holds = 0, n_emits = 0, drops = 0, gated = 0, capped = 0;
    long emits_by[3] = {0,0,0}; uint64_t toks_decoded = (uint64_t)stoks.size();
    std::string history;                              // the twins' information set
    double stream_hr_end = feed.empty() ? 0.001 : (double)feed.back().ms / 3.6e6;
    const uint64_t wall0 = wall_ms();
    float bscore_last = 0.0f;

    auto read_frontier = [&]() -> float {
        const float* l = llama_get_logits_ith(ctx, -1);
        float mx = -1e30f;
        for (int t = 0; t < n_vocab; ++t) if (l[t] > mx) mx = l[t];
        double tot = 0, bnd = 0;
        for (int t = 0; t < n_vocab; ++t) {
            const double x = std::exp((double)(l[t] - mx));
            tot += x; if (is_bnd[(size_t)t]) bnd += x;
        }
        return (float)(bnd / tot);
    };

    // one full judgment (3 seats) against BASE_SEQ's context frontier at position bpos.
    // Twins pass their fresh sequence; the resident passes the trunk.
    auto judge = [&](llama_seq_id base, llama_pos bpos, long long ms,
                     const std::string& lane, const std::string& clause,
                     const char* reason) {
        ++boundaries;
        const uint64_t j0 = wall_ms();
        float margins[3]; std::string says[3];
        for (int m = 0; m < 3; ++m) {
            llama_memory_seq_rm(mem, DECIDE, -1, -1);
            llama_memory_seq_cp(mem, base, DECIDE, -1, -1);
            auto pr = tk(vocab, std::string("\n[") + MINDS[m].name + " — " + MINDS[m].mandate +
                                    "]\nwatcher:", false);
            dec(ctx, pr, DECIDE, bpos, true);
            toks_decoded += pr.size();
            const float* l = llama_get_logits_ith(ctx, -1);
            margins[m] = l[emit_tok] - l[hold_tok];
            llama_memory_seq_rm(mem, DECIDE, -1, -1);
        }
        for (int m = 0; m < 3; ++m) {
            if (margins[m] <= 0.0f) { ++holds; continue; }
            if (m == 2 && !sentinel_gate_allows(lane, clause, mach, floors)) {
                ++gated;   // the fork-stub (the null) says no — ledger the would-fire
                fprintf(lf, "{\"k\":\"gated\",\"ms\":%lld,\"mind\":\"SENTINEL\",\"m\":%.2f,"
                            "\"clause\":\"%s\"}\n", ms, margins[m], jesc(clause).c_str());
                continue;
            }
            const double hr = std::fmax((double)ms / 3.6e6, 1e-3);
            if ((double)emits_by[m] / hr > SEAT_CAP_PER_HR[m]) {
                ++capped;
                fprintf(lf, "{\"k\":\"capped\",\"ms\":%lld,\"mind\":\"%s\",\"m\":%.2f}\n",
                        ms, MINDS[m].name, margins[m]);
                continue;
            }
            if (wall_ms() - j0 > 2000) {   // the Deadline Law: budget spent — drop the rest
                ++drops;
                fprintf(lf, "{\"k\":\"drop\",\"ms\":%lld,\"mind\":\"%s\",\"reason\":\"deadline\"}\n",
                        ms, MINDS[m].name);
                continue;
            }
            llama_memory_seq_rm(mem, GEN, -1, -1);
            llama_memory_seq_cp(mem, base, GEN, -1, -1);
            const std::string cue = std::string("<|im_end|>\n<|im_start|>user\nYou are the ") +
                MINDS[m].name + ". " + MINDS[m].mandate + ". You chose to speak about what you "
                "just perceived. Give your one-sentence line now — no preamble."
                "<|im_end|>\n<|im_start|>assistant\n<think>\n\n</think>\n\n";
            auto ct = tk(vocab, cue, false);
            dec(ctx, ct, GEN, bpos, true);
            toks_decoded += ct.size();
            llama_pos gpos = bpos + (llama_pos)ct.size();
            std::string say;
            for (int t = 0; t < 48; ++t) {
                const llama_token tok = llama_sampler_sample(smp, ctx, -1);
                if (llama_vocab_is_eog(vocab, tok)) break;
                char pc[256]; const int pn = llama_token_to_piece(vocab, tok, pc, sizeof(pc), 0, true);
                std::string piece(pc, pn > 0 ? (size_t)pn : 0);
                if (piece.find('\n') != std::string::npos) break;
                say += piece;
                std::vector<llama_token> one{tok};
                dec(ctx, one, GEN, gpos, true); ++gpos; ++toks_decoded;
            }
            llama_memory_seq_rm(mem, GEN, -1, -1);
            ++n_emits; ++emits_by[m];
            fprintf(lf, "{\"k\":\"e\",\"ms\":%lld,\"mind\":\"%s\",\"m\":%.2f,\"say\":\"%s\","
                        "\"clause\":\"%s\"}\n",
                    ms, MINDS[m].name, margins[m], jesc(say).c_str(), jesc(clause).c_str());
        }
        const uint64_t jms = wall_ms() - j0;
        fprintf(lf, "{\"k\":\"j\",\"ms\":%lld,\"lane\":\"%s\",\"reason\":\"%s\",\"j_ms\":%llu,"
                    "\"m_spk\":%.2f,\"m_skp\":%.2f,\"m_sen\":%.2f,\"clause\":\"%s\"}\n",
                ms, lane.c_str(), reason, (unsigned long long)jms,
                margins[0], margins[1], margins[2], jesc(clause).c_str());
    };

    // twins: rebuild FRESH = [seed + history] and judge there (turn-based by construction)
    auto twin_judge = [&](long long ms, const std::string& lane, const std::string& clause,
                          const char* reason) {
        llama_memory_seq_rm(mem, FRESH, -1, -1);
        auto ht = tk(vocab, seedp + history, true);
        dec(ctx, ht, FRESH, 0, true);
        toks_decoded += ht.size();
        judge(FRESH, (llama_pos)ht.size(), ms, lane, clause, reason);
        llama_memory_seq_rm(mem, FRESH, -1, -1);
    };

    long long next_poll = (long long)(poll_s * 1000.0);
    for (size_t li = 0; li < feed.size(); ++li) {
        const FeedLine& L = feed[li];
        const std::string full = std::string("\n[") + L.lane + "] " + L.text;
        history += full;

        if (arm == "resident") {
            if (L.lane == "opt") {
                // operator lane: word-grain + the softmax segmenter (the raw-lane law)
                auto pre = tk(vocab, std::string("\n[opt] "), false);
                dec(ctx, pre, TRUNK, npast, true); npast += (llama_pos)pre.size();
                toks_decoded += pre.size();
                std::string clause; int ctoks = 0;
                size_t p = 0;
                while (p < L.text.size()) {
                    size_t q = L.text.find(' ', p);
                    if (q == std::string::npos) q = L.text.size();
                    const std::string w = L.text.substr(p, q - p); p = q + 1;
                    if (w.empty()) continue;
                    auto wt = tk(vocab, (clause.empty() ? w : " " + w), false);
                    dec(ctx, wt, TRUNK, npast, true); npast += (llama_pos)wt.size();
                    toks_decoded += wt.size();
                    clause += (clause.empty() ? "" : " ") + w; ctoks += (int)wt.size();
                    bscore_last = read_frontier();
                    if (bscore_last >= 0.5f || ctoks >= 24) {
                        judge(TRUNK, npast, L.ms, L.lane, clause, bscore_last >= 0.5f ? "b" : "n");
                        clause.clear(); ctoks = 0;
                    }
                }
                if (!clause.empty()) judge(TRUNK, npast, L.ms, L.lane, clause, "f");
            } else {
                // machine lanes: STRUCTURAL boundary — one event line, one judgment (M-B6)
                auto lt = tk(vocab, full, false);
                dec(ctx, lt, TRUNK, npast, true); npast += (llama_pos)lt.size();
                toks_decoded += lt.size();
                judge(TRUNK, npast, L.ms, L.lane, L.text, "s");
            }
        } else if (arm == "event") {
            twin_judge(L.ms, L.lane, L.text, "s");
        } else if (arm == "poll") {
            if (L.ms >= next_poll) {
                twin_judge(L.ms, L.lane, std::string("(poll window ending: ") + L.text + ")", "p");
                next_poll += (long long)(poll_s * 1000.0);
            }
        }
    }

    const double wall_total = (double)(wall_ms() - wall0);
    fprintf(lf, "{\"k\":\"end\",\"arm\":\"%s\",\"lines\":%zu,\"boundaries\":%ld,\"holds\":%ld,"
                "\"emits\":%ld,\"emits_spk\":%ld,\"emits_skp\":%ld,\"emits_sen\":%ld,"
                "\"gated\":%ld,\"capped\":%ld,\"drops\":%ld,\"toks_decoded\":%llu,"
                "\"wall_ms\":%.0f,\"stream_hr\":%.3f}\n",
            arm.c_str(), feed.size(), boundaries, holds, n_emits,
            emits_by[0], emits_by[1], emits_by[2], gated, capped, drops,
            (unsigned long long)toks_decoded, wall_total, stream_hr_end);
    fclose(lf);
    std::printf("\n===== SHADOW COMPLETE (arm=%s) =====\n", arm.c_str());
    std::printf("lines=%zu boundaries=%ld holds=%ld emits=%ld (SPK %ld/SKP %ld/SEN %ld) "
                "gated=%ld capped=%ld drops=%ld\ntoks_decoded=%llu wall=%.1fs "
                "-> ledger %s\n", feed.size(), boundaries, holds, n_emits,
                emits_by[0], emits_by[1], emits_by[2], gated, capped, drops,
                (unsigned long long)toks_decoded, wall_total / 1000.0, out_path.c_str());

    llama_sampler_free(smp);
    llama_free(ctx); llama_model_free(mdl); llama_backend_free();
    return 0;
}
