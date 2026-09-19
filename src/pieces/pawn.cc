#include "pawn.h"
#include "board.h"

bool Pawn::IsEnPassant(const std::string& to) const {
	if (loc[1] != '4' || loc[1] != '5') return false;
	if (loc[0] == to[0] || abs(loc[1] - to[1]) > 1) return false;
	std::string en_passant_loc = std::string() + to[0] + loc[1];
	return (*board)[en_passant_loc] != nullptr && (*board)[en_passant_loc]->IsPawn() &&
			(*board)[en_passant_loc]->FirstMove() && board->LastMove() == en_passant_loc;
}

int Pawn::CapturedRank(const std::string& to) {
	auto capture_loc = to;
	if (IsEnPassant(to)) {
		capture_loc = std::string() + to[0] + loc[1];
	}
	const auto& capture = (*board)[capture_loc];
	if (capture != nullptr) return capture->Priority();
	return -1;
}

bool Pawn::CanMove(const std::string& to) {
	if (board->GetPiecePlayer(to) == player) return false;
	if (to[0] == loc[0]) {
		if (abs(to[1] - loc[1]) == 1) return board->Empty(to) && !board->IsRevealingKing(this, to);
		if (abs(to[1] - loc[1]) == 2) return board->Empty(to) && move_count == 0 && !board->IsRevealingKing(this, to);
	} else 
	if (abs(to[0] - loc[0]) == 1 && abs(to[1] - loc[1]) == 1) {
		return !board->Empty(to) || IsEnPassant(to);
	}
	return false;
}

bool Pawn::CanCapture(const std::string& to) {
	for (const auto& move : *this) {
		if (move == to && abs(to[0] - loc[0]) == 1 && abs(to[1] - loc[1]) == 1) return !board->IsRevealingKing(this, to);
	}
	return false;
}

bool Pawn::CanPromote(const std::string& to) {
	if (player == WHITE) return to[1] == board->TopRow();
	return to[1] == board->BotRow();
}

std::string Pawn::CaptureLoc(const std::string& to) {
	if (!board->Empty(to)) return to;
	if (IsEnPassant(to)) return std::string() + to[0] + loc[1];
	return "";
}