#ifndef SHOGI_BOARD_H
#define SHOGI_BOARD_H

#include "board.h"

class ShogiBoard : public Board {
	
	static const char LEFT_COL = 'a';
	static const char RIGHT_COL = 'i';
	static const char TOP_ROW =	'9';
	static const char BOT_ROW =	'1';

	static const int ROW_DIM = 9;
	static const int COL_DIM = 9;

	public:

		char LeftCol() const override { return LEFT_COL; }
		char RightCol() const override { return RIGHT_COL; }
		char TopRow() const override { return TOP_ROW; }
		char BotRow() const override { return BOT_ROW; }

		int BoardSize() const override { return ROW_DIM; }

		ShogiBoard() : Board{} { 
			player = BLACK; 
			opponent = WHITE; 
		}

		void Reset() override;
		void Clear() override;
		void SetPiece(const std::string&, char name, char player) override;

		void AddHumanPlayer() override;

		bool InBound(const std::string& loc) override {
			return LEFT_COL <= loc[0] && loc[0] <= RIGHT_COL && BOT_ROW <= loc[1] && loc[1] <= TOP_ROW;
		}

};

#endif