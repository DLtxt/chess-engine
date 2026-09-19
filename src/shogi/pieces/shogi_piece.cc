#include "shogi/pieces/shogi_piece.h"
#include "shogi/shogi_board.h"

bool ShogiPiece::CanPromote(const std::string& to) { 
	return player == BLACK ? to[1] >= board->TopRow() - 2 : to[1] <= board->BotRow() + 2;
}