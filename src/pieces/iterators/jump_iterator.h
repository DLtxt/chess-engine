#ifndef JUMP_ITERATOR_H
#define JUMP_ITERATOR_H

#include "pieces/iterators/iterator.h"

template<size_t NUM_DIR>
class JumpIterator : public PieceIterator {
	
	int dir[NUM_DIR][2];
	size_t cur_dir;
	int mov_dir;

	public:
		
		JumpIterator(Board* board, const std::string& loc, int (&ndir)[NUM_DIR][2], int mov_dir) : PieceIterator{board, loc}, cur_dir{0}, mov_dir{mov_dir} {
			for (size_t i = 0; i < NUM_DIR; ++i) {
				dir[i][0] = ndir[i][0];
				dir[i][1] = ndir[i][1];
			}
		}
		
		void operator++() override;
};

#endif