#include "computer_player.h"
#include "parser.h"
#include "board.h"
#include <random>
#include <string>
#include <utility>
#include <vector>
#include <sstream>

// Seeding on every call meant that two calls in the same second returned the
// same value, which correlated the piece choice with the move choice.
int rand_n(int n) {
	static std::mt19937 rng{std::random_device{}()};
	if (n <= 0) return 0;
	return std::uniform_int_distribution<int>(0, n - 1)(rng);
}

void ComputerLevel1::MakeMove(ComputerPlayer* player) {
	// Collect every legal move in one pass, split into those that do not leave
	// the piece capturable and those that do. The original version re-rolled a
	// random piece until it found a safe move, which never terminated when no
	// piece had one.
	std::vector<std::pair<std::string, std::string>> safe, all;

	for (const auto& piece : player->board->GetHand(player->player)) {
		const auto from = piece->Location();

		std::vector<std::string> candidates;
		for (const auto& to : *piece) {
			if (piece->CanMove(to)) candidates.push_back(to);
		}

		for (const auto& to : candidates) {
			all.emplace_back(from, to);
			if (!player->board->CanBeCaptured(to, player->player))
				safe.emplace_back(from, to);
		}
	}

	// Prefer a safe move, but play a hanging one rather than not moving.
	const auto& pool = safe.empty() ? all : safe;
	if (pool.empty()) throw _no_moves_found_{};

	const auto& choice = pool[rand_n(static_cast<int>(pool.size()))];

	try {
		player->board->MakeMove(player->ParseCommand(choice.first, choice.second));
	} catch (...) {
		throw;
	}
}
