// Entry point for the standalone search engine.
//
//   ./engine                 speak UCI on stdin/stdout (use it from any GUI)
//   ./engine perfttest       run the move generator against known node counts
//   ./engine perft <fen> <d> per-move node counts for one position
//   ./engine bench [depth]   fixed workload, for comparing builds
//   ./engine selfplay ...    play a match of the engine against itself

#include "engine.h"
#include "perft.h"
#include "position.h"
#include "selfplay.h"
#include "uci.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

namespace {

int run_selfplay(int argc, char** argv) {
	eng::MatchConfig cfg;

	for (int i = 0; i < argc; ++i) {
		const std::string arg = argv[i];
		const char* next = (i + 1 < argc) ? argv[i + 1] : nullptr;
		if (!next) break;

		if      (arg == "-games")    { cfg.games = std::atoi(next); ++i; }
		else if (arg == "-depth")    { cfg.depth_a = cfg.depth_b = std::atoi(next); ++i; }
		else if (arg == "-deptha")   { cfg.depth_a = std::atoi(next); ++i; }
		else if (arg == "-depthb")   { cfg.depth_b = std::atoi(next); ++i; }
		else if (arg == "-movetime") { cfg.movetime_a = cfg.movetime_b = std::atoll(next); cfg.depth_a = cfg.depth_b = 0; ++i; }
		else if (arg == "-random")   { cfg.random_plies = std::atoi(next); ++i; }
		else if (arg == "-seed")     { cfg.seed = unsigned(std::atoi(next)); ++i; }
	}

	std::cout << "A: depth " << cfg.depth_a << "   B: depth " << cfg.depth_b
	          << "   games " << cfg.games << std::endl;

	const eng::MatchResult r = eng::run_match(cfg);
	eng::print_match_report(r);
	return 0;
}

} // namespace

int main(int argc, char** argv) {
	eng::init_engine(64);

	if (argc > 1) {
		const std::string cmd = argv[1];

		if (cmd == "perfttest")
			return eng::perft_suite() ? 0 : 1;

		if (cmd == "bench") {
			eng::bench(argc > 2 ? std::atoi(argv[2]) : 8);
			return 0;
		}

		if (cmd == "selfplay")
			return run_selfplay(argc - 2, argv + 2);

		if (cmd == "perft") {
			eng::Position pos;
			int depth = 1;
			if (argc > 3) {
				if (!pos.set_fen(argv[2])) {
					std::cerr << "bad fen\n";
					return 1;
				}
				depth = std::atoi(argv[3]);
			} else if (argc > 2) {
				pos.set_start();
				depth = std::atoi(argv[2]);
			}
			eng::perft_divide(pos, depth);
			return 0;
		}

		std::cerr << "unknown command: " << cmd << "\n";
		return 1;
	}

	eng::uci_loop();
	return 0;
}
