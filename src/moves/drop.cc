#include "drop.h"
#include "board.h"

void Drop::MakeMoveOn(Board* board) {
	my_board = board;

	if (!my_board->Empty(loc)) throw _invalid_move_{"You can't drop a piece onto another piece!"};

	for (const auto piece : board->CapturedBy(player)) {
		if (piece == name) {
			my_board->SetPiece(loc, name, player);
			board->RemoveCapture(name, player);
			if (!(*board)[loc]->CanDrop()) {
				Undo();
				throw _invalid_move_{"You can't drop your piece here!"};
			}
			return;
		}
	}

	throw _invalid_move_{"Can't find the piece to drop in your captures!"};
}

void Drop::Undo() noexcept {
	my_board->Capture(name, player == WHITE ? BLACK : WHITE);
	my_board->RemovePiece(loc);
} 