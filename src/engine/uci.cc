#include "uci.h"

#include "book.h"
#include "eval.h"
#include "movegen.h"
#include "perft.h"
#include "position.h"
#include "search.h"
#include "tt.h"

#include <chrono>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <string>
#include <thread>

namespace eng {

namespace {

const char* kEngineName = "ChessEngine 2.0";
const char* kEngineAuthor = "CS246 chess engine";

Position g_pos;
Searcher g_searcher;
std::thread g_thread;
int g_threads = 1;

// Abort a running search and reclaim the thread. Used by "stop" and "quit".
void join_search() {
	if (g_thread.joinable()) {
		g_searcher.stop();
		g_thread.join();
	}
}

// Let a running search finish on its own. Used when stdin reaches EOF, so that
// piping a script into the engine reports the move it was asked for rather than
// whatever it had after a few nodes.
void wait_search() {
	if (g_thread.joinable()) g_thread.join();
}

// Turns "e2e4" or "e7e8q" into the matching legal move, or kMoveNone.
Move parse_move(const Position& pos, const std::string& text) {
	Move moves[kMaxMoves];
	const int n = generate_legal(pos, moves);
	for (int i = 0; i < n; ++i)
		if (move_to_uci(moves[i]) == text) return moves[i];
	return kMoveNone;
}

void cmd_position(std::istringstream& is) {
	std::string token;
	is >> token;

	if (token == "startpos") {
		g_pos.set_start();
		is >> token;  // consume "moves" if present
	} else if (token == "fen") {
		std::string fen;
		while (is >> token && token != "moves") fen += token + " ";
		if (!g_pos.set_fen(fen)) {
			std::cout << "info string invalid fen" << std::endl;
			return;
		}
	} else {
		return;
	}

	if (token == "moves" || token == "startpos") {
		while (is >> token) {
			const Move m = parse_move(g_pos, token);
			if (m == kMoveNone) {
				std::cout << "info string illegal move " << token << std::endl;
				break;
			}
			g_pos.do_move(m);
		}
	}
}

void cmd_go(std::istringstream& is) {
	join_search();

	SearchLimits limits;
	std::string token;

	while (is >> token) {
		if      (token == "wtime")     is >> limits.time[kWhite];
		else if (token == "btime")     is >> limits.time[kBlack];
		else if (token == "winc")      is >> limits.inc[kWhite];
		else if (token == "binc")      is >> limits.inc[kBlack];
		else if (token == "movestogo") is >> limits.movestogo;
		else if (token == "movetime")  is >> limits.movetime;
		else if (token == "depth")     is >> limits.depth;
		else if (token == "nodes")     is >> limits.nodes;
		else if (token == "infinite")  limits.infinite = true;
		else if (token == "perft") {
			int d = 1;
			is >> d;
			Position copy = g_pos;
			perft_divide(copy, d);
			return;
		}
	}

	// An opening-book hit answers instantly, with no search at all.
	const Move book = probe_book(g_pos);
	if (book != kMoveNone) {
		std::cout << "info string book move" << std::endl;
		std::cout << "bestmove " << move_to_uci(book) << std::endl;
		return;
	}

	// Search on its own thread so that "stop" can still be read from stdin.
	g_thread = std::thread([limits]() {
		const SearchResult r = (g_threads > 1)
			? search_parallel(g_pos, limits, g_threads, true)
			: [&] { Position copy = g_pos; return g_searcher.search(copy, limits, true); }();
		std::cout << "bestmove " << move_to_uci(r.best);
		if (r.ponder != kMoveNone) std::cout << " ponder " << move_to_uci(r.ponder);
		std::cout << std::endl;
	});
}

void cmd_setoption(std::istringstream& is) {
	std::string token, name, value;
	is >> token;  // "name"
	while (is >> token && token != "value") name += (name.empty() ? "" : " ") + token;
	while (is >> token) value += (value.empty() ? "" : " ") + token;

	if (name == "Hash") {
		try { TT.resize(size_t(std::stoi(value))); } catch (...) {}
	} else if (name == "Threads") {
		try { g_threads = std::max(1, std::min(64, std::stoi(value))); } catch (...) {}
	} else if (name == "OwnBook") {
		set_book_enabled(value == "true" || value == "1");
	} else if (name == "Clear Hash") {
		TT.clear();
	}
}

} // namespace

void bench(int depth) {
	// A fixed set of positions spanning opening, middlegame and endgame.
	const char* positions[] = {
		"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
		"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
		"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
		"r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5Q2/PPPP1PPP/RNB1K1NR w KQkq - 4 4",
		"8/8/8/2k5/8/2K5/4P3/8 w - - 0 1",
		"r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
	};

	using namespace std::chrono;
	uint64_t total = 0;
	const auto start = steady_clock::now();

	for (const char* fen : positions) {
		Position pos;
		if (!pos.set_fen(fen)) continue;

		TT.clear();
		clear_eval_caches();

		Searcher s;
		SearchLimits limits;
		limits.depth = depth;
		const SearchResult r = s.search(pos, limits, false);
		total += r.nodes;

		std::cout << "  depth " << r.depth << "  " << r.nodes << " nodes  best "
		          << move_to_uci(r.best) << "  score " << r.score << '\n';
	}

	const auto ms = duration_cast<milliseconds>(steady_clock::now() - start).count();
	std::cout << "\nbench: " << total << " nodes  " << ms << " ms  "
	          << (ms > 0 ? total * 1000 / uint64_t(ms) : 0) << " nps" << std::endl;
}

void uci_loop() {
	std::string line;

	while (std::getline(std::cin, line)) {
		std::istringstream is(line);
		std::string token;
		is >> token;

		if (token == "uci") {
			std::cout << "id name " << kEngineName << '\n'
			          << "id author " << kEngineAuthor << '\n'
			          << "option name Hash type spin default 64 min 1 max 4096\n"
			          << "option name Threads type spin default 1 min 1 max 64\n"
		          << "option name OwnBook type check default true\n"
		          << "option name Clear Hash type button\n"
			          << "uciok" << std::endl;
		}
		else if (token == "isready")    { std::cout << "readyok" << std::endl; }
		else if (token == "ucinewgame") { join_search(); TT.clear(); clear_eval_caches(); g_searcher.clear(); g_pos.set_start(); }
		else if (token == "position")   { join_search(); cmd_position(is); }
		else if (token == "go")         { cmd_go(is); }
		else if (token == "stop")       { join_search(); }
		else if (token == "setoption")  { join_search(); cmd_setoption(is); }
		else if (token == "d")          { std::cout << g_pos.to_string() << std::flush; }
		else if (token == "eval")       { std::cout << "eval " << evaluate(g_pos) << std::endl; }
		else if (token == "perft")      { int d = 1; is >> d; Position c = g_pos; perft_divide(c, d); }
		else if (token == "perfttest")  { perft_suite(); }
		else if (token == "book")       { book_validate(true); }
		else if (token == "bench")      { int d = 8; is >> d; bench(d); }
		else if (token == "quit")       { join_search(); break; }
	}

	// Reaching here without "quit" means stdin closed; finish what we started.
	wait_search();
}

} // namespace eng
