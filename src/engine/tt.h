#ifndef ENGINE_TT_H
#define ENGINE_TT_H

#include "types.h"

#include <cstddef>
#include <vector>

namespace eng {

enum Bound : uint8_t {
	kBoundNone  = 0,
	kBoundUpper = 1,  // score is an upper bound (failed low)
	kBoundLower = 2,  // score is a lower bound (failed high)
	kBoundExact = 3
};

struct TTEntry {
	uint32_t key32;
	Move move;
	int16_t score;
	int16_t eval;
	int8_t depth;
	uint8_t gen_bound;

	Bound bound() const { return Bound(gen_bound & 3); }
	uint8_t generation() const { return gen_bound >> 2; }
};

class TranspositionTable {
public:
	// Buckets of four keep replacement decisions local and cheap.
	static const int kBucketSize = 4;

	void resize(size_t mb);
	void clear();
	void new_search() { generation_ = (generation_ + 1) & 63; }

	// Returns the matching entry, or nullptr. `found` distinguishes a real hit
	// from the empty slot that a store should overwrite.
	const TTEntry* probe(Key key, bool& found) const;
	void store(Key key, Move move, int score, int eval, int depth, Bound bound);

	// Permille of the table used, for UCI "hashfull".
	int hashfull() const;

private:
	size_t index_of(Key key) const { return size_t(key >> 32) % bucket_count_; }

	std::vector<TTEntry> table_;
	size_t bucket_count_ = 0;
	uint8_t generation_ = 0;
};

extern TranspositionTable TT;

// Mate scores are stored relative to the node that found them, so they have to
// be shifted by the current distance from the root on the way in and out.
inline int score_to_tt(int score, int ply) {
	if (score >= kValueMateInMaxPly) return score + ply;
	if (score <= -kValueMateInMaxPly) return score - ply;
	return score;
}

inline int score_from_tt(int score, int ply) {
	if (score >= kValueMateInMaxPly) return score - ply;
	if (score <= -kValueMateInMaxPly) return score + ply;
	return score;
}

} // namespace eng

#endif
