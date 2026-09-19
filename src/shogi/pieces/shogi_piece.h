#ifndef SHOGI_PIECE_H
#define SHOGI_PIECE_H

#include "pieces/piece.h"

class ShogiPiece : public Piece {
	
	public:
		
		ShogiPiece(Board* board, const std::string& loc, char name, char player) : Piece(board, loc, name, player) {}

		bool CanPromote(const std::string&) override;

		Iterator begin() override = 0;
};

#endif