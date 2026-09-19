#ifndef COMPUTER_LEVEL_5
#define COMPUTER_LEVEL_5

#include <string>

class Board;
class ComputerPlayer;

// Level 5 hands the position to the bitboard search engine in src/engine and
// plays whatever it returns. Unlike levels 1-4 it has no fallback: the search
// always produces a legal move unless the side to move has none at all.
class ComputerLevel5 {
public:
	explicit ComputerLevel5(int movetime_ms = 2000) : movetime_ms_{movetime_ms} {}

	void MakeMove(ComputerPlayer* player);

	void SetMoveTime(int ms) { movetime_ms_ = ms; }
	void SetDepth(int depth) { depth_ = depth; }
	void SetThreads(int threads) { threads_ = threads; }

private:
	int movetime_ms_;
	int depth_ = 0;     // when non-zero, search to a fixed depth instead
	int threads_ = 1;   // Lazy SMP search threads
};

// Renders the legacy board as a FEN string. Exposed for testing the bridge.
std::string BoardToFen(Board* board, char player);

#endif
