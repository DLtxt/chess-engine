#ifndef PROMOTION_H
#define PROMOTION_H

#include "abstract_move.h"
#include "pieces/piece.h"

class Promotion final : public AbstractMove {

	Board *my_board;
	std::string from, to;

	std::shared_ptr<Piece> captured;
	std::shared_ptr<Piece> promoted;
	char promotion;
	bool promote;
	
	public:
	
		Promotion(const std::string&, const std::string&, char promo = PAWN, bool promote = false);
		
		void MakeMoveOn(Board*) override;
		void Undo() noexcept override;
};

#endif