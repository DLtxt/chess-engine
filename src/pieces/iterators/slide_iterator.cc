#include "slide_iterator.h"
#include "board.h"

template<size_t NUM_DIR>
void SlideIterator<NUM_DIR>::operator++() {
	// search for moves on current direction
	std::string next_loc = cloc;
	if (board->Empty(cloc) || cloc == sloc) {
		next_loc[0] += dir[cur_dir][0];
		next_loc[1] += dir[cur_dir][1] * mov_dir;
		if (board->InBound(next_loc)) {
			cloc = next_loc;
			return;
		}
	}
	// switch direction
	while (++cur_dir < NUM_DIR) {
		next_loc = sloc;
		next_loc[0] += dir[cur_dir][0];
		next_loc[1] += dir[cur_dir][1] * mov_dir;
		if (board->InBound(next_loc)) {
			cloc = next_loc;
			return;
		}
	}
	// no moves left
	board = nullptr;
	done = true;
}

template
void SlideIterator<8>::operator++();

template
void SlideIterator<4>::operator++();

template
void SlideIterator<2>::operator++();

template
void SlideIterator<1>::operator++();