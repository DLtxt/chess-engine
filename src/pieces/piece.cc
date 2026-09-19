#include "piece.h"
#include "chessboard.h"
#include "moves/move.h"

Piece::~Piece() {}

bool Piece::CanCapture(const std::string& to) {
	// compare this move with all valid moves
	for (const auto move : *this) {
		if (move == to) return !board->IsRevealingKing(this, to);
	}
	// no match, not a valid move
	// can't move away if current piece is protecting the king
	return false;
}

bool Piece::CanMove(const std::string& to) {
	// piece can move if it's following the rules and not stepping over
	return board->GetPiecePlayer(to) != player && CanCapture(to);
}

bool Piece::CanPromote(const std::string& to) {
	return false;
}

int Piece::CapturedRank(const std::string& loc) {
	const auto& capture = (*board)[loc];
	if (capture != nullptr) return capture->Priority();
	return -1;
}

bool Piece::CanGetCaptured(const std::string& loc) {
	return board->CanBeCaptured(loc, player);
}

void Piece::TakeMove(const std::string& to) {
	board->MovePiece(loc, to);
	loc = to;
	++move_count;
}

void Piece::UndoMove(const std::string& from) {
	board->UndoMove(from, loc);
	loc = from;
	--move_count;
}