#include "lance.h"
#include "shogi/shogi_board.h"

bool ShogiLance::CanDrop() {
	if (player == BLACK && loc[1] == board->TopRow()) return false;
	if (player == WHITE && loc[1] == board->BotRow()) return false;
	return true;
}
