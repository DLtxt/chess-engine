#ifndef PAWN_H
#define PAWN_H

#include "piece.h"
#include "iterators/pawn_iterator.h"

class Pawn : public Piece {
	public:
		Pawn(Board* board, const std::string& loc, char player) : Piece{board, loc, PAWN, player} {}

		bool IsPawn() const override { return true; }
		bool IsEnPassant(const std::string&) const override; 

		bool CanCapture(const std::string&) override;
		bool CanMove(const std::string&) override;
		bool CanPromote(const std::string&) override;

		int Priority() const override { return Piece::PAWN_RANK; }

		int CapturedRank(const std::string&) override;

		std::string CaptureLoc(const std::string& to) override;

		Iterator begin() override {
			auto iter = Iterator{std::make_shared<PawnIterator>(board, loc)};
			++iter;
			return iter;
		}
};

#endif