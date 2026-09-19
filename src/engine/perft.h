#ifndef ENGINE_PERFT_H
#define ENGINE_PERFT_H

#include "position.h"

#include <cstdint>

namespace eng {

// Counts leaf nodes of the legal move tree. This is the standard correctness
// test for a move generator: the numbers are published for known positions, so
// any disagreement localises a bug precisely.
uint64_t perft(Position& pos, int depth);

// Per-root-move breakdown, which narrows a mismatch to a single branch.
void perft_divide(Position& pos, int depth);

// Runs the standard suite and reports pass/fail. Returns true if all matched.
bool perft_suite();

} // namespace eng

#endif
