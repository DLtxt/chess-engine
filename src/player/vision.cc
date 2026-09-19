#include "vision.h"
#include "chessboard.h"
#include "player.h"

void Vision::Refresh(Board* board, char player) {
	for (char c = board->LeftCol(); c <= board->RightCol(); ++c) {
		for (char r = board->BotRow(); r <= board->TopRow(); ++r) {
			std::string loc = std::string() + c + r;
			vision[loc] = 0;
		}
	}
	for (char c = board->LeftCol(); c <= board->RightCol(); ++c) {
		for (char r = board->BotRow(); r <= board->TopRow(); ++r) {
			auto loc = std::string() + c + r;
			
			if (board->GetPiecePlayer(loc) == player) {

				const auto& piece = (*board)[loc];
				for (const auto& move : *(*board)[loc]) {
					if (!piece->IsPawn()) ++vision[move];
					else if (move[0] != piece->Location()[0]) ++vision[move];
				}
			}
		}
	}
}

int Vision::CanSee(const std::string& loc) {
	return vision[loc];
}