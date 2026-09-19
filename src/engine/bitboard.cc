#include "bitboard.h"

#include <algorithm>
#include <cstdlib>

namespace eng {

Bitboard kFileBB[8];
Bitboard kRankBB[8];
Bitboard kAdjacentFiles[8];

Bitboard kRays[kDirectionNb][64];
Bitboard kPawnAttacks[kColourNb][64];
Bitboard kKnightAttacks[64];
Bitboard kKingAttacks[64];

Bitboard kBetween[64][64];
Bitboard kLine[64][64];

Bitboard kForwardFile[kColourNb][64];
Bitboard kPassedPawnMask[kColourNb][64];
Bitboard kForwardRanks[kColourNb][64];
Bitboard kKingZone[kColourNb][64];

int kSquareDistance[64][64];

namespace {

// File/rank deltas for each of the eight ray directions, in table order.
const int kDirFile[kDirectionNb] = {  0, +1, +1, +1,  0, -1, -1, -1 };
const int kDirRank[kDirectionNb] = { +1, +1,  0, -1, -1, -1,  0, +1 };

constexpr Direction opposite(Direction d) { return Direction((int(d) + 4) & 7); }

bool on_board(int file, int rank) {
	return file >= 0 && file < 8 && rank >= 0 && rank < 8;
}

} // namespace

void init_bitboards() {
	for (int f = 0; f < 8; ++f) {
		kFileBB[f] = kFileABB << f;
		kRankBB[f] = kRank1BB << (8 * f);
	}
	for (int f = 0; f < 8; ++f) {
		kAdjacentFiles[f] = 0;
		if (f > 0) kAdjacentFiles[f] |= kFileBB[f - 1];
		if (f < 7) kAdjacentFiles[f] |= kFileBB[f + 1];
	}

	for (int a = 0; a < 64; ++a)
		for (int b = 0; b < 64; ++b)
			kSquareDistance[a][b] = std::max(std::abs(file_of(a) - file_of(b)),
			                                 std::abs(rank_of(a) - rank_of(b)));

	// Rays: walk outward from each square until the edge of the board.
	for (int sq = 0; sq < 64; ++sq) {
		for (int d = 0; d < kDirectionNb; ++d) {
			Bitboard ray = 0;
			int f = file_of(sq) + kDirFile[d];
			int r = rank_of(sq) + kDirRank[d];
			while (on_board(f, r)) {
				ray |= square_bb(make_square(f, r));
				f += kDirFile[d];
				r += kDirRank[d];
			}
			kRays[d][sq] = ray;
		}
	}

	// Leaper attacks.
	const int kKnightF[8] = { +1, +2, +2, +1, -1, -2, -2, -1 };
	const int kKnightR[8] = { +2, +1, -1, -2, -2, -1, +1, +2 };

	for (int sq = 0; sq < 64; ++sq) {
		const int f = file_of(sq), r = rank_of(sq);

		Bitboard knight = 0;
		for (int i = 0; i < 8; ++i)
			if (on_board(f + kKnightF[i], r + kKnightR[i]))
				knight |= square_bb(make_square(f + kKnightF[i], r + kKnightR[i]));
		kKnightAttacks[sq] = knight;

		Bitboard king = 0;
		for (int df = -1; df <= 1; ++df)
			for (int dr = -1; dr <= 1; ++dr)
				if ((df || dr) && on_board(f + df, r + dr))
					king |= square_bb(make_square(f + df, r + dr));
		kKingAttacks[sq] = king;

		const Bitboard b = square_bb(sq);
		kPawnAttacks[kWhite][sq] = shift_east(shift_north(b)) | shift_west(shift_north(b));
		kPawnAttacks[kBlack][sq] = shift_east(shift_south(b)) | shift_west(shift_south(b));
	}

	// Lines and between-masks for pin and check-block logic.
	for (int a = 0; a < 64; ++a) {
		for (int b = 0; b < 64; ++b) {
			kBetween[a][b] = 0;
			kLine[a][b] = 0;
		}
		for (int d = 0; d < kDirectionNb; ++d) {
			Bitboard ray = kRays[d][a];
			while (ray) {
				const int b = pop_lsb(ray);
				const Direction dir = Direction(d);
				kBetween[a][b] = kRays[dir][a] & kRays[opposite(dir)][b];
				kLine[a][b] = square_bb(a) | kRays[dir][a] | kRays[opposite(dir)][a];
			}
		}
	}

	// Evaluation masks.
	for (int sq = 0; sq < 64; ++sq) {
		const int f = file_of(sq);

		kForwardFile[kWhite][sq] = kRays[kNorth][sq];
		kForwardFile[kBlack][sq] = kRays[kSouth][sq];

		for (int c = 0; c < kColourNb; ++c) {
			Bitboard ahead = 0;
			const int r = rank_of(sq);
			for (int rr = 0; rr < 8; ++rr)
				if (c == kWhite ? rr > r : rr < r) ahead |= kRankBB[rr];
			kForwardRanks[Colour(c)][sq] = ahead;
			kPassedPawnMask[Colour(c)][sq] = ahead & (kFileBB[f] | kAdjacentFiles[f]);
		}

		// King zone: the king's neighbourhood plus one more rank in front of it,
		// which is the region attack-counting cares about.
		for (int c = 0; c < kColourNb; ++c) {
			Bitboard zone = kKingAttacks[sq] | square_bb(sq);
			zone |= (c == kWhite) ? shift_north(zone) : shift_south(zone);
			kKingZone[Colour(c)][sq] = zone;
		}
	}
}

} // namespace eng
