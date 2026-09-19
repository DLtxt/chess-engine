#ifndef ENGINE_BITBOARD_H
#define ENGINE_BITBOARD_H

#include "types.h"

namespace eng {

// ------------------------------------------------------------- primitives ---

constexpr Bitboard square_bb(int sq) { return Bitboard(1) << sq; }

constexpr Bitboard kFileABB = 0x0101010101010101ULL;
constexpr Bitboard kFileHBB = kFileABB << 7;
constexpr Bitboard kRank1BB = 0xFFULL;

inline int popcount(Bitboard b) { return __builtin_popcountll(b); }
inline int lsb(Bitboard b) { return __builtin_ctzll(b); }
inline int msb(Bitboard b) { return 63 ^ __builtin_clzll(b); }

inline int pop_lsb(Bitboard& b) {
	const int s = lsb(b);
	b &= b - 1;
	return s;
}

inline bool more_than_one(Bitboard b) { return b & (b - 1); }

// Shifts that drop anything wrapping around a file edge.
constexpr Bitboard shift_north(Bitboard b) { return b << 8; }
constexpr Bitboard shift_south(Bitboard b) { return b >> 8; }
constexpr Bitboard shift_east(Bitboard b)  { return (b & ~kFileHBB) << 1; }
constexpr Bitboard shift_west(Bitboard b)  { return (b & ~kFileABB) >> 1; }

// ------------------------------------------------------------------ rays ---

enum Direction : int {
	kNorth = 0, kNorthEast, kEast, kSouthEast,
	kSouth, kSouthWest, kWest, kNorthWest, kDirectionNb
};

// --------------------------------------------------------------- tables ----

extern Bitboard kFileBB[8];
extern Bitboard kRankBB[8];
extern Bitboard kAdjacentFiles[8];

extern Bitboard kRays[kDirectionNb][64];
extern Bitboard kPawnAttacks[kColourNb][64];
extern Bitboard kKnightAttacks[64];
extern Bitboard kKingAttacks[64];

// Squares strictly between two aligned squares; empty if not aligned.
extern Bitboard kBetween[64][64];
// The full line through two aligned squares; empty if not aligned.
extern Bitboard kLine[64][64];

// Everything ahead of a square on its own file, from that side's point of view.
extern Bitboard kForwardFile[kColourNb][64];
// The three files ahead of a pawn: if this is empty of enemy pawns it is passed.
extern Bitboard kPassedPawnMask[kColourNb][64];
// Ranks strictly ahead of a square, used for backward-pawn detection.
extern Bitboard kForwardRanks[kColourNb][64];
// The 3x3 (edge-clamped) neighbourhood of a king, plus the rank it shields.
extern Bitboard kKingZone[kColourNb][64];

extern int kSquareDistance[64][64];

void init_bitboards();

// ------------------------------------------------------ sliding attacks ----

// Classical ray attacks: take the whole ray, and if it hits a blocker, mask off
// everything beyond that blocker. Positive directions scan from the low bit.
inline Bitboard ray_attacks(Direction d, int sq, Bitboard occ) {
	Bitboard attacks = kRays[d][sq];
	Bitboard blockers = attacks & occ;
	if (blockers) {
		const bool positive = (d == kNorth || d == kNorthEast || d == kEast || d == kNorthWest);
		attacks ^= kRays[d][positive ? lsb(blockers) : msb(blockers)];
	}
	return attacks;
}

inline Bitboard bishop_attacks(int sq, Bitboard occ) {
	return ray_attacks(kNorthEast, sq, occ) | ray_attacks(kSouthEast, sq, occ)
	     | ray_attacks(kSouthWest, sq, occ) | ray_attacks(kNorthWest, sq, occ);
}

inline Bitboard rook_attacks(int sq, Bitboard occ) {
	return ray_attacks(kNorth, sq, occ) | ray_attacks(kEast, sq, occ)
	     | ray_attacks(kSouth, sq, occ) | ray_attacks(kWest, sq, occ);
}

inline Bitboard queen_attacks(int sq, Bitboard occ) {
	return bishop_attacks(sq, occ) | rook_attacks(sq, occ);
}

inline Bitboard attacks_from(PieceType pt, int sq, Bitboard occ) {
	switch (pt) {
		case kKnight: return kKnightAttacks[sq];
		case kBishop: return bishop_attacks(sq, occ);
		case kRook:   return rook_attacks(sq, occ);
		case kQueen:  return queen_attacks(sq, occ);
		case kKing:   return kKingAttacks[sq];
		default:      return 0;
	}
}

// True when three squares share a rank, file or diagonal.
inline bool aligned(int a, int b, int c) {
	return kLine[a][b] & square_bb(c);
}

} // namespace eng

#endif
