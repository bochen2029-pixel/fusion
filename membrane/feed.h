// membrane/feed.h — render the plant's POD events + golden series into the text feed
// (the tokenizer of machine experience, v0). Text exists ONLY here, post-run (M-M11).
#pragma once
#include "core/sim.h"
#include <string>
#include <vector>

namespace fusion::membrane {

struct FeedLine { long long ms; std::string lane, text; };

// Render one run: events -> vocabulary lines; ticks synthesized from the golden series
// at events.toml cadence in SCALED stream time (scale: sim seconds -> stream seconds).
// events_on=false is THE TOKENIZER NULL (M-M9): ticks-only at a faster cadence
// (approximately matched token budget), no event lines — the fixed-cadence summarizer.
std::vector<FeedLine> render_feed(const RunResult& r, double scale, long long t0_ms,
                                  double tick_interval_stream_s, bool events_on = true);

// Ground truth for the F-VERACITY grader: scaled ms -> (Q, T, ne20), 1 s sim cadence.
void write_truth_tsv(const std::string& path, const RunResult& r, double scale,
                     long long t0_ms);

bool write_feed_tsv(const std::string& path, const std::vector<FeedLine>& lines);
bool read_feed_tsv(const std::string& path, std::vector<FeedLine>& out);
std::vector<FeedLine> merge_feeds(const std::vector<std::vector<FeedLine>>& feeds);

} // namespace fusion::membrane
