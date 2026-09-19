#include "computer_player.h"
#include "computer_level_3.h"
#include "board.h"
#include "parser.h"
#include "moves/abstract_move.h"

void ComputerLevel3::MakeMove(ComputerPlayer* player) {
	int highest_rank = 0;
	std::string escape_from = "", escape_to = "";

	for (const auto& piece : player->board->GetHand(player->player)) {

		if (player->board->CanBeSeen(piece->Location(), player->player) && piece->Priority() >= highest_rank) {

			auto from = piece->Location();

			for (const auto& to : *piece) {
				if (!escape_to.empty()) break;
				if (!piece->CanMove(to)) continue;

				player->board->ApplyMove(player->ParseCommand(from, to));

				if (!piece->CanGetCaptured(piece->Location())) {

					highest_rank = piece->Priority();
					escape_from = from;
					escape_to = to;
				}

				player->board->Undo();
			}

			if (!escape_to.empty()) break;

			for (const auto& savior : player->board->GetHand(player->player)) {
				if (savior == piece) continue;

				for (const auto& savior_move : *savior) {
					if (!escape_to.empty()) break;
					if (!savior->CanMove(savior_move)) continue;

					auto savior_from = savior->Location();
					player->board->ApplyMove(player->ParseCommand(savior_from, savior_move));

					if (!piece->CanGetCaptured(piece->Location()) && (!savior->CanGetCaptured(savior->Location()) || savior->Priority() < piece->Priority())) {

						highest_rank = piece->Priority();
						escape_from = savior_from;
						escape_to = savior_move;
					}

					player->board->Undo();
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
