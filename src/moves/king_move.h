#ifndef KING_MOVE_H
#define KING_MOVE_H

#include "abstract_move.h"

class KingMove final : public AbstractMove {
	
	Board *my_board;
	std::string from, to;

	int capture_count;
	std::shared_ptr<Piece> captured;

	std::string rook_loc, rook_dest;
	std::shared_ptr<Piece> rook;
	
	public:

		KingMove(const std::string&, const std::string&);
		void MakeMoveOn(Board*) override;
		void Undo() noexcept override;
};

#endif