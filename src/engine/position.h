#ifndef ENGINE_POSITION_H
#define ENGINE_POSITION_H

#include "bitboard.h"
#include "zobrist.h"

#include <string>
#include <vector>

namespace eng {

// Everything do_move() cannot recompute cheaply, saved so undo_move() can
// restore it exactly.
struct StateInfo {
	Key key;
	Key pawn_key;
	int castling;
	int ep;
	int halfmove;
	Piece captured;
	Bitboard checkers;
};

extern const int kSeeValue[kPieceTypeNb];

class Position {
public:
	Position() { set_start(); }

	void set_start();
	bool set_fen(const std::string& fen);
	std::string fen() const;

	// --- board queries -----------------------------------------------------

	Bitboard pieces() const { return by_colour_[kWhite] | by_colour_[kBlack]; }
	Bitboard pieces(Colour c) const { return by_colour_[c]; }
	Bitboard pieces(PieceType pt) const { return by_type_[pt]; }
	Bitboard pieces(Colour c, PieceType pt) const { return by_colour_[c] & by_type_[pt]; }
	Bitboard pieces(Colour c, PieceType a, PieceType b) const {
		return by_colour_[c] & (by_type_[a] | by_type_[b]);
	}

	Piece piece_on(int sq) const { return board_[sq]; }
	bool empty(int sq) const { return board_[sq] == kNoPiece; }
	int king_sq(Colour c) const { return lsb(pieces(c, kKing)); }
	int count(Colour c, PieceType pt) const { return popcount(pieces(c, pt)); }

	Colour side_to_move() const { return stm_; }
	int ep_square() const { return ep_; }
	int castling_rights() const { return castling_; }
	int halfmove_clock() const { return halfmove_; }
	int game_ply() const { return ply_; }
	Key key() const { return key_; }
	Key pawn_key() const { return pawn_key_; }

	// --- attacks and legality ----------------------------------------------

	Bitboard attackers_to(int sq, Bitboard occ) const;
	Bitboard attackers_to(int sq) const { return attackers_to(sq, pieces()); }

	Bitboard checkers() const { return checkers_; }
	bool in_check() const { return checkers_ != 0; }

	// Is this pseudo-legal move actually legal (does it leave our king safe)?
	bool legal(Move m) const;

	// Rights present, path clear, and the king neither starts, crosses nor
	// lands on an attacked square.
	bool can_castle(Colour c, bool kingside) const;

	// Does this move exist in the current position? Used to validate a move
	// recovered from the transposition table before playing it.
	bool pseudo_legal(Move m) const;

	bool is_capture(Move m) const {
		return (!empty(to_sq(m)) && move_type(m) != kCastling) || move_type(m) == kEnPassant;
	}

	// --- making moves ------------------------------------------------------

	void do_move(Move m);
	void undo_move(Move m);
	void do_null_move();
	void undo_null_move();

	// --- draw detection ----------------------------------------------------

	bool is_repetition() const;
	bool is_fifty_move() const { return halfmove_ >= 100; }
	bool is_insufficient_material() const;
	bool is_draw() const {
		return is_fifty_move() || is_repetition() || is_insufficient_material();
	}

	// --- evaluation helpers ------------------------------------------------

	int non_pawn_material(Colour c) const;

	// Static exchange evaluation: material won or lost by the capture sequence
	// starting with this move, assuming both sides always recapture with their
	// least valuable attacker.
	int see(Move m) const;
	bool see_ge(Move m, int threshold) const { return see(m) >= threshold; }

	std::string to_string() const;

private:
	void put_piece(Piece pc, int sq);
	void remove_piece(int sq);
	void move_piece(int from, int to);
	void set_check_info();
	void clear();

	Bitboard by_type_[kPieceTypeNb];
	Bitboard by_colour_[kColourNb];
	Piece board_[64];

	Colour stm_;
	int castling_;
	int ep_;
	int halfmove_;
	int ply_;

	Key key_;
	Key pawn_key_;
	Bitboard checkers_;

	std::vector<StateInfo> st_;
};

} // namespace eng

#endif
