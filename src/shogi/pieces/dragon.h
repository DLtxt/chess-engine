#ifndef SHOGI_DRAGON_H
#define SHOGI_DRAGON_H

#include "shogi_piece.h"

class ShogiDragon : public ShogiPiece {
	
	static const int NUM_DIR_JUMP = 4;
	int dir_jump[NUM_DIR_JUMP][2] = {
		{1, 1},
		{-1, 1},
		{-1, -1},
		{1, -1}
	};

	static const int NUM_DIR_SLIDE = 4;
	int dir_slide[NUM_DIR_SLIDE][2] = {
		{1, 0},
		{-1, 0},
		{0, -1},
		{0, 1}
	};

	public:
	
		ShogiDragon(Board* board, const std::string& loc, char player) : ShogiPiece{board, loc, DRAGON, player} {}

		ShogiDragon(const ShogiPiece& other) : ShogiPiece{other} {
			name = DRAGON;
		}

		int Priority() const override { return ShogiPiece::DRAGON_RANK; }

		Iterator begin() override {
			auto iter = Iterator{std::make_shared<JumpIterator<ShogiDragon::NUM_DIR_JUMP>>(board, loc, dir_jump, player == BLACK ? 1 : -1), std::make_shared<SlideIterator<ShogiDragon::NUM_DIR_SLIDE>>(board, loc, dir_slide, player == BLACK ? 1 : -1)};
			++iter;
			return iter;
		}
};

#endif