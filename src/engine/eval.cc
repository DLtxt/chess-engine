#include "eval.h"

#include <algorithm>
#include <cstring>

namespace eng {

namespace {

// ------------------------------------------------------------- material ---

// Two sets of values, blended by how much material is left on the board. A
// rook is worth more in an endgame; a knight is worth less.
const int kMgValue[kPieceTypeNb] = {  82, 337, 365, 477, 1025, 0 };
const int kEgValue[kPieceTypeNb] = {  94, 281, 297, 512,  936, 0 };

// How much each piece contributes to "this is still a middlegame".
const int kPhaseInc[kPieceTypeNb] = { 0, 1, 1, 2, 4, 0 };
const int kPhaseMax = 24;

// -------------------------------------------------- piece-square tables ---

// Written from Black's side of the board so that they read like a diagram:
// index 0 is a8. White reads them flipped (sq ^ 56).

const int kMgPawn[64] = {
	  0,   0,   0,   0,   0,   0,   0,   0,
	 98, 134,  61,  95,  68, 126,  34, -11,
	 -6,   7,  26,  31,  65,  56,  25, -20,
	-14,  13,   6,  21,  23,  12,  17, -23,
	-27,  -2,  -5,  12,  17,   6,  10, -25,
	-26,  -4,  -4, -10,   3,   3,  33, -12,
	-35,  -1, -20, -23, -15,  24,  38, -22,
	  0,   0,   0,   0,   0,   0,   0,   0,
};
const int kEgPawn[64] = {
	  0,   0,   0,   0,   0,   0,   0,   0,
	178, 173, 158, 134, 147, 132, 165, 187,
	 94, 100,  85,  67,  56,  53,  82,  84,
	 32,  24,  13,   5,  -2,   4,  17,  17,
	 13,   9,  -3,  -7,  -7,  -8,   3,  -1,
	  4,   7,  -6,   1,   0,  -5,  -1,  -8,
	 13,   8,   8,  10,  13,   0,   2,  -7,
	  0,   0,   0,   0,   0,   0,   0,   0,
};
const int kMgKnight[64] = {
	-167, -89, -34, -49,  61, -97, -15, -107,
	 -73, -41,  72,  36,  23,  62,   7,  -17,
	 -47,  60,  37,  65,  84, 129,  73,   44,
	  -9,  17,  19,  53,  37,  69,  18,   22,
	 -13,   4,  16,  13,  28,  19,  21,   -8,
	 -23,  -9,  12,  10,  19,  17,  25,  -16,
	 -29, -53, -12,  -3,  -1,  18, -14,  -19,
	-105, -21, -58, -33, -17, -28, -19,  -23,
};
const int kEgKnight[64] = {
	-58, -38, -13, -28, -31, -27, -63, -99,
	-25,  -8, -25,  -2,  -9, -25, -24, -52,
	-24, -20,  10,   9,  -1,  -9, -19, -41,
	-17,   3,  22,  22,  22,  11,   8, -18,
	-18,  -6,  16,  25,  16,  17,   4, -18,
	-23,  -3,  -1,  15,  10,  -3, -20, -22,
	-42, -20, -10,  -5,  -2, -20, -23, -44,
	-29, -51, -23, -15, -22, -18, -50, -64,
};
const int kMgBishop[64] = {
	-29,   4, -82, -37, -25, -42,   7,  -8,
	-26,  16, -18, -13,  30,  59,  18, -47,
	-16,  37,  43,  40,  35,  50,  37,  -2,
	 -4,   5,  19,  50,  37,  37,   7,  -2,
	 -6,  13,  13,  26,  34,  12,  10,   4,
	  0,  15,  15,  15,  14,  27,  18,  10,
	  4,  15,  16,   0,   7,  21,  33,   1,
	-33,  -3, -14, -21, -13, -12, -39, -21,
};
const int kEgBishop[64] = {
	-14, -21, -11,  -8,  -7,  -9, -17, -24,
	 -8,  -4,   7, -12,  -3, -13,  -4, -14,
	  2,  -8,   0,  -1,  -2,   6,   0,   4,
	 -3,   9,  12,   9,  14,  10,   3,   2,
	 -6,   3,  13,  19,   7,  10,  -3,  -9,
	-12,  -3,   8,  10,  13,   3,  -7, -15,
	-14, -18,  -7,  -1,   4,  -9, -15, -27,
	-23,  -9, -23,  -5,  -9, -16,  -5, -17,
};
const int kMgRook[64] = {
	 32,  42,  32,  51,  63,   9,  31,  43,
	 27,  32,  58,  62,  80,  67,  26,  44,
	 -5,  19,  26,  36,  17,  45,  61,  16,
	-24, -11,   7,  26,  24,  35,  -8, -20,
	-36, -26, -12,  -1,   9,  -7,   6, -23,
	-45, -25, -16, -17,   3,   0,  -5, -33,
	-44, -16, -20,  -9,  -1,  11,  -6, -71,
	-19, -13,   1,  17,  16,   7, -37, -26,
};
const int kEgRook[64] = {
	13, 10, 18, 15, 12,  12,   8,   5,
	11, 13, 13, 11, -3,   3,   8,   3,
	 7,  7,  7,  5,  4,  -3,  -5,  -3,
	 4,  3, 13,  1,  2,   1,  -1,   2,
	 3,  5,  8,  4, -5,  -6,  -8, -11,
	-4,  0, -5, -1, -7, -12,  -8, -16,
	-6, -6,  0,  2, -9,  -9, -11,  -3,
	-9,  2,  3, -1, -5, -13,   4, -20,
};
const int kMgQueen[64] = {
	-28,   0,  29,  12,  59,  44,  43,  45,
	-24, -39,  -5,   1, -16,  57,  28,  54,
	-13, -17,   7,   8,  29,  56,  47,  57,
	-27, -27, -16, -16,  -1,  17,  -2,   1,
	 -9, -26,  -9, -10,  -2,  -4,   3,  -3,
	-14,   2, -11,  -2,  -5,   2,  14,   5,
	-35,  -8,  11,   2,   8,  15,  -3,   1,
	 -1, -18,  -9,  10, -15, -25, -31, -50,
};
const int kEgQueen[64] = {
	 -9,  22,  22,  27,  27,  19,  10,  20,
	-17,  20,  32,  41,  58,  25,  30,   0,
	-20,   6,   9,  49,  47,  35,  19,   9,
	  3,  22,  24,  45,  57,  40,  57,  36,
	-18,  28,  19,  47,  31,  34,  39,  23,
	-16, -27,  15,   6,   9,  17,  10,   5,
	-22, -23, -30, -16, -16, -23, -36, -32,
	-33, -28, -22, -43,  -5, -32, -20, -41,
};
const int kMgKing[64] = {
	-65,  23,  16, -15, -56, -34,   2,  13,
	 29,  -1, -20,  -7,  -8,  -4, -38, -29,
	 -9,  24,   2, -16, -20,   6,  22, -22,
	-17, -20, -12, -27, -30, -25, -14, -36,
	-49,  -1, -27, -39, -46, -44, -33, -51,
	-14, -14, -22, -46, -44, -30, -15, -27,
	  1,   7,  -8, -64, -43, -16,   9,   8,
	-15,  36,  12, -54,   8, -28,  24,  14,
};
const int kEgKing[64] = {
	-74, -35, -18, -18, -11,  15,   4, -17,
	-12,  17,  14,  17,  17,  38,  23,  11,
	 10,  17,  23,  15,  20,  45,  44,  13,
	 -8,  22,  24,  27,  26,  33,  26,   3,
	-18,  -4,  21,  24,  27,  23,   9, -11,
	-19,  -3,  11,  21,  23,  16,   7,  -9,
	-27, -11,   4,  13,  14,   4,  -5, -17,
	-53, -34, -21, -11, -28, -14, -24, -43,
};

const int* kMgTables[kPieceTypeNb] = { kMgPawn, kMgKnight, kMgBishop, kMgRook, kMgQueen, kMgKing };
const int* kEgTables[kPieceTypeNb] = { kEgPawn, kEgKnight, kEgBishop, kEgRook, kEgQueen, kEgKing };

// Fully resolved [piece][square] tables, material folded in.
int mg_psq[kPieceNb][64];
int eg_psq[kPieceNb][64];

// ----------------------------------------------------- structural terms ---

// Passed pawns are scored by how far they have advanced.
const int kPassedMg[8] = { 0,  5, 12, 20,  38,  68, 120, 0 };
const int kPassedEg[8] = { 0, 12, 22, 38,  68, 110, 180, 0 };

const int kIsolatedMg = -14, kIsolatedEg = -18;
const int kDoubledMg  = -10, kDoubledEg  = -22;
const int kBackwardMg =  -9, kBackwardEg = -12;
const int kConnectedMg =  8, kConnectedEg = 6;

const int kBishopPairMg = 28, kBishopPairEg = 48;
const int kRookOpenMg = 26, kRookOpenEg = 12;
const int kRookSemiOpenMg = 12, kRookSemiOpenEg = 6;
const int kRookOnSeventhMg = 18, kRookOnSeventhEg = 32;

// Mobility bonus per reachable square, indexed by piece type. Sliding pieces
// gain more from space than knights do.
const int kMobilityMg[kPieceTypeNb] = { 0, 4, 5, 3, 2, 0 };
const int kMobilityEg[kPieceTypeNb] = { 0, 4, 5, 5, 4, 0 };

// Weight applied per attacker in the enemy king's zone.
const int kKingAttackWeight[kPieceTypeNb] = { 0, 20, 20, 40, 80, 0 };
// Scales the raw attack count into a penalty; index is the attack units, capped.
const int kKingSafetyScale[8] = { 0, 0, 12, 30, 56, 88, 120, 150 };
const int kPawnShieldMg = 14;

// ---------------------------------------------------- pawn structure cache ---

struct PawnEntry {
	Key key;
	int mg;
	int eg;
	Bitboard passed[kColourNb];
};

const size_t kPawnTableSize = 1 << 14;
PawnEntry pawn_table[kPawnTableSize];

struct Trace {
	int mg = 0;
	int eg = 0;
	void add(int m, int e) { mg += m; eg += e; }
};

// --------------------------------------------------------------- helpers ---

// Everything about pawn skeletons, cached because it changes rarely.
const PawnEntry& probe_pawns(const Position& pos) {
	PawnEntry& e = pawn_table[pos.pawn_key() & (kPawnTableSize - 1)];
	if (e.key == pos.pawn_key()) return e;

	e.key = pos.pawn_key();
	e.mg = e.eg = 0;
	e.passed[kWhite] = e.passed[kBlack] = 0;

	for (int c = 0; c < kColourNb; ++c) {
		const Colour us = Colour(c), them = ~us;
		const int sign = us == kWhite ? 1 : -1;
		const Bitboard ours = pos.pieces(us, kPawn);
		const Bitboard theirs = pos.pieces(them, kPawn);

		Bitboard b = ours;
		while (b) {
			const int sq = pop_lsb(b);
			const int f = file_of(sq);
			const int rr = relative_rank(us, sq);

			const bool doubled = (kForwardFile[us][sq] & ours) != 0;
			const bool isolated = (kAdjacentFiles[f] & ours) == 0;
			const bool passed = (kPassedPawnMask[us][sq] & theirs) == 0 && !doubled;
			// Backward: no friendly pawn beside or behind to support it, and the
			// square in front is covered by an enemy pawn.
			const bool supported_behind =
				(kAdjacentFiles[f] & ours & ~kForwardRanks[us][sq]) != 0;
			const bool stop_attacked =
				(kPawnAttacks[us][sq] & theirs) != 0;
			const bool backward = !passed && !supported_behind && stop_attacked;
			const bool connected =
				(kAdjacentFiles[f] & ours & (kRankBB[rank_of(sq)])) != 0
				|| (kPawnAttacks[them][sq] & ours) != 0;

			if (doubled)  { e.mg += sign * kDoubledMg;  e.eg += sign * kDoubledEg; }
			if (isolated) { e.mg += sign * kIsolatedMg; e.eg += sign * kIsolatedEg; }
			if (backward) { e.mg += sign * kBackwardMg; e.eg += sign * kBackwardEg; }
			if (connected){ e.mg += sign * kConnectedMg;e.eg += sign * kConnectedEg; }
			if (passed) {
				e.passed[us] |= square_bb(sq);
				e.mg += sign * kPassedMg[rr];
				e.eg += sign * kPassedEg[rr];
			}
		}
	}
	return e;
}

// Mobility, king attacks and rook placement for one side.
void evaluate_pieces(const Position& pos, Colour us, Trace& t, int& king_attack_units) {
	const Colour them = ~us;
	const int sign = us == kWhite ? 1 : -1;
	const Bitboard occ = pos.pieces();
	const Bitboard own = pos.pieces(us);

	// Squares controlled by enemy pawns are not real mobility.
	Bitboard their_pawn_attacks = 0;
	Bitboard tp = pos.pieces(them, kPawn);
	while (tp) their_pawn_attacks |= kPawnAttacks[them][pop_lsb(tp)];

	const Bitboard mobility_area = ~own & ~their_pawn_attacks;
	const Bitboard king_zone = kKingZone[them][pos.king_sq(them)];

	king_attack_units = 0;

	for (int pt = kKnight; pt <= kQueen; ++pt) {
		Bitboard b = pos.pieces(us, PieceType(pt));
		while (b) {
			const int sq = pop_lsb(b);
			const Bitboard att = attacks_from(PieceType(pt), sq, occ);

			const int moves = popcount(att & mobility_area);
			t.add(sign * moves * kMobilityMg[pt], sign * moves * kMobilityEg[pt]);

			if (att & king_zone)
				king_attack_units += kKingAttackWeight[pt] * popcount(att & king_zone) / 10;

			if (pt == kRook) {
				const Bitboard file_bb = kFileBB[file_of(sq)];
				const bool own_pawn = (file_bb & pos.pieces(us, kPawn)) != 0;
				const bool their_pawn = (file_bb & pos.pieces(them, kPawn)) != 0;
				if (!own_pawn && !their_pawn)
					t.add(sign * kRookOpenMg, sign * kRookOpenEg);
				else if (!own_pawn)
					t.add(sign * kRookSemiOpenMg, sign * kRookSemiOpenEg);
				if (relative_rank(us, sq) == 6)
					t.add(sign * kRookOnSeventhMg, sign * kRookOnSeventhEg);
			}
		}
	}

	if (popcount(pos.pieces(us, kBishop)) >= 2)
		t.add(sign * kBishopPairMg, sign * kBishopPairEg);
}

// Pawn cover in front of the king, and the penalty for enemy pieces swarming it.
void evaluate_king_safety(const Position& pos, Colour us, int attack_units, Trace& t) {
	const int sign = us == kWhite ? 1 : -1;
	const int ksq = pos.king_sq(us);

	// Our own shield: pawns standing in the king's zone.
	const Bitboard shield = kKingZone[us][ksq] & pos.pieces(us, kPawn);
	t.add(sign * kPawnShieldMg * popcount(shield), 0);

	// attack_units was accumulated by the opponent against this king.
	const int idx = std::min(attack_units, 7);
	t.add(-sign * kKingSafetyScale[idx], 0);
}

} // namespace

void init_eval() {
	for (int c = 0; c < kColourNb; ++c) {
		for (int pt = 0; pt < kPieceTypeNb; ++pt) {
			const Piece pc = make_piece(Colour(c), PieceType(pt));
			for (int sq = 0; sq < 64; ++sq) {
				// White reads the tables flipped; Black reads them as written.
				const int idx = (c == kWhite) ? (sq ^ 56) : sq;
				mg_psq[pc][sq] = kMgValue[pt] + kMgTables[pt][idx];
				eg_psq[pc][sq] = kEgValue[pt] + kEgTables[pt][idx];
			}
		}
	}
	clear_eval_caches();
}

void clear_eval_caches() {
	std::memset(pawn_table, 0, sizeof(pawn_table));
}

int evaluate(const Position& pos) {
	Trace t;
	int phase = 0;

	// Material and piece-square scores in one sweep.
	for (int c = 0; c < kColourNb; ++c) {
		const int sign = c == kWhite ? 1 : -1;
		for (int pt = 0; pt < kPieceTypeNb; ++pt) {
			const Piece pc = make_piece(Colour(c), PieceType(pt));
			Bitboard b = pos.pieces(Colour(c), PieceType(pt));
			while (b) {
				const int sq = pop_lsb(b);
				t.add(sign * mg_psq[pc][sq], sign * eg_psq[pc][sq]);
				phase += kPhaseInc[pt];
			}
		}
	}

	const PawnEntry& pe = probe_pawns(pos);
	t.add(pe.mg, pe.eg);

	int white_attacks = 0, black_attacks = 0;
	evaluate_pieces(pos, kWhite, t, white_attacks);
	evaluate_pieces(pos, kBlack, t, black_attacks);

	// White's attack units are pressure on the black king, and vice versa.
	evaluate_king_safety(pos, kWhite, black_attacks, t);
	evaluate_king_safety(pos, kBlack, white_attacks, t);

	// Blend the middlegame and endgame scores by how much material remains.
	const int mg_phase = std::min(phase, kPhaseMax);
	const int eg_phase = kPhaseMax - mg_phase;
	int score = (t.mg * mg_phase + t.eg * eg_phase) / kPhaseMax;

	// A small bonus for having the move keeps the engine from drifting.
	score += (pos.side_to_move() == kWhite) ? 10 : -10;

	return pos.side_to_move() == kWhite ? score : -score;
}

} // namespace eng
