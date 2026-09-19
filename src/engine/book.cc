#include "book.h"

#include "movegen.h"

#include <algorithm>
#include <iostream>
#include <random>
#include <sstream>
#include <vector>

namespace eng {

namespace {

// Main lines of the standard openings, written in the same long algebraic form
// the engine speaks. Every line is replayed and checked at startup, so a typo
// here is reported rather than silently producing an illegal book move.
const char* kLines[] = {
	// --- 1.e4 e5 ---
	"e2e4 e7e5 g1f3 b8c6 f1b5 a7a6 b5a4 g8f6 e1g1 f8e7 f1e1 b7b5 a4b3 d7d6 c2c3 e8g8", // Ruy Lopez
	"e2e4 e7e5 g1f3 b8c6 f1c4 f8c5 c2c3 g8f6 d2d4 e5d4 c3d4 c5b4 c1d2 b4d2 b1d2 d7d5", // Italian
	"e2e4 e7e5 g1f3 b8c6 d2d4 e5d4 f3d4 f8c5 c1e3 d8f6",                               // Scotch
	"e2e4 e7e5 g1f3 b8c6 b1c3 g8f6 f1b5 f8b4",                                         // Four Knights
	"e2e4 e7e5 g1f3 g8f6 f3e5 d7d6 e5f3 f6e4 d2d4 d6d5",                               // Petrov
	"e2e4 e7e5 b1c3 g8f6 f2f4 d7d5 f4e5 f6e4",                                         // Vienna

	// --- 1.e4 c5 ---
	"e2e4 c7c5 g1f3 d7d6 d2d4 c5d4 f3d4 g8f6 b1c3 a7a6",                               // Najdorf
	"e2e4 c7c5 g1f3 d7d6 d2d4 c5d4 f3d4 g8f6 b1c3 g7g6 c1e3 f8g7",                     // Dragon
	"e2e4 c7c5 g1f3 b8c6 d2d4 c5d4 f3d4 g8f6 b1c3 e7e5",                               // Sveshnikov
	"e2e4 c7c5 b1c3 b8c6 g2g3 g7g6 f1g2 f8g7",                                         // Closed Sicilian

	// --- other 1.e4 ---
	"e2e4 e7e6 d2d4 d7d5 b1c3 g8f6 c1g5 f8e7 e4e5 f6d7",                               // French Classical
	"e2e4 e7e6 d2d4 d7d5 b1d2 g8f6 e4e5 f6d7 f1d3 c7c5",                               // French Tarrasch
	"e2e4 c7c6 d2d4 d7d5 b1c3 d5e4 c3e4 c8f5 e4g3 f5g6",                               // Caro-Kann
	"e2e4 d7d5 e4d5 d8d5 b1c3 d5a5 d2d4 g8f6 g1f3 c7c6",                               // Scandinavian
	"e2e4 g8f6 e4e5 f6d5 d2d4 d7d6 g1f3 c8g4",                                         // Alekhine
	"e2e4 d7d6 d2d4 g8f6 b1c3 g7g6 g1f3 f8g7 f1e2 e8g8",                               // Pirc

	// --- 1.d4 d5 ---
	"d2d4 d7d5 c2c4 e7e6 b1c3 g8f6 c1g5 f8e7 e2e3 e8g8",                               // Queen's Gambit Declined
	"d2d4 d7d5 c2c4 d5c4 g1f3 g8f6 e2e3 e7e6 f1c4 c7c5",                               // Queen's Gambit Accepted
	"d2d4 d7d5 c2c4 c7c6 g1f3 g8f6 b1c3 d5c4 a2a4 c8f5",                               // Slav
	"d2d4 d7d5 c1f4 g8f6 e2e3 e7e6 g1f3 f8d6 f4g3 e8g8",                               // London

	// --- 1.d4 Nf6 ---
	"d2d4 g8f6 c2c4 e7e6 b1c3 f8b4 e2e3 e8g8 f1d3 d7d5",                               // Nimzo-Indian
	"d2d4 g8f6 c2c4 e7e6 g1f3 b7b6 g2g3 c8b7 f1g2 f8e7",                               // Queen's Indian
	"d2d4 g8f6 c2c4 g7g6 b1c3 f8g7 e2e4 d7d6 g1f3 e8g8",                               // King's Indian
	"d2d4 g8f6 c2c4 g7g6 b1c3 d7d5 c4d5 f6d5 e2e4 d5c3 b2c3 f8g7",                     // Gruenfeld
	"d2d4 g8f6 c2c4 c7c5 d4d5 e7e6 b1c3 e6d5 c4d5 d7d6",                               // Benoni
	"d2d4 f7f5 g2g3 g8f6 f1g2 e7e6 g1f3 f8e7 e1g1 e8g8",                               // Dutch

	// --- flank openings ---
	"c2c4 e7e5 b1c3 g8f6 g1f3 b8c6 g2g3 d7d5 c4d5 f6d5",                               // English
	"g1f3 d7d5 c2c4 e7e6 g2g3 g8f6 f1g2 f8e7 e1g1 e8g8",                               // Reti
};

struct BookEntry {
	Key key;
	Move move;
	uint16_t weight;
};

std::vector<BookEntry> g_book;
bool g_enabled = true;
std::mt19937 g_rng{20240719u};

// Records a move for a position, or bumps its weight if already known. Weight
// is how many lines play this move here, which doubles as a popularity score.
void add_entry(Key key, Move move) {
	for (BookEntry& e : g_book)
		if (e.key == key && e.move == move) { ++e.weight; return; }
	g_book.push_back({key, move, 1});
}

// Walks one line from the start position. Returns the index of the first
// illegal move, or -1 when the whole line is sound.
int replay_line(const char* line, bool record) {
	Position pos;
	pos.set_start();

	std::istringstream ss(line);
	std::string token;
	int index = 0;

	while (ss >> token) {
		Move moves[kMaxMoves];
		const int n = generate_legal(pos, moves);

		Move found = kMoveNone;
		for (int i = 0; i < n; ++i)
			if (move_to_uci(moves[i]) == token) { found = moves[i]; break; }

		if (found == kMoveNone) return index;

		if (record) add_entry(pos.key(), found);
		pos.do_move(found);
		++index;
	}
	return -1;
}

} // namespace

void init_book() {
	g_book.clear();
	for (const char* line : kLines) {
		// Only record lines that are entirely legal, so one typo cannot inject
		// an illegal move into the book.
		if (replay_line(line, false) == -1) replay_line(line, true);
	}
	std::sort(g_book.begin(), g_book.end(),
	          [](const BookEntry& a, const BookEntry& b) { return a.key < b.key; });
}

int book_validate(bool verbose) {
	int bad = 0;
	for (const char* line : kLines) {
		const int at = replay_line(line, false);
		if (at >= 0) {
			++bad;
			if (verbose) {
				std::istringstream ss(line);
				std::string token;
				for (int i = 0; i <= at && ss >> token; ++i) {}
				std::cout << "BAD LINE  move " << (at + 1) << " (\"" << token
				          << "\") is illegal:\n  " << line << '\n';
			}
		}
	}
	if (verbose)
		std::cout << (bad ? "" : "All book lines are legal. ")
		          << book_size() << " positions covered by " << g_book.size()
		          << " entries." << std::endl;
	return bad;
}

Move probe_book(const Position& pos) {
	if (!g_enabled || g_book.empty()) return kMoveNone;

	// The book is sorted, so all entries for a position sit together.
	auto it = std::lower_bound(g_book.begin(), g_book.end(), pos.key(),
	                           [](const BookEntry& e, Key k) { return e.key < k; });

	int total = 0;
	auto scan = it;
	while (scan != g_book.end() && scan->key == pos.key()) { total += scan->weight; ++scan; }
	if (total == 0) return kMoveNone;

	int pick = std::uniform_int_distribution<int>(1, total)(g_rng);
	for (auto e = it; e != g_book.end() && e->key == pos.key(); ++e) {
		pick -= e->weight;
		// Guard against a stale entry from a key collision.
		if (pick <= 0)
			return (pos.pseudo_legal(e->move) && pos.legal(e->move)) ? e->move : kMoveNone;
	}
	return kMoveNone;
}

void set_book_enabled(bool on) { g_enabled = on; }
bool book_enabled() { return g_enabled; }

int book_size() {
	int distinct = 0;
	for (size_t i = 0; i < g_book.size(); ++i)
		if (i == 0 || g_book[i].key != g_book[i - 1].key) ++distinct;
	return distinct;
}

} // namespace eng
