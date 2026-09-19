#ifndef SHOGI_SILVER_H
#define SHOGI_SILVER_H

#include "shogi_piece.h"

class ShogiSilver : public ShogiPiece {

	static const int NUM_DIR = 5;
	int dir[NUM_DIR][2] = {
		{-1, 1},
		{0, 1},
		{1, 1},
		{-1, -1},
		{1, -1}
	};

	public:
	
		ShogiSilver(Board* board, const std::string& loc, char player) : ShogiPiece{board, loc, SILVER, player} {}
		
		int Priority() const override { return ShogiPiece::SILVER_RANK; }

		Iterator begin() override {
			auto iter = Iterator{std::make_shared<JumpIterator<ShogiSilver::NUM_DIR>>(board, loc, dir, player == BLACK ? 1 : -1)};
			++iter;
			return iter;
		}
};

#endif