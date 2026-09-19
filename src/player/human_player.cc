#include "human_player.h"
#include "parser.h"
#include "moves/move.h"
#include "vision.h"
#include <iostream>
#include <sstream>

void HumanPlayer::TakeAction() {
	std::string readline, command;
	if (!getline(std::cin, readline)) throw _end_of_line_{};
	std::stringstream ss{readline};
	ss >> command;
	if (command.compare("resign") == 0) {
		board->PlayerResign();
	} else
	if (command.compare("draw") == 0) {
		board->Draw();
	} else
	if (command.compare("undo") == 0) {
		try {
			board->Undo();
		} catch (...) {
			throw;
		}
	} else 
	if (command.compare("move") == 0) {
		from = to = promotion = "";
		if (!(ss >> from)) { 
			throw _parsing_error_{"Missing first coordinate."}; 
		}
		if (!(ss >> to)) { 
			throw _parsing_error_{"Missing second coordinate."}; 
		}
		if (board->Empty(from)) { 
			throw _invalid_move_{"You should move a piece!"}; 
		}
		if (board->GetPiecePlayer(from) != player) { 
			throw _invalid_move_{"You should move one of your pieces!"}; 
		}
		if (from == to) { 
			throw _invalid_move_{"You should move to a different square!"}; 
		}
		if (board->GetPiecePlayer(to) == player) { 
			throw _invalid_move_{"You can't step over your own pieces!"}; 
		}
		if ((*board)[from]->CanPromote(to)) {
			if (!(ss >> promotion)) {
				throw _parsing_error_{"Missing pawn promotion."}; 
			}
			if (toupper(promotion[0]) == KING || toupper(promotion[0]) == PAWN) {
				throw _invalid_move_{"You should promote your pawn to a Queen, Bishop, Rook, or Knight only."};
			}
		}
		try {
			MakeMove();
			vision->Refresh(board, player);
		} catch (...) {
			throw;
		}
	} else {
		throw _parsing_error_{"Player action is unknown. Please try again."};
	}
}

void HumanPlayer::MakeMove() {
	std::unique_ptr<AbstractMove> move;
	if (promotion.empty()) {
		move = parser->ParseCommand(board, from, to);
	} else {
		move = parser->ParseCommand(board, from, to, promotion[0]);
	}
	try {
		board->MakeMove(std::move(move));
	} catch (...) {
		throw;
	}
}