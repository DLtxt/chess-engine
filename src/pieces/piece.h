#ifndef PIECE_H
#define PIECE_H

#include "iterators/iterator.h"
#include "iterators/slide_iterator.h"
#include "iterators/jump_iterator.h"
#include <string>
#include <memory>
#include <iostream>
#include <stack>

#define BLACK 'B'
#define WHITE 'W'
#define KING 'K'
#define QUEEN 'Q'
#define BISHOP 'B'
#define KNIGHT 'N'
#define ROOK 'R'
#define PAWN 'P'

#define LANCE 'L'
#define SILVER 'S'
#define GOLD 'G'
#define DRAGON 'D'
#define HORSE 'H'

class Board;
class Move;

class Piece {
	
	protected:
		
		Board* board;
		std::string loc;
		char name, player; // default capital = WHITE lowercase = BLACK
		int move_count;
	
	public:
	
		Piece(Board* board, const std::string& loc, char n, char p) : board{board}, loc{loc}, name{n}, player{p}, move_count{0} {}

		Piece(const Piece& other) : board{other.board}, loc{other.loc}, name{other.name}, player{other.player}, move_count{other.move_count} {}
		
		virtual ~Piece();
		
		char Name() const { return toupper(name); }
		char Player() const { return player; }
		std::string Location() const { return loc; }

		static const int HIGHEST_RANK = 100000;
		static const int PAWN_RANK = 100;
		static const int KNIGHT_RANK = 200;
		static const int BISHOP_RANK = 300;
		static const int ROOK_RANK = 500;
		static const int QUEEN_RANK = 1000;
		static const int KING_RANK = 2000;

		static const int DRAGON_RANK = 60;
		static const int HORSE_RANK = 60;
		static const int GOLD_RANK = 50;
		static const int SILVER_RANK = 40;
		static const int LANCE_RANK = 12;

		virtual int Priority() const { return false; }

		bool HasMoved() const { return move_count != 0; }
		bool FirstMove() const { return move_count == 1; }

		void TakeMove(const std::string&);
		void UndoMove(const std::string&);

		// if an opponent piece is on this square, can I capture?
		virtual bool CanCapture(const std::string&);
		// can I move to this square?
		virtual bool CanMove(const std::string&);
		virtual bool CanPromote(const std::string&);
		
		bool CanGetCaptured(const std::string&);
		virtual int CapturedRank(const std::string&);

		virtual bool CanDrop() { return true; }
		
		virtual bool IsKing() const { return false; }
		virtual bool IsPawn() const { return false; }
		virtual bool IsRook() const { return false; }

		virtual bool IsEnPassant(const std::string& to) const { return false; };
		virtual bool IsCastling(const std::string& to) const { return false; };

		virtual std::string CaptureLoc(const std::string& to) { return to; }

		/* iterate through all squares that can either move to or attack */
		class Iterator {
			friend class Piece;
			
			std::shared_ptr<PieceIterator> iterator;
			std::shared_ptr<PieceIterator> second_iterator;
			public:

				Iterator() : iterator{std::make_shared<PieceIterator>()}, second_iterator{std::make_shared<PieceIterator>()} {}

				Iterator(const std::shared_ptr<PieceIterator>& it) : iterator{it}, second_iterator{std::make_shared<PieceIterator>()} {}

				Iterator(const std::shared_ptr<PieceIterator>& it, const std::shared_ptr<PieceIterator>& it_2) : iterator{it}, second_iterator{it_2} {}

				bool operator!=(const Iterator& other) {
					return *iterator != *other.iterator || *second_iterator != *other.second_iterator;
				}

				void operator++() { 
					++(*iterator);
					if (iterator->Done()) iterator = second_iterator;
				}

				std::string operator*() { return **iterator; }
		};

		virtual Iterator begin() = 0;
		
		Iterator end() {
			return Iterator{};
		}
};

#endif