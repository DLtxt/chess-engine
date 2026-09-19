#include "tt.h"

#include <algorithm>
#include <cstring>

namespace eng {

TranspositionTable TT;

void TranspositionTable::resize(size_t mb) {
	if (mb < 1) mb = 1;
	const size_t bytes = mb * 1024 * 1024;
	size_t buckets = bytes / (sizeof(TTEntry) * kBucketSize);
	if (buckets < 1) buckets = 1;

	bucket_count_ = buckets;
	table_.assign(bucket_count_ * kBucketSize, TTEntry{});
	generation_ = 0;
}

void TranspositionTable::clear() {
	std::fill(table_.begin(), table_.end(), TTEntry{});
	generation_ = 0;
}

const TTEntry* TranspositionTable::probe(Key key, bool& found) const {
	if (table_.empty()) { found = false; return nullptr; }

	const TTEntry* bucket = &table_[index_of(key) * kBucketSize];
	const uint32_t k32 = uint32_t(key);

	for (int i = 0; i < kBucketSize; ++i) {
		if (bucket[i].key32 == k32 && bucket[i].bound() != kBoundNone) {
			found = true;
			return &bucket[i];
		}
	}
	found = false;
	return nullptr;
}

void TranspositionTable::store(Key key, Move move, int score, int eval, int depth, Bound bound) {
	if (table_.empty()) return;

	TTEntry* bucket = &table_[index_of(key) * kBucketSize];
	const uint32_t k32 = uint32_t(key);

	TTEntry* replace = bucket;
	for (int i = 0; i < kBucketSize; ++i) {
		// Same position: always refresh it in place.
		if (bucket[i].key32 == k32 || bucket[i].bound() == kBoundNone) {
			replace = &bucket[i];
			break;
		}
		// Otherwise prefer to evict shallow entries and stale generations.
		const int cur_value = bucket[i].depth - ((64 + generation_ - bucket[i].generation()) & 63) * 2;
		const int best_value = replace->depth - ((64 + generation_ - replace->generation()) & 63) * 2;
		if (cur_value < best_value) replace = &bucket[i];
	}

	// Keep an existing deeper entry's move if this store has none to offer.
	if (move == kMoveNone && replace->key32 == k32) move = replace->move;

	// Do not let a shallow non-exact result clobber a deep exact one.
	if (replace->key32 == k32 && bound != kBoundExact && depth < replace->depth - 3) return;

	replace->key32 = k32;
	replace->move = move;
	replace->score = int16_t(score);
	replace->eval = int16_t(eval);
	replace->depth = int8_t(depth);
	replace->gen_bound = uint8_t((generation_ << 2) | bound);
}

int TranspositionTable::hashfull() const {
	if (table_.empty()) return 0;
	const int sample = int(std::min<size_t>(table_.size(), 1000));
	int used = 0;
	for (int i = 0; i < sample; ++i)
		if (table_[i].bound() != kBoundNone) ++used;
	return used * 1000 / sample;
}

} // namespace eng
