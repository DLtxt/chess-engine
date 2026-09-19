#include "zobrist.h"

namespace eng {
namespace zobrist {

Key psq[kPieceNb][64];
Key side;
Key castling[16];
Key ep_file[8];

namespace {

// xorshift64* -- a fixed seed keeps hashes reproducible between runs, which
// matters when comparing search output across builds.
Key next_random(Key& s) {
	s ^= s >> 12;
	s ^= s << 25;
	s ^= s >> 27;
	return s * 2685821657736338717ULL;
}

} // namespace

void init() {
	Key s = 1070372ULL;
	for (int p = 0; p < kPieceNb; ++p)
		for (int sq = 0; sq < 64; ++sq)
			psq[p][sq] = next_random(s);
	side = next_random(s);
	for (int i = 0; i < 16; ++i) castling[i] = next_random(s);
	for (int f = 0; f < 8; ++f) ep_file[f] = next_random(s);
}

} // namespace zobrist
} // namespace eng
