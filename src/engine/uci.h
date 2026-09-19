#ifndef ENGINE_UCI_H
#define ENGINE_UCI_H

namespace eng {

// Reads UCI commands from stdin until "quit". Also accepts a few non-standard
// helpers: "d" prints the board, "perft N" and "perfttest" run the move
// generator tests, and "bench" reports a fixed-workload node count.
void uci_loop();

// Fixed-depth search over a set of positions: a reproducible speed and
// correctness check to compare builds against each other.
void bench(int depth);

} // namespace eng

#endif
