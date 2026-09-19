#include "search.h"

#include "eval.h"
#include "movegen.h"
#include "tt.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace eng {

int Searcher::reductions_[64][64];
bool Searcher::reductions_ready_ = false;

namespace {

int64_t now_ms() {
	using namespace std::chrono;
	return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

// Ordering scores. The bands are spaced far enough apart that a move can never
// jump from one class into another through its within-class bonus.
const int kScoreTT        = 1 << 24;
const int kScoreGoodCap   = 1 << 22;
const int kScoreKiller1   = (1 << 21) + 1000;
const int kScoreKiller2   = (1 << 21);
const int kScoreCounter   = (1 << 20);
const int kScoreBadCap    = -(1 << 22);

int piece_value_on(const Position& pos, int sq) {
	const Piece pc = pos.piece_on(sq);
	return pc == kNoPiece ? 0 : kSeeValue[type_of(pc)];
}

// What this move stands to win outright, used for delta pruning.
int capture_gain(const Position& pos, Move m) {
	int gain = move_type(m) == kEnPassant ? kSeeValue[kPawn]
	                                      : piece_value_on(pos, to_sq(m));
	if (move_type(m) == kPromotion)
		gain += kSeeValue[promotion_type(m)] - kSeeValue[kPawn];
	return gain;
}

std::string score_to_uci(int score) {
	if (is_mate_score(score)) {
		const int plies = score > 0 ? kValueMate - score : -kValueMate - score;
		// UCI reports mate distance in moves, rounding away from zero.
		const int moves = (plies + (plies > 0 ? 1 : -1)) / 2;
		return "mate " + std::to_string(moves);
	}
	return "cp " + std::to_string(score);
}

} // namespace

void Searcher::init_reductions() {
	for (int d = 1; d < 64; ++d)
		for (int m = 1; m < 64; ++m)
			reductions_[d][m] = int(0.75 + std::log(double(d)) * std::log(double(m)) / 2.25);
	reductions_[0][0] = 0;
	reductions_ready_ = true;
}

void Searcher::clear() {
	if (!reductions_ready_) init_reductions();
	std::memset(killers_, 0, sizeof(killers_));
	std::memset(history_, 0, sizeof(history_));
	std::memset(counter_moves_, 0, sizeof(counter_moves_));
	std::memset(prev_move_, 0, sizeof(prev_move_));
	std::memset(pv_, 0, sizeof(pv_));
	std::memset(pv_len_, 0, sizeof(pv_len_));
	for (int i = 0; i < kMaxPly; ++i) eval_stack_[i] = kValueNone;
	nodes_ = 0;
	stop_ = false;
}

int64_t Searcher::elapsed() const { return now_ms() - start_ms_; }

bool Searcher::out_of_time() {
	if (node_limit_ && nodes_ >= node_limit_) return true;
	if (!use_clock_) return false;
	return elapsed() >= hard_limit_;
}

void Searcher::init_time(const SearchLimits& l, Colour us) {
	start_ms_ = now_ms();
	node_limit_ = l.nodes;
	use_clock_ = false;

	if (l.movetime > 0) {
		use_clock_ = true;
		soft_limit_ = hard_limit_ = std::max<int64_t>(1, l.movetime - 20);
		return;
	}
	if (l.infinite || l.depth > 0 || l.nodes > 0) return;

	const int64_t remaining = l.time[us];
	if (remaining <= 0) return;

	use_clock_ = true;
	const int64_t inc = l.inc[us];
	const int moves_to_go = l.movestogo > 0 ? l.movestogo : 30;

	// Spend a slice of the remaining time, plus most of the increment, and keep
	// a small reserve so we never flag on the way back out of the search.
	const int64_t reserve = std::min<int64_t>(50, remaining / 2);
	int64_t alloc = remaining / moves_to_go + inc * 3 / 4;

	soft_limit_ = std::min(alloc, remaining - reserve);
	hard_limit_ = std::min(alloc * 4, remaining - reserve);
	soft_limit_ = std::max<int64_t>(1, soft_limit_);
	hard_limit_ = std::max<int64_t>(soft_limit_, hard_limit_);
}

// ------------------------------------------------------- move ordering ----

void Searcher::pick_best(Move* moves, int* scores, int n, int start) {
	int best = start;
	for (int i = start + 1; i < n; ++i)
		if (scores[i] > scores[best]) best = i;
	if (best != start) {
		std::swap(moves[start], moves[best]);
		std::swap(scores[start], scores[best]);
	}
}

void Searcher::score_moves(const Position& pos, Move* moves, int* scores, int n,
                           Move tt_move, int ply) const {
	const Colour us = pos.side_to_move();
	const Move prev = ply > 0 ? prev_move_[ply - 1] : kMoveNone;
	const Move counter = prev != kMoveNone
		? counter_moves_[pos.piece_on(to_sq(prev)) == kNoPiece ? 0 : pos.piece_on(to_sq(prev))][to_sq(prev)]
		: kMoveNone;

	for (int i = 0; i < n; ++i) {
		const Move m = moves[i];

		if (m == tt_move) { scores[i] = kScoreTT; continue; }

		const bool capture = pos.is_capture(m);
		const bool promotion = move_type(m) == kPromotion;

		if (capture || promotion) {
			// MVV-LVA orders within a class; SEE decides which class.
			const int victim = move_type(m) == kEnPassant ? kSeeValue[kPawn]
			                                             : piece_value_on(pos, to_sq(m));
			const int attacker = kSeeValue[type_of(pos.piece_on(from_sq(m)))];
			int bonus = victim * 16 - attacker;
			if (promotion) bonus += kSeeValue[promotion_type(m)];

			scores[i] = pos.see_ge(m, 0) ? kScoreGoodCap + bonus : kScoreBadCap + bonus;
			continue;
		}

		if (m == killers_[ply][0])      { scores[i] = kScoreKiller1; continue; }
		if (m == killers_[ply][1])      { scores[i] = kScoreKiller2; continue; }
		if (counter != kMoveNone && m == counter) { scores[i] = kScoreCounter; continue; }

		scores[i] = history_[us][from_sq(m)][to_sq(m)];
	}
}

void Searcher::score_captures(const Position& pos, Move* moves, int* scores, int n) const {
	for (int i = 0; i < n; ++i) {
		const Move m = moves[i];
		const int victim = move_type(m) == kEnPassant ? kSeeValue[kPawn]
		                                             : piece_value_on(pos, to_sq(m));
		const int attacker = kSeeValue[type_of(pos.piece_on(from_sq(m)))];
		scores[i] = victim * 16 - attacker;
		if (move_type(m) == kPromotion) scores[i] += kSeeValue[promotion_type(m)];
	}
}

void Searcher::update_pv(int ply, Move m) {
	pv_[ply][0] = m;
	for (int i = 0; i < pv_len_[ply + 1]; ++i)
		pv_[ply][i + 1] = pv_[ply + 1][i];
	pv_len_[ply] = pv_len_[ply + 1] + 1;
}

void Searcher::update_quiet_stats(const Position& pos, Move best, Move* quiets,
                                  int quiet_count, int depth, int ply) {
	const Colour us = pos.side_to_move();

	if (killers_[ply][0] != best) {
		killers_[ply][1] = killers_[ply][0];
		killers_[ply][0] = best;
	}

	const Move prev = ply > 0 ? prev_move_[ply - 1] : kMoveNone;
	if (prev != kMoveNone) {
		const Piece pc = pos.piece_on(to_sq(prev));
		if (pc != kNoPiece) counter_moves_[pc][to_sq(prev)] = best;
	}

	// Reward the move that caused the cutoff and penalise the quiets that were
	// tried before it, scaled by depth. The cap keeps the table from saturating.
	const int bonus = std::min(depth * depth * 4, 1200);
	int& h = history_[us][from_sq(best)][to_sq(best)];
	h += bonus - h * std::abs(bonus) / 8192;

	for (int i = 0; i < quiet_count; ++i) {
		if (quiets[i] == best) continue;
		int& hq = history_[us][from_sq(quiets[i])][to_sq(quiets[i])];
		hq += -bonus - hq * std::abs(bonus) / 8192;
	}
}

// --------------------------------------------------- quiescence search ----

int Searcher::qsearch(Position& pos, int alpha, int beta, int ply) {
	if ((nodes_ & 1023) == 0 && out_of_time()) stop_ = true;
	if (stop_) return kValueDraw;

	++nodes_;

	if (pos.is_draw()) return kValueDraw;
	if (ply >= kMaxPly - 1) return evaluate(pos);

	const bool in_check = pos.in_check();
	int best = -kValueInfinite;

	if (!in_check) {
		// Standing pat: we are never forced to capture, so the static score is
		// a lower bound on what this position is worth.
		best = evaluate(pos);
		if (best >= beta) return best;
		if (best > alpha) alpha = best;
	}

	Move moves[kMaxMoves];
	const int n = in_check ? generate_legal(pos, moves) : generate_captures(pos, moves);

	if (n == 0) return in_check ? -kValueMate + ply : best;

	int scores[kMaxMoves];
	score_captures(pos, moves, scores, n);

	for (int i = 0; i < n; ++i) {
		pick_best(moves, scores, n, i);
		const Move m = moves[i];

		if (!in_check) {
			// Delta pruning: if winning this capture outright still leaves us
			// far below alpha, nothing downstream will rescue it.
			if (best + capture_gain(pos, m) + 200 < alpha) continue;
			// Never bother searching a capture that loses material outright.
			if (!pos.see_ge(m, 0)) continue;
		}

		pos.do_move(m);
		const int score = -qsearch(pos, -beta, -alpha, ply + 1);
		pos.undo_move(m);

		if (stop_) return kValueDraw;

		if (score > best) {
			best = score;
			if (score > alpha) {
				alpha = score;
				if (alpha >= beta) break;
			}
		}
	}

	return best;
}

// ------------------------------------------------------- main search -----

int Searcher::negamax(Position& pos, int depth, int alpha, int beta, int ply, bool is_pv) {
	if ((nodes_ & 1023) == 0 && out_of_time()) stop_ = true;
	if (stop_) return kValueDraw;

	pv_len_[ply] = 0;

	if (depth <= 0) return qsearch(pos, alpha, beta, ply);

	++nodes_;

	if (ply > 0) {
		if (pos.is_draw()) return kValueDraw;
		if (ply >= kMaxPly - 1) return evaluate(pos);

		// Mate-distance pruning: a mate already found closer to the root cannot
		// be improved on from here.
		alpha = std::max(alpha, -kValueMate + ply);
		beta  = std::min(beta, kValueMate - ply - 1);
		if (alpha >= beta) return alpha;
	}

	const bool in_check = pos.in_check();
	const bool root = (ply == 0);

	// --- transposition table ---------------------------------------------
	bool tt_hit = false;
	const TTEntry* tte = TT.probe(pos.key(), tt_hit);
	Move tt_move = kMoveNone;

	if (tt_hit) {
		tt_move = tte->move;
		const int tt_score = score_from_tt(tte->score, ply);
		if (!is_pv && !root && tte->depth >= depth) {
			const Bound b = tte->bound();
			if (b == kBoundExact
			 || (b == kBoundLower && tt_score >= beta)
			 || (b == kBoundUpper && tt_score <= alpha))
				return tt_score;
		}
	}
	// A key collision can hand back a move that is not playable here.
	if (tt_move != kMoveNone && !(pos.pseudo_legal(tt_move) && pos.legal(tt_move)))
		tt_move = kMoveNone;

	// --- static evaluation ------------------------------------------------
	const int static_eval = in_check ? kValueNone : evaluate(pos);
	eval_stack_[ply] = static_eval;

	const bool improving = !in_check && ply >= 2
	                    && eval_stack_[ply - 2] != kValueNone
	                    && static_eval > eval_stack_[ply - 2];

	// --- whole-node pruning -----------------------------------------------
	if (!is_pv && !in_check && !root && std::abs(beta) < kValueMateInMaxPly) {

		// Reverse futility: we are so far ahead that giving back a margin still
		// beats beta, so this node will not fail low.
		if (depth <= 6 && static_eval - 85 * (depth - (improving ? 1 : 0)) >= beta)
			return static_eval;

		// Null move: if passing the turn still fails high, the position is won
		// well enough to prune. Skipped in pawn endings, where zugzwang bites.
		if (depth >= 3 && static_eval >= beta
		    && pos.non_pawn_material(pos.side_to_move()) > 0) {
			const int R = 3 + depth / 6 + std::min((static_eval - beta) / 200, 3);
			pos.do_null_move();
			prev_move_[ply] = kMoveNone;
			const int score = -negamax(pos, depth - R, -beta, -beta + 1, ply + 1, false);
			pos.undo_null_move();
			if (stop_) return kValueDraw;
			if (score >= beta)
				return is_mate_score(score) ? beta : score;
		}
	}

	// --- move loop ---------------------------------------------------------
	Move moves[kMaxMoves];
	const int n = generate_legal(pos, moves);

	// No legal move at all: mate if we are in check, stalemate otherwise. This
	// single test replaces separate checkmate and stalemate detection.
	if (n == 0) return in_check ? -kValueMate + ply : kValueDraw;

	int scores[kMaxMoves];
	score_moves(pos, moves, scores, n, tt_move, ply);

	Move best_move = kMoveNone;
	int best_score = -kValueInfinite;
	Bound bound = kBoundUpper;

	Move quiets[64];
	int quiet_count = 0;

	for (int i = 0; i < n; ++i) {
		pick_best(moves, scores, n, i);
		const Move m = moves[i];

		const bool capture = pos.is_capture(m) || move_type(m) == kPromotion;

		// --- shallow pruning of clearly unpromising moves ---
		if (!root && !is_pv && !in_check && best_score > -kValueMateInMaxPly) {
			if (!capture) {
				// Late move pruning: deep in a bad move list, stop looking.
				if (depth <= 5 && quiet_count >= 4 + depth * depth) continue;
				// Futility: this quiet move cannot plausibly reach alpha.
				if (depth <= 6 && static_eval != kValueNone
				    && static_eval + 120 + 90 * depth <= alpha) continue;
			} else if (depth <= 5 && !pos.see_ge(m, -20 * depth * depth)) {
				continue;
			}
		}

		pos.do_move(m);
		prev_move_[ply] = m;

		const bool gives_check = pos.in_check();
		int new_depth = depth - 1;
		if (gives_check) ++new_depth;  // check extension

		int score;
		if (i == 0) {
			score = -negamax(pos, new_depth, -beta, -alpha, ply + 1, is_pv);
		} else {
			// Late move reductions: search later quiet moves shallower first,
			// and only re-search at full depth if one of them beats alpha.
			int r = 0;
			if (depth >= 3 && i >= 3 && !capture && !in_check && !gives_check) {
				r = reductions_[std::min(depth, 63)][std::min(i, 63)];
				if (is_pv) --r;
				if (!improving) ++r;
				if (m == killers_[ply][0] || m == killers_[ply][1]) --r;
				r = std::max(0, std::min(r, new_depth - 1));
			}

			// Principal variation search: prove the rest of the moves are worse
			// with a null window, and only open it up when one is not.
			score = -negamax(pos, new_depth - r, -alpha - 1, -alpha, ply + 1, false);

			if (score > alpha && r > 0)
				score = -negamax(pos, new_depth, -alpha - 1, -alpha, ply + 1, false);

			if (score > alpha && score < beta)
				score = -negamax(pos, new_depth, -beta, -alpha, ply + 1, is_pv);
		}

		pos.undo_move(m);

		if (stop_) return kValueDraw;

		if (!capture && quiet_count < 64) quiets[quiet_count++] = m;

		if (score > best_score) {
			best_score = score;
			best_move = m;

			if (score > alpha) {
				alpha = score;
				bound = kBoundExact;
				update_pv(ply, m);

				if (alpha >= beta) {
					bound = kBoundLower;
					if (!capture)
						update_quiet_stats(pos, m, quiets, quiet_count, depth, ply);
					break;
				}
			}
		}
	}

	TT.store(pos.key(), best_move, score_to_tt(best_score, ply),
	         in_check ? kValueNone : static_eval, depth, bound);

	return best_score;
}

// ------------------------------------------------ iterative deepening ----

SearchResult Searcher::search(Position& pos, const SearchLimits& limits, bool print_info) {
	stop_ = false;
	nodes_ = 0;
	std::memset(pv_, 0, sizeof(pv_));
	std::memset(pv_len_, 0, sizeof(pv_len_));
	for (int i = 0; i < kMaxPly; ++i) eval_stack_[i] = kValueNone;

	init_time(limits, pos.side_to_move());
	TT.new_search();

	SearchResult result;

	// Always have something legal to return, even if we are stopped instantly.
	Move root_moves[kMaxMoves];
	const int root_count = generate_legal(pos, root_moves);
	if (root_count == 0) return result;
	result.best = root_moves[0];

	const int max_depth = limits.depth > 0 ? std::min(limits.depth, kMaxPly - 2) : kMaxPly - 2;

	int prev_score = 0;

	for (int depth = 1; depth <= max_depth; ++depth) {
		root_depth_ = depth;

		int score;
		if (depth <= 4) {
			score = negamax(pos, depth, -kValueInfinite, kValueInfinite, 0, true);
		} else {
			// Aspiration windows: assume this iteration lands near the last one
			// and widen only when that guess is wrong.
			int window = 18;
			int alpha = std::max(prev_score - window, -kValueInfinite);
			int beta  = std::min(prev_score + window, kValueInfinite);

			while (true) {
				score = negamax(pos, depth, alpha, beta, 0, true);
				if (stop_) break;

				if (score <= alpha) {
					beta = (alpha + beta) / 2;
					alpha = std::max(score - window, -kValueInfinite);
				} else if (score >= beta) {
					beta = std::min(score + window, kValueInfinite);
				} else {
					break;
				}
				window += window / 2 + 6;
			}
		}

		if (stop_ && depth > 1) break;

		prev_score = score;
		result.score = score;
		result.depth = depth;
		result.nodes = nodes_;
		result.elapsed_ms = elapsed();

		if (pv_len_[0] > 0) {
			result.best = pv_[0][0];
			result.ponder = pv_len_[0] > 1 ? pv_[0][1] : kMoveNone;
		}

		if (print_info) {
			const int64_t ms = std::max<int64_t>(1, result.elapsed_ms);
			std::cout << "info depth " << depth
			          << " score " << score_to_uci(score)
			          << " nodes " << nodes_
			          << " nps " << (nodes_ * 1000 / ms)
			          << " hashfull " << TT.hashfull()
			          << " time " << result.elapsed_ms
			          << " pv";
			for (int i = 0; i < pv_len_[0]; ++i)
				std::cout << ' ' << move_to_uci(pv_[0][i]);
			std::cout << std::endl;
		}

		// A forced mate has been proved; searching deeper cannot improve on it.
		if (is_mate_score(score) && limits.depth == 0) break;

		// Stop starting an iteration we have no realistic chance of finishing.
		if (use_clock_ && elapsed() >= soft_limit_ * 6 / 10) break;
		if (node_limit_ && nodes_ >= node_limit_) break;
	}

	return result;
}

SearchResult search_parallel(const Position& pos, const SearchLimits& limits,
                             int threads, bool print_info) {
	threads = std::max(1, threads);

	if (threads == 1) {
		Position copy = pos;
		Searcher solo;
		return solo.search(copy, limits, print_info);
	}

	std::vector<std::unique_ptr<Searcher>> searchers;
	searchers.reserve(threads);
	for (int i = 0; i < threads; ++i) searchers.push_back(std::make_unique<Searcher>());

	std::vector<SearchResult> results(threads);
	std::vector<std::thread> helpers;
	helpers.reserve(threads - 1);

	for (int i = 1; i < threads; ++i) {
		helpers.emplace_back([&, i]() {
			Position copy = pos;

			// Helpers run without a clock of their own and are stopped by the
			// main thread, so they never decide when the move is due.
			SearchLimits l = limits;
			l.movetime = 0;
			l.time[kWhite] = l.time[kBlack] = 0;
			l.inc[kWhite] = l.inc[kBlack] = 0;
			l.nodes = 0;
			l.infinite = true;

			// Staggering the depth gives the helpers different move orders, so
			// they fill the shared table with work the main thread has not done.
			if (l.depth > 0) l.depth = std::min(l.depth + (i % 3), kMaxPly - 2);

			results[i] = searchers[i]->search(copy, l, false);
		});
	}

	Position main_copy = pos;
	results[0] = searchers[0]->search(main_copy, limits, print_info);

	for (auto& s : searchers) s->stop();
	for (auto& t : helpers) t.join();

	// Report the whole pool's work, not just the main thread's share.
	uint64_t total = 0;
	for (const SearchResult& r : results) total += r.nodes;
	results[0].nodes = total;

	return results[0];
}

} // namespace eng
