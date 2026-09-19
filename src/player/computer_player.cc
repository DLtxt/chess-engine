#include "computer_player.h"
#include "board.h"
#include "vision.h"

ComputerPlayer::ComputerPlayer(Board* board, char player, int level) : Player{board, player}, level{level}, level_4{board} {}

void ComputerPlayer::TakeAction() {
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
		try {
			MakeMove();
			vision->Refresh(board, player);
		} catch (...) {
			throw;
		}
	} else {
		throw _parsing_error_{"Player action is invalid."};
	}
}

void ComputerPlayer::MakeMove() {
	if (level >= 4) {
		try {
			level_4.MakeMove(this);
			return;
		} catch (...) {}
	}
	if (level >= 3) {
		try {
			level_3.MakeMove(this);
			return;
		} catch (...) {}
	}
	if (level >= 2) {
		try {
			level_2.MakeMove(this);
			return;
		} catch (...) {}
	}
	if (level >= 1) {
		try {
			level_1.MakeMove(this);
		} catch (...) {
			board->PlayerResign();
		}
	}
}