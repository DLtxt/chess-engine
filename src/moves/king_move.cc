#include "king_move.h"
#include "chessboard.h"

KingMove::KingMove(const std::string& from, const std::string& to) : from{from}, to{to} {}

void KingMove::MakeMoveOn(Board* board) {
	my_board = board;

	if (!(*my_board)[from]->CanMove(to)) throw _invalid_move_{"Can't move from " + from + " to " + to};
	// valid move
	if ((*my_board)[from]->IsCastling(to)) { // if castling
		rook_dest = from;
		rook_dest[0] += (to[0] - from[0]) / 2;
		rook_loc = rook_dest;

		while (my_board->InBound(rook_loc)) {
			if (toupper(my_board->GetPieceName(rook_loc)) == ROOK) break;
			rook_loc[0] += (to[0] - from[0]) / 2;
		}

		(*my_board)[rook_loc]->TakeMove(rook_dest);
	}

	captured = my_board->Retrieve(to);

	if (captured != nullptr) my_board->Capture(captured->Name(), captured->Player());

	(*my_board)[from]->TakeMove(to);
	my_board->KingIsHere(to);
}

void KingMove::Undo() noexcept {
	if (!rook_dest.empty()) (*my_board)[rook_dest]->UndoMove(rook_loc);
	(*my_board)[to]->UndoMove(from);
	
	my_board->KingIsHere(from);

	if (captured != nullptr) {
		my_board->Release(captured->Player());
		my_board->Place(std::move(captured));
	}
}