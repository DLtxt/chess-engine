#include "jump_iterator.h"
#include "board.h"

template<size_t NUM_DIR>
void JumpIterator<NUM_DIR>::operator++() {
	bool found_step = false;
	// search adjacent squares, find valid move
	while (cur_dir < NUM_DIR && !found_step) {
		cloc = sloc;
		cloc[0] += dir[cur_dir][0];
		cloc[1] += dir[cur_dir][1] * mov_dir;
		++cur_dir;
		if (board->InBound(cloc)) {
			found_step = true;
		}
	}
	// no moves left
	if (!found_step) { board = nullptr; done = true; }
}

template
void JumpIterator<10>::operator++();

template
void JumpIterator<8>::operator++();

template
void JumpIterator<6>::operator++();

template
void JumpIterator<5>::operator++();

template
void JumpIterator<4>::operator++();

template
void JumpIterator<2>::operator++();

template
void JumpIterator<1>::operator++();