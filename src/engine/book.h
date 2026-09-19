#ifndef ENGINE_BOOK_H
#define ENGINE_BOOK_H

#include "position.h"

namespace eng {

// A small opening book compiled into the binary, so there is no external file
// to ship or lose. Lines are stored as move sequences and replayed at startup
// into a table of position key -> move, which means the book automatically
// covers transpositions between the lines it knows.
void init_book();

// A book move for this position, or kMoveNone when out of book. Among the
// moves recorded for a position, one is chosen at random weighted by how many
// of the stored lines played it.
Move probe_book(const Position& pos);

void set_book_enabled(bool on);
bool book_enabled();

// Number of distinct positions the book covers.
int book_size();

// Replays every line and reports any that contain an illegal move, which is
// how a typo in the hand-written line table gets caught. Returns the number of
// bad lines.
int book_validate(bool verbose);

} // namespace eng

#endif
