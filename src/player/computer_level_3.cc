#include "computer_player.h"
#include "computer_level_3.h"
#include "board.h"
#include "parser.h"
#include "moves/abstract_move.h"
#include <vector>

void ComputerLevel3::MakeMove(ComputerPlayer* player) {
	int highest_rank = 0;
	std::string escape_from = "", escape_to = "";

	for (const auto& piece : player->board->GetHand(player->player)) {

		if (player->board->CanBeSeen(piece->Location(), player->player) && piece->Priority() >= highest_rank) {

			auto from = piece->Location();

			// Snapshot before mutating: ApplyMove moves the piece out from under
			// the iterator that is still walking its move list.
			std::vector<std::string> candidates;
			for (const auto& to : *piece) {
				if (piece->CanMove(to)) candidates.push_back(to);
			}

			for (const auto& to : candidates) {
				if (!escape_to.empty()) break;

				try {
					player->board->ApplyMove(player->ParseCommand(from, to));
				} catch (...) {
					continue;
				}

				try {
					if (!piece->CanGetCaptured(piece->Location())) {
						highest_rank = piece->Priority();
						escape_from = from;
						escape_to = to;
					}
				} catch (...) {}

				try { player->board->Undo(); } catch (...) {}
			}

			if (!escape_to.empty()) break;

			for (const auto& savior : player->board->GetHand(player->player)) {
				if (savior == piece) continue;
				if (!escape_to.empty()) break;

				const auto savior_from = savior->Location();

				std::vector<std::string> savior_moves;
				for (const auto& savior_move : *savior) {
					if (savior->CanMove(savior_move)) savior_moves.push_back(savior_move);
				}

				for (const auto& savior_move : savior_moves) {
					if (!escape_to.empty()) break;

					try {
						player->board->ApplyMove(player->ParseCommand(savior_from, savior_move));
					} catch (...) {
						continue;
					}

					try {
						if (!piece->CanGetCaptured(piece->Location()) && (!savior->CanGetCaptured(savior->Location()) || savior->Priority() < piece->Priority())) {
							highest_rank = piece->Priority();
							escape_from = savior_from;
							escape_to = savior_move;
						}
					} catch (...) {}

					try { player->board->Undo(); } catch (...) {}
				}
			}
		}
	}

	if (!escape_to.empty()) {
		try {
			player->board->MakeMove(player->ParseCommand(escape_from, escape_to));
			return;
		} catch (...) {
			throw;
		}
	}
	
	throw _no_moves_found_{};
}
