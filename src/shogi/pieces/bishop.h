#ifndef SHOGI_BISHOP_H
#define SHOGI_BISHOP_H

#include "shogi_piece.h"

class ShogiBishop : public ShogiPiece {
	
	static const int NUM_DIR = 4;
	int dir[NUM_DIR][2] = {
		{1, 1},
		{-1, 1},
		{-1, -1},
		{1, -1}
	};

	public:
	
		ShogiBishop(Board* board, const std::string& loc, char player) : ShogiPiece{board, loc, BISHOP, player} {}

		ShogiBishop(const ShogiPiece& other) : ShogiPiece{other} {
			name = BISHOP;
		}

		int Priority() const override { return ShogiPiece::BISHOP_RANK; }

		Iterator begin() override {
			auto iter = Iterator{std::make_shared<SlideIterator<ShogiBishop::NUM_DIR>>(board, loc, dir, player == BLACK ? 1 : -1)};
			++iter;
			return iter;
		}
};

#endif