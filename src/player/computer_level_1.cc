#include "computer_player.h"
#include "parser.h"
#include "board.h"
#include <random>
#include <sstream>

// Seeding on every call meant that two calls in the same second returned the
// same value, which correlated the piece choice with the move choice.
int rand_n(int n) {
	static std::mt19937 rng{std::random_device{}()};
	if (n <= 0) return 0;
	return std::uniform_int_distribution<int>(0, n - 1)(rng);
}

void ComputerLevel1::MakeMove(ComputerPlayer* player) {
	while (true) {
		auto hand = player->board->GetHand(player->player);
		auto piece = hand[rand_n(hand.size())];
		// make a random move
		int count_moves = 0;

		for (const auto& to : *piece) {

			if (piece->CanMove(to) && !player->board->CanBeCaptured(to, player->player)) {
				++count_moves;
			}
		}

		if (count_moves == 0) continue; // no valid moves

		int rand_move = rand_n(count_moves) + 1;

		for (const auto& to: *piece) {

			if (piece->CanMove(to) && !player->board->CanBeCaptured(to, player->player)) {
				--rand_move;
			}

			if (rand_move == 0) {
				
				try {
					player->board->MakeMove(player->ParseCommand(piece->Location(), to));
					return;
				} catch (...) {
					throw;
				}
			}
		}
	}
}