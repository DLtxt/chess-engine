#include "computer_player.h"
#include "computer_level_2.h"
#include "board.h"
#include "parser.h"
#include "moves/abstract_move.h"
#include <vector>

void ComputerLevel2::MakeMove(ComputerPlayer* player) {
	int highest_captured_rank = 0, lowest_used_rank = Piece::HIGHEST_RANK;

	std::string checkmate_move_from = "", checkmate_move_to = "";
	std::string stalemate_move_from = "", stalemate_move_to = "";
	std::string capture_move_from = "", capture_move_to = "";
	std::string check_move_from = "", check_move_to = "";
	std::string move_from = "", move_to = "";

	for (const auto& piece : player->board->GetHand(player->player)) {

		auto from = piece->Location();

		// Snapshot the candidates before touching the board: ApplyMove relocates
		// the piece, which invalidates the iterator we would still be walking.
		std::vector<std::string> candidates;
		for (const auto& to : *piece) {
			if (piece->CanMove(to)) candidates.push_back(to);
		}

		for (const auto& to : candidates) {

			int captured_rank = piece->CapturedRank(to);

			try {
				player->board->ApplyMove(player->ParseCommand(from, to));
			} catch (...) {
				continue;  // nothing was applied, so there is nothing to undo
			}

			try {

				if ((captured_rank >= piece->Priority() || !player->board->CanBeCaptured(to, player->player)) &&
					(captured_rank > highest_captured_rank || (captured_rank == highest_captured_rank && piece->Priority() < lowest_used_rank))) {

					highest_captured_rank = captured_rank;
					lowest_used_rank = piece->Priority();
					capture_move_from = from;
					capture_move_to = to;
				}

				if (!player->board->CanBeCaptured(to, player->player) && player->board->Check()) {

					check_move_from = from;
					check_move_to = to;
				}

				if (player->board->CheckMate()) {

					checkmate_move_from = from;
					checkmate_move_to = to;
				}

			} catch (...) {}

			// Outside the try, so a throw from the tests above can never leave the
			// trial move applied to the live board.
			try {
				player->board->Undo();
			} catch (...) {}
		}
	}

	std::unique_ptr<AbstractMove> move = nullptr;

	if (!checkmate_move_to.empty()) {
		move = player->ParseCommand(checkmate_move_from, checkmate_move_to);
	} else 
	if (!capture_move_to.empty()) {
		move = player->ParseCommand(capture_move_from, capture_move_to);
	} else 
	if (!check_move_to.empty()) {
		move = player->ParseCommand(check_move_from, check_move_to);
	}

	if (move != nullptr) {
		try {
			player->board->MakeMove(std::move(move));
			return;
		} catch (...) {
			throw;
		}
	}

	throw _no_moves_found_{};
}