#include "computer_player.h"
#include "board.h"
#include "parser.h"
#include "moves/abstract_move.h"

int ComputerLevel4::alpha_beta_max(ComputerPlayer* comp_player, size_t level, int alpha, int beta) {
	if (level == 0) {
		return board->BoardScore();
	}
	// make a move, then select one that yields max return if player is white, or min if black
	int best_score = - Board::MAX_SCORE - 1;
	for (const auto& piece : board->GetHand(WHITE)) {
		auto location = piece->Location();
		std::vector<std::string> valid_moves;
		for (const auto& move : *piece) if (piece->CanMove(move)) valid_moves.push_back(move);
		for (const auto& move : valid_moves) {
			board->ApplyMove(comp_player->ParseCommand(location, move));
			board->PlayerMovesNext(BLACK);
			best_score = std::max(best_score, alpha_beta_min(comp_player, level - 1, alpha, beta));
			board->PlayerMovesNext(WHITE);
			board->Undo();
			if (best_score > beta) return best_score;
			if (alpha < best_score) {
				alpha = best_score;
				if (level == START_LEVEL) {
					best_from = location;
					best_move = move;
				}
			}
		}
	}
	return best_score;
}

int ComputerLevel4::alpha_beta_min(ComputerPlayer* comp_player, size_t level, int alpha, int beta) {
	if (level == 0) {
		return board->BoardScore();
	}
	// make a move, then select one that yields max return if player is white, or min if black
	int best_score = Board::MAX_SCORE + 1;
	for (const auto& piece : board->GetHand(BLACK)) {
		auto location = piece->Location();
		std::vector<std::string> valid_moves;
		for (const auto& move : *piece) if (piece->CanMove(move)) valid_moves.push_back(move);
		for (const auto& move : valid_moves) {
			board->ApplyMove(comp_player->ParseCommand(location, move));
			board->PlayerMovesNext(WHITE);
			best_score = std::min(best_score, alpha_beta_max(comp_player, level - 1, alpha, beta));
			board->PlayerMovesNext(BLACK);
			board->Undo();
			if (best_score < alpha) return best_score;
			if (best_score < beta) {
				beta = best_score;
				if (level == START_LEVEL) {
					best_from = location;
					best_move = move;
				}
			}
		}
	}
	return best_score;
}

void ComputerLevel4::MakeMove(ComputerPlayer* comp_player) {

	if (++move_count < 2) throw _no_moves_found_{};

	player = comp_player->player;

	START_LEVEL = 3;
	
	if (player == WHITE) alpha_beta_max(comp_player, START_LEVEL, -Board::MAX_SCORE - 1, Board::MAX_SCORE + 1);
	else alpha_beta_min(comp_player, START_LEVEL, -Board::MAX_SCORE - 1, Board::MAX_SCORE + 1);
	
	try {
		board->MakeMove(comp_player->ParseCommand(best_from, best_move));
		return;
	} catch (...) {
		throw;
	}

	throw _no_moves_found_{};
}
