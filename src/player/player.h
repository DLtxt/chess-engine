#ifndef PLAYER_H
#define PLAYER_H

#include "pieces/piece.h"
#include <memory>
#include <vector>
#include <stack>
#include <sstream>
#include <memory>

class Board;
class Move;
class Parser;
class Piece;
class Vision;
class AbstractMove;

class Player {

	protected:	

		Board* board;
		char player;
		std::unique_ptr<Parser> parser;

		std::unique_ptr<Vision> vision;

	public:

		Player(Board*, char);

		virtual ~Player();

		std::unique_ptr<AbstractMove> ParseCommand(const std::string& from, const std::string& to, char promotion = QUEEN) noexcept;
		
		virtual void TakeAction() = 0;
		virtual void MakeMove() = 0;

		void RefreshVision();
		int CanSee(const std::string&) const;
		bool CanCapture(const std::string&) const;
};

#endif