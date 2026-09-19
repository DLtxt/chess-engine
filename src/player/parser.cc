#include "parser.h"
#include "moves/move.h"
#include "moves/king_move.h"
#include "moves/promotion.h"
#include <iostream>

std::unique_ptr<AbstractMove> Parser::ParseCommand(Board* board, const std::string& from, const std::string& to, char promotion) noexcept {
	// Move move;
	switch (toupper(board->GetPieceName(from))) {
	case KING:
		return std::make_unique<KingMove>(from, to);
	case PAWN:
		return std::make_unique<Promotion>(from, to, promotion, promotion != PAWN);
	default:
		return std::make_unique<Move>(from, to);
	}
}