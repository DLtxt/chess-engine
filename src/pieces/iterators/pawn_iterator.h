#ifndef PAWN_ITERATOR_H
#define PAWN_ITERATOR_H

#include "iterator.h"

class PawnIterator : public PieceIterator {
	
	static const size_t NUM_DIR = 3;
	int dir[NUM_DIR][2] = {
		{1, 1},
		{0, 1},
		{-1, 1},
	};
	size_t cur_dir;

	public:
		
		PawnIterator(Board* board, const std::string& loc) : PieceIterator{board, loc}, cur_dir{0} {}
		
		void operator++() override;
};

#endif 