#ifndef HUMAN_H
#define HUMAN_H

#include "player.h"

class HumanPlayer final : public Player {
	std::string from, to, promotion;
	public:
		HumanPlayer(Board* board, char player) : Player{board, player} {}
		
		void TakeAction() override;
		void MakeMove() override;
};

#endif