#ifndef ENGINE_SEARCH_H
#define ENGINE_SEARCH_H

#include "position.h"

#include <atomic>
#include <cstdint>

namespace eng {

struct SearchLimits {
	int64_t time[kColourNb] = { 0, 0 };
	int64_t inc[kColourNb]  = { 0, 0 };
	int movestogo = 0;
	int64_t movetime = 0;
	int depth = 0;
	uint64_t nodes = 0;
	bool infinite = false;
};

struct SearchResult {
	Move best = kMoveNone;
	Move ponder = kMoveNone;
	int score = 0;
	int depth = 0;
	uint64_t nodes = 0;
	int64_t elapsed_ms = 0;
};

class Searcher {
public:
	Searcher() { clear(); }

	// Runs iterative deepening until the limits are hit. When `print_info` is
	// set, a UCI "info" line is emitted after every completed iteration.
	SearchResult search(Position& pos, const SearchLimits& limits, bool print_info);

	void stop() { stop_ = true; }
	void clear();

	uint64_t nodes() const { return nodes_; }

private:
	int negamax(Position& pos, int depth, int alpha, int beta, int ply, bool is_pv);
	int qsearch(Position& pos, int alpha, int beta, int ply);

	void score_moves(const Position& pos, Move* moves, int* scores, int n,
	                 Move tt_move, int ply) const;
	void score_captures(const Position& pos, Move* moves, int* scores, int n) const;
	static void pick_best(Move* moves, int* scores, int n, int start);

	void update_pv(int ply, Move m);
	void update_quiet_stats(const Position& pos, Move best, Move* quiets,
	                        int quiet_count, int depth, int ply);

	bool out_of_time();
	void init_time(const SearchLimits& limits, Colour us);
	int64_t elapsed() const;

	std::atomic<bool> stop_{false};
	uint64_t nodes_ = 0;
	uint64_t node_limit_ = 0;

	int64_t start_ms_ = 0;
	int64_t soft_limit_ = 0;
	int64_t hard_limit_ = 0;
	bool use_clock_ = false;

	int root_depth_ = 0;

	// Move-ordering memory.
	Move killers_[kMaxPly][2];
	int history_[kColourNb][64][64];
	Move counter_moves_[kPieceNb][64];
	Move prev_move_[kMaxPly];

	// Triangular principal-variation table.
	Move pv_[kMaxPly][kMaxPly];
	int pv_len_[kMaxPly];

	int eval_stack_[kMaxPly];

	static int reductions_[64][64];
	static bool reductions_ready_;
	static void init_reductions();
};

} // namespace eng

#endif
