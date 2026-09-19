#ifndef DROP_H
#define DROP_H

#include "moves/abstract_move.h"

class Board;
class Piece;

class Drop final : public AbstractMove {

	Board *my_board;
	std::string loc;
	char name, player;

	public:

		Drop(const std::string& loc, char name, char player) : loc{loc}, name{name}, player{player} {}

		void MakeMoveOn(Board*) override; // return true if execution is successful
		void Undo() noexcept override; // any valid execution can be undone
};

#endif