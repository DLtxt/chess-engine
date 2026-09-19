#include "engine.h"

#include "bitboard.h"
#include "book.h"
#include "eval.h"
#include "tt.h"
#include "zobrist.h"

namespace eng {

void init_engine(size_t hash_mb) {
	init_bitboards();
	zobrist::init();
	init_eval();
	init_book();
	TT.resize(hash_mb);
}

} // namespace eng
