#ifndef ENGINE_SELFPLAY_H
#define ENGINE_SELFPLAY_H

#include "search.h"

#include <cstdint>

namespace eng {

struct MatchConfig {
	int games = 100;
	// Per-side search limits. Giving the two sides different limits is how you
	// sanity-check that more search really is stronger.
	int depth_a = 6, depth_b = 6;
	int64_t movetime_a = 0, movetime_b = 0;
	// Plies of random legal moves used to vary the opening.
	int random_plies = 8;
	int max_plies = 300;
	// Adjudicate once one side is this far ahead for several plies.
	int resign_margin = 900;
	unsigned seed = 20240719u;
	bool verbose = false;
};

struct MatchResult {
	int wins = 0;    // for side A
	int draws = 0;
	int losses = 0;
	int games() const { return wins + draws + losses; }
	double score() const {
		return games() ? (wins + 0.5 * draws) / games() : 0.0;
	}
};

// Plays a match of engine A against engine B, alternating colours each game.
MatchResult run_match(const MatchConfig& cfg);

// Elo difference implied by a score rate, and the likelihood that A is really
// stronger than B given the observed result.
double elo_from_score(double score);
double likelihood_of_superiority(const MatchResult& r);

void print_match_report(const MatchResult& r);

} // namespace eng

#endif
