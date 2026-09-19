#include "engine.h"

#include "bitboard.h"
#include "eval.h"
#include "tt.h"
#include "zobrist.h"

namespace eng {

void init_engine(size_t hash_mb) {
	init_bitboards();
	zobrist::init();
	init_eval();
	TT.resize(hash_mb);
}

} // namespace eng
