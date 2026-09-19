#include "move.h"
#include "board.h"
#include <assert.h>

Move::Move(const std::string& from, const std::string& to) : from{from}, to{to}, capture_count{0}, captured{nullptr} {}

void Move::MakeMoveOn(Board* board) {
	my_board = board;

	if (!(*board)[from]->CanMove(to)) throw _invalid_move_{"Can't move from " + from + " to " + to};

	captured = board->Retrieve((*board)[from]->CaptureLoc(to));

	if (captured != nullptr) board->Capture(captured->Name(), captured->Player());

	(*board)[from]->TakeMove(to);
}

void Move::Undo() noexcept {
	(*my_board)[to]->UndoMove(from);
	
	if (captured != nullptr) {
		my_board->Release(captured->Player());
		my_board->Place(std::move(captured));
	}
}