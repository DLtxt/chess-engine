#ifndef MOVE_H
#define MOVE_H

#include "abstract_move.h"

class Board;
class Piece;

class Move final : public AbstractMove {

	Board *my_board;
	std::string from, to;

	int capture_count;
	std::shared_ptr<Piece> captured;

	public:

		Move(const std::string& from, const std::string& to);

		virtual void MakeMoveOn(Board*) override; // return true if execution is successful
		virtual void Undo() noexcept override; // any valid execution can be undone
};

#endif