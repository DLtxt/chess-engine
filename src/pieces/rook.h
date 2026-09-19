#ifndef ROOK_H
#define ROOK_H

#include "piece.h"
#include "iterators/slide_iterator.h"

class Rook : public Piece {

	static const int NUM_DIR = 4;
	int dir[NUM_DIR][2] = {
		{1, 0},
		{0, 1},
		{-1, 0},
		{0, -1},
	};

	public:
	Rook(Board* board, const std::string& loc, char player) : Piece{board, loc, ROOK, player} {}

	Rook(const Piece& other) : Piece{other} {
		name = ROOK;
	}

	bool IsRook() const override { return true; }

	int Priority() const override { return Piece::ROOK_RANK; }

	Iterator begin() override {
		auto iter = Iterator{std::make_shared<SlideIterator<Rook::NUM_DIR>>(board, loc, dir, player == WHITE ? 1 : -1)};
		++iter;
		return iter;
	}
};

#endif