#ifndef ENGINE_ENGINE_H
#define ENGINE_ENGINE_H

#include <cstddef>

namespace eng {

// Builds every lookup table and sizes the transposition table. Must be called
// once before any Position, search or evaluation is used.
void init_engine(size_t hash_mb = 64);

} // namespace eng

#endif
