#include "perft.h"

#include "movegen.h"

#include <chrono>
#include <iostream>

namespace eng {

uint64_t perft(Position& pos, int depth) {
	if (depth == 0) return 1;

	Move moves[kMaxMoves];
	const int n = generate_legal(pos, moves);

	// At depth 1 the move count is the answer, so skip the make/unmake.
	if (depth == 1) return uint64_t(n);

	uint64_t total = 0;
	for (int i = 0; i < n; ++i) {
		pos.do_move(moves[i]);
		total += perft(pos, depth - 1);
		pos.undo_move(moves[i]);
	}
	return total;
}

void perft_divide(Position& pos, int depth) {
	Move moves[kMaxMoves];
	const int n = generate_legal(pos, moves);

	uint64_t total = 0;
	for (int i = 0; i < n; ++i) {
		pos.do_move(moves[i]);
		const uint64_t sub = depth > 1 ? perft(pos, depth - 1) : 1;
		pos.undo_move(moves[i]);
		std::cout << move_to_uci(moves[i]) << ": " << sub << '\n';
		total += sub;
	}
	std::cout << "\nNodes: " << total << std::endl;
}

namespace {

struct PerftCase {
	const char* fen;
	int depth;
	uint64_t expected;
	const char* name;
};

// The standard positions. Between them these cover castling, en passant,
// promotion, pins, discovered check and double check.
const PerftCase kCases[] = {
	{ "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 5, 4865609, "startpos" },
	{ "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 4, 4085603, "kiwipete" },
	{ "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 6, 11030083, "position 3" },
	{ "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 5, 15833292, "position 4" },
	{ "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 4, 2103487, "position 5" },
	{ "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 4, 3894594, "position 6" },
};

} // namespace

bool perft_suite() {
	using namespace std::chrono;

	bool all_ok = true;
	uint64_t total_nodes = 0;
	const auto start = steady_clock::now();

	for (const PerftCase& c : kCases) {
		Position pos;
		if (!pos.set_fen(c.fen)) {
			std::cout << "FAIL  " << c.name << ": could not parse FEN\n";
			all_ok = false;
			continue;
		}

		const uint64_t got = perft(pos, c.depth);
		total_nodes += got;
		const bool ok = (got == c.expected);
		all_ok = all_ok && ok;

		std::cout << (ok ? "ok    " : "FAIL  ")
		          << c.name << "  depth " << c.depth
		          << "  got " << got << "  expected " << c.expected << '\n';
	}

	const auto ms = duration_cast<milliseconds>(steady_clock::now() - start).count();
	std::cout << "\n" << total_nodes << " nodes in " << ms << " ms";
	if (ms > 0) std::cout << "  (" << (total_nodes / uint64_t(ms)) << " knps)";
	std::cout << "\n" << (all_ok ? "All perft tests passed." : "PERFT FAILURES -- move generation is wrong.") << std::endl;

	return all_ok;
}

} // namespace eng
