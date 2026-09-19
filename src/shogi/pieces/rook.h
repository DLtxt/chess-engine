#ifndef SHOGI_ROOK_H
#define SHOGI_ROOK_H

#include "shogi_piece.h"

class ShogiRook : public ShogiPiece {
	
	static const int NUM_DIR = 4;
	int dir[NUM_DIR][2] = {
		{1, 0},
		{0, 1},
		{-1, 0},
		{0, -1},
	};

	public:
	
		ShogiRook(Board* board, const std::string& loc, char player) : ShogiPiece{board, loc, ROOK, player} {}

		int Priority() const override { return ShogiPiece::ROOK_RANK; }
		
		Iterator begin() override {
			auto iter = Iterator{std::make_shared<SlideIterator<ShogiRook::NUM_DIR>>(board, loc, dir, player == BLACK ? 1 : -1)};
			++iter;
			return iter;
		}
};

#endif