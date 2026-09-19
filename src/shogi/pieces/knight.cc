#include "knight.h"
#include "shogi/shogi_board.h"

bool ShogiKnight::CanDrop() {
	if (player == BLACK && loc[1] >= board->TopRow() - 1) return false;
	if (player == WHITE && loc[1] <= board->BotRow() + 1) return false;
	return true;
}
