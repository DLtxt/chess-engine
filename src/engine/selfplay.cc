#include "selfplay.h"

#include "engine.h"
#include "eval.h"
#include "movegen.h"
#include "position.h"
#include "tt.h"

#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <random>

namespace eng {

namespace {

// Result of a single game, from White's point of view.
enum GameResult { kWhiteWins, kBlackWins, kDrawn };

GameResult play_game(const MatchConfig& cfg, bool a_is_white, std::mt19937& rng) {
	Position pos;
	pos.set_start();

	// Random opening plies so the match is not the same game repeated.
	for (int i = 0; i < cfg.random_plies; ++i) {
		Move moves[kMaxMoves];
		const int n = generate_legal(pos, moves);
		if (n == 0) break;
		pos.do_move(moves[std::uniform_int_distribution<int>(0, n - 1)(rng)]);
	}

	Searcher white_searcher, black_searcher;
	white_searcher.clear();
	black_searcher.clear();

	int resign_streak = 0;

	for (int ply = 0; ply < cfg.max_plies; ++ply) {
		Move moves[kMaxMoves];
		const int n = generate_legal(pos, moves);

		if (n == 0)
			return pos.in_check()
				? (pos.side_to_move() == kWhite ? kBlackWins : kWhiteWins)
				: kDrawn;

		if (pos.is_draw()) return kDrawn;

		const bool white_to_move = pos.side_to_move() == kWhite;
		const bool a_to_move = (white_to_move == a_is_white);

		SearchLimits limits;
		if (a_to_move) {
			limits.depth = cfg.depth_a;
			limits.movetime = cfg.movetime_a;
		} else {
			limits.depth = cfg.depth_b;
			limits.movetime = cfg.movetime_b;
		}
		// Without a depth or a clock the search would never return.
		if (limits.depth <= 0 && limits.movetime <= 0) limits.depth = 6;

		Searcher& s = white_to_move ? white_searcher : black_searcher;
		const SearchResult r = s.search(pos, limits, false);
		if (r.best == kMoveNone) return kDrawn;

		// Adjudicate hopeless positions rather than playing them out.
		const int white_score = white_to_move ? r.score : -r.score;
		if (std::abs(white_score) >= cfg.resign_margin) {
			if (++resign_streak >= 4)
				return white_score > 0 ? kWhiteWins : kBlackWins;
		} else {
			resign_streak = 0;
		}

		pos.do_move(r.best);
	}

	return kDrawn;
}

} // namespace

double elo_from_score(double score) {
	if (score <= 0.0) return -800.0;
	if (score >= 1.0) return 800.0;
	return -400.0 * std::log10(1.0 / score - 1.0);
}

double likelihood_of_superiority(const MatchResult& r) {
	const int n = r.games();
	if (n == 0) return 0.5;

	// Normal approximation over the wins/losses only -- draws carry no signal
	// about which side is stronger.
	const double w = r.wins, l = r.losses;
	if (w + l == 0.0) return 0.5;
	const double z = (w - l) / std::sqrt(w + l);
	return 0.5 * (1.0 + std::erf(z / std::sqrt(2.0)));
}

MatchResult run_match(const MatchConfig& cfg) {
	MatchResult result;
	std::mt19937 rng(cfg.seed);

	for (int g = 0; g < cfg.games; ++g) {
		// Alternate colours so that any first-move advantage cancels out.
		const bool a_is_white = (g % 2 == 0);

		TT.clear();
		clear_eval_caches();

		const GameResult gr = play_game(cfg, a_is_white, rng);

		if (gr == kDrawn) ++result.draws;
		else if ((gr == kWhiteWins) == a_is_white) ++result.wins;
		else ++result.losses;

		if (cfg.verbose || (g + 1) % 10 == 0) {
			std::printf("\rgame %d/%d   +%d =%d -%d   ",
			            g + 1, cfg.games, result.wins, result.draws, result.losses);
			std::fflush(stdout);
		}
	}
	std::printf("\n");
	return result;
}

void print_match_report(const MatchResult& r) {
	const double score = r.score();
	const double elo = elo_from_score(score);
	const double los = likelihood_of_superiority(r);

	// Standard error on the score rate, converted to an Elo band.
	const int n = r.games();
	double stderr_score = 0.0;
	if (n > 1) {
		const double p = score;
		stderr_score = std::sqrt(p * (1.0 - p) / n);
	}
	const double elo_lo = elo_from_score(std::max(0.0, score - 1.96 * stderr_score));
	const double elo_hi = elo_from_score(std::min(1.0, score + 1.96 * stderr_score));

	std::printf("\n");
	std::printf("games   %d\n", n);
	std::printf("record  +%d =%d -%d\n", r.wins, r.draws, r.losses);
	std::printf("score   %.1f%%\n", score * 100.0);
	std::printf("elo     %+.1f   [%.1f, %.1f] 95%%\n", elo, elo_lo, elo_hi);
	std::printf("LOS     %.1f%%\n", los * 100.0);
	if (los > 0.95)      std::printf("verdict A is stronger.\n");
	else if (los < 0.05) std::printf("verdict B is stronger.\n");
	else                 std::printf("verdict inconclusive -- play more games.\n");
}

} // namespace eng
