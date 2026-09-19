#include "pawn_iterator.h"
#include "board.h"

void PawnIterator::operator++() {
	// search for regular moves, including capturing
	int player_dir = board->GetPiecePlayer(sloc) == WHITE ? 1 : -1;
	bool found_next = false;
	while (cur_dir < NUM_DIR && !found_next) {
		auto next_loc = sloc;
		next_loc[0] += dir[cur_dir][0];
		next_loc[1] += dir[cur_dir][1] * player_dir; // white pawn moves up, black pawn moves down
		if (board->InBound(next_loc)) {
			cloc = next_loc;
			found_next = true;
		}
		++cur_dir;
	}
	// can pawn make two-square initiation move?
	if (cur_dir == NUM_DIR && !found_next && !(*board)[sloc]->HasMoved()) {
		auto next_loc = sloc;
		next_loc[1] += 2 * player_dir;
		std::string up_loc = sloc;
		up_loc[1] += player_dir;
		if (board->InBound(next_loc) && board->Empty(next_loc) && board->Empty(up_loc)) {
			cloc = next_loc;
			found_next = true;
		}
		++cur_dir;
	}
	// no moves left
	if (!found_next) { board = nullptr; done = true; }
}