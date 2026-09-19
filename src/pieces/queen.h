#ifndef QUEEN_H
#define QUEEN_H

#include "piece.h"
#include "iterators/slide_iterator.h"

class Queen : public Piece {

	static const size_t NUM_DIR = 8;
	int dir[NUM_DIR][2] = {
		{1, 0},
		{1, 1},
		{0, 1},
		{-1, 1},
		{-1, 0},
		{-1, -1},
		{0, -1},
		{1, -1}
	};

	public:
		Queen(Board* board, const std::string& loc, char player) : Piece{board, loc, QUEEN, player} {}

		Queen(const Piece& other) : Piece{other} {
			name = QUEEN;
		}

		int Priority() const override { return Piece::QUEEN_RANK; }

		Iterator begin() override {
			auto iter = Iterator{std::make_shared<SlideIterator<Queen::NUM_DIR>>(board, loc, dir, player == WHITE ? 1 : -1)};
			++iter;
			return iter;
		}
};

#endif