#include "computer_level_5.h"

#include "board.h"
#include "computer_player.h"
#include "moves/abstract_move.h"
#include "parser.h"

#include "engine/book.h"
#include "engine/engine.h"
#include "engine/movegen.h"
#include "engine/position.h"
#include "engine/search.h"

#include <cctype>
#include <string>

namespace {

bool engine_ready = false;

void ensure_engine() {
	if (engine_ready) return;
	eng::init_engine(64);
	engine_ready = true;
}

// Is there an unmoved piece of this exact name and colour on this square?
bool unmoved_piece(Board* board, const std::string& loc, char name, char owner) {
	const auto& piece = (*board)[loc];
	return piece != nullptr && piece->Name() == name
	    && piece->Player() == owner && !piece->HasMoved();
}

} // namespace

std::string BoardToFen(Board* board, char player) {
	std::string fen;

	for (char r = '8'; r >= '1'; --r) {
		int gap = 0;
		for (char c = 'a'; c <= 'h'; ++c) {
			const std::string loc = std::string() + c + r;
			const char name = board->GetPieceName(loc);
			if (name == ' ') { ++gap; continue; }
			if (gap) { fen += std::to_string(gap); gap = 0; }
			fen += (board->GetPiecePlayer(loc) == WHITE)
				? char(std::toupper(name))
				: char(std::tolower(name));
		}
		if (gap) fen += std::to_string(gap);
		if (r != '1') fen += '/';
	}

	fen += ' ';
	fen += (player == WHITE) ? 'w' : 'b';
	fen += ' ';

	// The legacy board tracks move counts rather than castling rights, so the
	// rights are reconstructed from which kings and rooks are still untouched.
	std::string castling;
	if (unmoved_piece(board, "e1", KING, WHITE)) {
		if (unmoved_piece(board, "h1", ROOK, WHITE)) castling += 'K';
		if (unmoved_piece(board, "a1", ROOK, WHITE)) castling += 'Q';
	}
	if (unmoved_piece(board, "e8", KING, BLACK)) {
		if (unmoved_piece(board, "h8", ROOK, BLACK)) castling += 'k';
		if (unmoved_piece(board, "a8", ROOK, BLACK)) castling += 'q';
	}
	fen += castling.empty() ? "-" : castling;
	fen += ' ';

	// An en-passant square exists only if the opponent's last move was a double
	// pawn push, which shows up as a pawn on its fourth rank that has moved
	// exactly once and was the piece that just moved.
	const char opponent = (player == WHITE) ? BLACK : WHITE;
	const char landed_rank = (player == WHITE) ? '5' : '4';
	const char skipped_rank = (player == WHITE) ? '6' : '3';

	std::string ep = "-";
	for (char c = 'a'; c <= 'h'; ++c) {
		const std::string loc = std::string() + c + landed_rank;
		const auto& piece = (*board)[loc];
		if (piece != nullptr && piece->IsPawn() && piece->Player() == opponent
		    && piece->FirstMove() && board->LastMoved(loc)) {
			ep = std::string() + c + skipped_rank;
			break;
		}
	}
	fen += ep;
	fen += " 0 1";

	return fen;
}

void ComputerLevel5::MakeMove(ComputerPlayer* player) {
	// The bitboard engine is 8x8 chess only; Shogi falls through to level 4.
	if (player->board->BoardSize() != 8) throw _no_moves_found_{};

	ensure_engine();

	eng::Position pos;
	if (!pos.set_fen(BoardToFen(player->board, player->player)))
		throw _no_moves_found_{};

	// Play straight from the opening book while it still knows the position.
	eng::Move chosen = eng::probe_book(pos);

	if (chosen == eng::kMoveNone) {
		eng::SearchLimits limits;
		if (depth_ > 0) limits.depth = depth_;
		else limits.movetime = movetime_ms_;

		const eng::SearchResult result =
			eng::search_parallel(pos, limits, threads_, false);
		chosen = chosen;
	}

	if (chosen == eng::kMoveNone) throw _no_moves_found_{};

	const std::string from = eng::square_to_string(eng::from_sq(chosen));
	const std::string to = eng::square_to_string(eng::to_sq(chosen));

	// ParseCommand treats PAWN as "no promotion requested".
	char promotion = PAWN;
	if (eng::move_type(chosen) == eng::kPromotion) {
		switch (eng::promotion_type(chosen)) {
			case eng::kQueen:  promotion = QUEEN;  break;
			case eng::kRook:   promotion = ROOK;   break;
			case eng::kBishop: promotion = BISHOP; break;
			case eng::kKnight: promotion = KNIGHT; break;
			default:           promotion = QUEEN;  break;
		}
	}

	try {
		player->board->MakeMove(player->ParseCommand(from, to, promotion));
	} catch (...) {
		throw;
	}
}
