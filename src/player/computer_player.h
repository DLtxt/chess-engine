#ifndef COMPUTER_PLAYER
#define COMPUTER_PLAYER

#include "pieces/piece.h"
#include "player.h"
#include "computer_level_1.h"
#include "computer_level_2.h"
#include "computer_level_3.h"
#include "computer_level_4.h"

class Board;

class ComputerPlayer final : public Player {

	friend class ComputerLevel1;
	friend class ComputerLevel2;
	friend class ComputerLevel3;
	friend class ComputerLevel4;
	
	int level;
	ComputerLevel1 level_1;
	ComputerLevel2 level_2;
	ComputerLevel3 level_3;
	ComputerLevel4 level_4;

	protected:

	public:
		ComputerPlayer(Board*, char, int);
		
		virtual ~ComputerPlayer() {}

		void TakeAction() override;
		void MakeMove() override;
};

#endif