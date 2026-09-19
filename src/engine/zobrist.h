#ifndef ENGINE_ZOBRIST_H
#define ENGINE_ZOBRIST_H

#include "types.h"

namespace eng {
namespace zobrist {

extern Key psq[kPieceNb][64];   // piece on square
extern Key side;                 // xored in when black is to move
extern Key castling[16];         // indexed by the castling-rights mask
extern Key ep_file[8];           // xored in only when a capture is available

void init();

} // namespace zobrist
} // namespace eng

#endif
