#ifndef SHOGI_HUMAN_H
#define SHOGI_HUMAN_H

#include "player/player.h"

class ShogiPlayer final : public Player {

	std::string from, to, command;

	public:
		
		ShogiPlayer(Board* board, char player) : Player{board, player} {}
		
		void TakeAction() override;
		void MakeMove() override;
};

#endif