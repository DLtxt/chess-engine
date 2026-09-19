#ifndef SHOGI_LANCE_H
#define SHOGI_LANCE_H

#include "shogi_piece.h"

class ShogiLance : public ShogiPiece {
	
	static const int NUM_DIR = 1;
	int dir_slide[NUM_DIR][2] = {
		{0, 1},
	};

	public:
	
		ShogiLance(Board* board, const std::string& loc, char player) : ShogiPiece{board, loc, LANCE, player} {}

		int Priority() const override { return ShogiPiece::LANCE_RANK; }

		bool CanDrop() override;

		Iterator begin() override {
			auto iter = Iterator{std::make_shared<SlideIterator<ShogiLance::NUM_DIR>>(board, loc, dir_slide, player == BLACK ? 1 : -1)};
			++iter;
			return iter;
		}
};

#endif