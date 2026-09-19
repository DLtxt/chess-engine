#include "promotion.h"
#include "pieces/pawn.h"
#include "board.h"

Promotion::Promotion(const std::string& from, const std::string& to, char promotion, bool promote) : from{from}, to{to}, captured{nullptr}, promoted{nullptr}, promotion{promotion}, promote{promote} {}

void Promotion::MakeMoveOn(Board* board) {
	my_board = board;

	if (!(*board)[from]->CanMove(to)) throw _invalid_move_{"Can't move from " + from + " to " + to};
	
	captured = board->Retrieve((*board)[from]->CaptureLoc(to));
	
	if (captured != nullptr) board->Capture(captured->Name(), captured->Player());

	// handle promotion
	if ((*board)[from]->CanPromote(to) && promotion != EMPTY && promote) {
		promoted = board->Retrieve(from);
		board->SetPiece(from, promotion, promoted->Player());
	}
	
	(*board)[from]->TakeMove(to); // then move pawn / promoted piece 
}

void Promotion::Undo() noexcept {
	(*my_board)[to]->UndoMove(from);
	my_board->Place(std::move(promoted));
	
	if (captured != nullptr) {
		my_board->Release(captured->Player());
		my_board->Place(std::move(captured));
	}
}