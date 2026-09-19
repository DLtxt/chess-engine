#include "king.h"
#include "board.h"

bool King::IsCastling(const std::string& to) const {
	if (board->Checked()) return false; // can't castle if not king or king in check
	if (HasMoved()) return false; // can't castle if king has moved
	if (abs(to[0] - loc[0]) != 2 || to[1] != loc[1]) return false;
	int castle_dir = (to[0] - loc[0]) / 2;
	std::string rook_loc = to;
	std::string rook_dest = loc;
	rook_dest[0] += castle_dir;
	if ((*board)[rook_dest]) return false; // can't castle if there are pieces on the way
	while (board->InBound(rook_loc)) {
		const auto& rook = (*board)[rook_loc];
		rook_loc[0] += castle_dir;
		if (!rook) continue;
		if (!rook->IsRook()) return false; // can't castle if there are pieces on the way
		return !rook->HasMoved();
	}
	return false;
}

bool King::CanMove(const std::string& to) {
	if (abs(loc[0] - to[0]) == 2 && !IsCastling(to)) return false;
	return board->GetPiecePlayer(to) != player && CanCapture(to);
}

// bool King::CanCapture(const std::string& to) {
// 	for (const auto& move : *this) {
// 		if (move == to) return !board->CanBeSeen(to, player);
// 	}
// }