#ifndef ENGINE_EVAL_H
#define ENGINE_EVAL_H

#include "position.h"

namespace eng {

void init_eval();

// Static evaluation in centipawns, from the side to move's point of view.
int evaluate(const Position& pos);

// Clears the pawn-structure cache. Call between games.
void clear_eval_caches();

} // namespace eng

#endif
