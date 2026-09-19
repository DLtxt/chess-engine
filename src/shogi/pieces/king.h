#ifndef SHOGI_KING_H
#define SHOGI_KING_H

#include "shogi_piece.h"

class ShogiKing : public ShogiPiece {

	static const int NUM_DIR = 8;
	int dir[NUM_DIR][2] = {
		{1, 0},
		{0, 1},
		{-1, 0},
		{0, -1},
		{1, 1},
		{-1, -1},
		{-1, 1},
		{1, -1},
	};

	public:
	
		ShogiKing(Board* board, const std::string& loc, char player) : ShogiPiece{board, loc, KING, player} {}

		bool IsKing() const override { return true; }
		
		int Priority() const override { return ShogiPiece::KING_RANK; }
		
		Iterator begin() override {
			auto iter = Iterator{std::make_shared<JumpIterator<ShogiKing::NUM_DIR>>(board, loc, dir, player == BLACK ? 1 : -1)};
			++iter;
			return iter;
		}
};

#endif