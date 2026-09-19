#include "pawn.h"
#include "shogi/shogi_board.h"

bool ShogiPawn::CanDrop() {
	if (player == BLACK && loc[1] == board->TopRow()) return false;
	if (player == WHITE && loc[1] == board->BotRow()) return false;

	for (char r = board->BotRow(); r <= board->TopRow(); ++r) {
		std::string nloc = std::string() + loc[0] + r;
		if (nloc != loc && !board->Empty(nloc) && (*board)[nloc]->IsPawn()) return false;
	}

	return !board->CheckMate();
}
