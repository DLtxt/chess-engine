#include "movegen.h"

namespace eng {

namespace {

// What a generation pass is allowed to emit.
enum GenMode { kGenAll, kGenCaptures, kGenQuiets };

inline void add_promotions(Move*& out, int from, int to, GenMode mode) {
	// Queen and knight are the only promotions that are ever the unique best
	// move, but rook and bishop still have to exist for underpromotion puzzles
	// and for perft to match published node counts.
	*out++ = make_promotion(from, to, kQueen);
	*out++ = make_promotion(from, to, kKnight);
	if (mode != kGenCaptures) {
		*out++ = make_promotion(from, to, kRook);
		*out++ = make_promotion(from, to, kBishop);
	}
}

int generate_pawn_moves(const Position& pos, Move* list, GenMode mode) {
	Move* out = list;

	const Colour us = pos.side_to_move(), them = ~us;
	const int push = pawn_push(us);
	const Bitboard them_bb = pos.pieces(them);
	const int ep = pos.ep_square();

	Bitboard pawns = pos.pieces(us, kPawn);
	while (pawns) {
		const int from = pop_lsb(pawns);
		const bool promoting = relative_rank(us, from) == 6;

		// Captures, including the promotion-capture case.
		if (mode != kGenQuiets) {
			Bitboard att = kPawnAttacks[us][from] & them_bb;
			while (att) {
				const int to = pop_lsb(att);
				if (promoting) add_promotions(out, from, to, mode);
				else *out++ = make_move(from, to);
			}
			if (ep != kNoSquare && (kPawnAttacks[us][from] & square_bb(ep)))
				*out++ = make_move(from, ep, kEnPassant);
		}

		// Pushes. A promotion push counts as a capture-class move because it
		// changes material, so quiescence needs to see it.
		const int to = from + push;
		if (to < 0 || to > 63 || !pos.empty(to)) continue;

		if (promoting) {
			if (mode != kGenQuiets) add_promotions(out, from, to, mode);
			continue;
		}

		if (mode == kGenCaptures) continue;

		*out++ = make_move(from, to);
		if (relative_rank(us, from) == 1 && pos.empty(to + push))
			*out++ = make_move(from, to + push);
	}

	return int(out - list);
}

int generate_piece_moves(const Position& pos, Move* list, GenMode mode) {
	Move* out = list;

	const Colour us = pos.side_to_move();
	const Bitboard occ = pos.pieces();
	const Bitboard target = mode == kGenCaptures ? pos.pieces(~us)
	                      : mode == kGenQuiets   ? ~occ
	                                             : ~pos.pieces(us);

	for (int pt = kKnight; pt <= kKing; ++pt) {
		Bitboard from_bb = pos.pieces(us, PieceType(pt));
		while (from_bb) {
			const int from = pop_lsb(from_bb);
			Bitboard att = attacks_from(PieceType(pt), from, occ) & target;
			while (att) *out++ = make_move(from, pop_lsb(att));
		}
	}

	if (mode != kGenCaptures) {
		const int ksq = us == kWhite ? 4 : 60;
		if (pos.can_castle(us, true))  *out++ = make_move(ksq, ksq + 2, kCastling);
		if (pos.can_castle(us, false)) *out++ = make_move(ksq, ksq - 2, kCastling);
	}

	return int(out - list);
}

int generate_filtered(const Position& pos, Move* list, GenMode mode) {
	Move pseudo[kMaxMoves];
	int n = generate_pawn_moves(pos, pseudo, mode);
	n += generate_piece_moves(pos, pseudo + n, mode);

	int count = 0;
	for (int i = 0; i < n; ++i)
		if (pos.legal(pseudo[i])) list[count++] = pseudo[i];
	return count;
}

} // namespace

int generate_legal(const Position& pos, Move* list) {
	return generate_filtered(pos, list, kGenAll);
}

int generate_captures(const Position& pos, Move* list) {
	return generate_filtered(pos, list, kGenCaptures);
}

int generate_quiets(const Position& pos, Move* list) {
	return generate_filtered(pos, list, kGenQuiets);
}

} // namespace eng
