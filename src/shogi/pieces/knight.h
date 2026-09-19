#ifndef SHOGI_KNIGHT_H
#define SHOGI_KNIGHT_H

#include "shogi_piece.h"

class ShogiKnight : public ShogiPiece {

	static const int NUM_DIR = 2;
	int dir[NUM_DIR][2] = {
		{1, 2},
		{-1, 2}
	};

	public:
	
		ShogiKnight(Board* board, const std::string& loc, char player) : ShogiPiece{board, loc, KNIGHT, player} {}
		
		int Priority() const override { return ShogiPiece::KNIGHT_RANK; }

		bool CanDrop() override;

		Iterator begin() override {
			auto iter = Iterator{std::make_shared<JumpIterator<ShogiKnight::NUM_DIR>>(board, loc, dir, player == BLACK ? 1 : -1)};
			++iter;
			return iter;
		}
};

#endif