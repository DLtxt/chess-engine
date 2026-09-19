#ifndef ENGINE_MOVEGEN_H
#define ENGINE_MOVEGEN_H

#include "position.h"

namespace eng {

// Fully legal moves. Generation is pseudo-legal followed by a Position::legal()
// filter, which keeps the generator small and the legality rule in one place.
int generate_legal(const Position& pos, Move* list);

// Captures, en passant, and promotions only -- the quiescence search's universe.
int generate_captures(const Position& pos, Move* list);

// Legal moves that are not captures or promotions.
int generate_quiets(const Position& pos, Move* list);

inline bool has_legal_moves(const Position& pos) {
	Move buf[kMaxMoves];
	return generate_legal(pos, buf) > 0;
}

} // namespace eng

#endif
