#ifndef ENGINE_TYPES_H
#define ENGINE_TYPES_H

// Core value types for the search engine.
//
// Everything lives in namespace eng and uses k-prefixed enumerators so that it
// never collides with the preprocessor macros (WHITE, BLACK, KING, PAWN, ...)
// that the legacy game code defines in pieces/piece.h.

#include <cstdint>
#include <string>

namespace eng {

using Bitboard = uint64_t;
using Key = uint64_t;

// ---------------------------------------------------------------- colours ---

enum Colour : int { kWhite = 0, kBlack = 1, kColourNb = 2 };

constexpr Colour operator~(Colour c) { return Colour(c ^ 1); }

// ----------------------------------------------------------------- pieces ---

enum PieceType : int {
	kPawn = 0, kKnight, kBishop, kRook, kQueen, kKing,
	kPieceTypeNb = 6, kNoPieceType = 6
};

// Piece = colour * 6 + type, so white pieces are 0..5 and black are 6..11.
enum Piece : int { kNoPiece = 12, kPieceNb = 13 };

constexpr Piece make_piece(Colour c, PieceType pt) { return Piece(int(c) * 6 + int(pt)); }
constexpr PieceType type_of(Piece p) { return PieceType(int(p) % 6); }
constexpr Colour colour_of(Piece p) { return Colour(int(p) / 6); }

// ---------------------------------------------------------------- squares ---

// A1 = 0, B1 = 1, ... H1 = 7, A2 = 8, ... H8 = 63.
enum Square : int { kSqA1 = 0, kSqH8 = 63, kSquareNb = 64, kNoSquare = 64 };

constexpr int make_square(int file, int rank) { return rank * 8 + file; }
constexpr int file_of(int sq) { return sq & 7; }
constexpr int rank_of(int sq) { return sq >> 3; }

// Rank as seen from a given side: relative_rank(kBlack, 55) == 1.
constexpr int relative_rank(Colour c, int sq) {
	return c == kWhite ? rank_of(sq) : 7 - rank_of(sq);
}
constexpr int relative_square(Colour c, int sq) {
	return c == kWhite ? sq : sq ^ 56;
}

// Direction a pawn of the given colour advances, in square deltas.
constexpr int pawn_push(Colour c) { return c == kWhite ? 8 : -8; }

std::string square_to_string(int sq);
int square_from_string(const std::string& s);

// ------------------------------------------------------------------ moves ---

// A move is packed into 16 bits:
//   bits  0-5   origin square
//   bits  6-11  destination square
//   bits 12-13  promotion piece, encoded as PieceType - kKnight
//   bits 14-15  move type (see MoveType below)
using Move = uint16_t;

enum MoveType : int {
	kNormal = 0,
	kPromotion = 1,
	kEnPassant = 2,
	kCastling = 3
};

constexpr Move kMoveNone = 0;

constexpr int from_sq(Move m) { return m & 0x3F; }
constexpr int to_sq(Move m) { return (m >> 6) & 0x3F; }
constexpr MoveType move_type(Move m) { return MoveType((m >> 14) & 3); }
constexpr PieceType promotion_type(Move m) { return PieceType(((m >> 12) & 3) + kKnight); }

constexpr Move make_move(int from, int to) {
	return Move(from | (to << 6));
}
constexpr Move make_move(int from, int to, MoveType t) {
	return Move(from | (to << 6) | (int(t) << 14));
}
constexpr Move make_promotion(int from, int to, PieceType pt) {
	return Move(from | (to << 6) | ((int(pt) - kKnight) << 12) | (int(kPromotion) << 14));
}

// Long algebraic form used by UCI, e.g. "e2e4", "e7e8q".
std::string move_to_uci(Move m);

// --------------------------------------------------------------- castling ---

enum CastlingRight : int {
	kNoCastling = 0,
	kWhiteOO  = 1,
	kWhiteOOO = 2,
	kBlackOO  = 4,
	kBlackOOO = 8,
	kAnyCastling = 15
};

// ----------------------------------------------------------------- scores ---

// Scores are in centipawns. kValueMate is kept well clear of the 16-bit
// transposition-table score field and of any plausible evaluation total.
constexpr int kValueDraw     = 0;
constexpr int kValueMate     = 32000;
constexpr int kValueInfinite = 32001;
constexpr int kValueNone     = 32002;

// A mate score seen from the root: mate in n plies scores kValueMate - n.
constexpr int kValueMateInMaxPly = kValueMate - 256;

constexpr bool is_mate_score(int v) {
	return v >= kValueMateInMaxPly || v <= -kValueMateInMaxPly;
}

constexpr int kMaxPly = 128;
constexpr int kMaxMoves = 256;

} // namespace eng

#endif
