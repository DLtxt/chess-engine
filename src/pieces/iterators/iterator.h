#ifndef ITERATOR_H
#define ITERATOR_H

#include <string>
#include <iostream>
#include <algorithm>

class Board;

class PieceIterator {
	
	protected:
		
		Board* board;
		std::string sloc, cloc;
		size_t cur_dir;
		bool done;

	public:

		PieceIterator() : board{nullptr}, done{true} {}

		PieceIterator(Board* board, const std::string& loc) : board{board}, sloc{loc}, cloc{loc}, cur_dir{0}, done{false} {}

		virtual ~PieceIterator() {}
		
		bool operator!=(PieceIterator& other) {
			return (!done || !other.done) && (sloc != other.sloc || cloc != other.cloc);
		}

		std::string operator*() { return cloc; }
		virtual void operator++() {}

		bool Done() { return done; }
};

#endif 