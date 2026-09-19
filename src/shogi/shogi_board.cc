#include "shogi_board.h"
#include "moves/move.h"
#include "player/player.h"
#include "player/human_player.h"
#include "player/computer_player.h"
#include "shogi/pieces/bishop.h"
#include "shogi/pieces/dragon.h"
#include "shogi/pieces/gold.h"
#include "shogi/pieces/horse.h"
#include "shogi/pieces/king.h"
#include "shogi/pieces/knight.h"
#include "shogi/pieces/lance.h"
#include "shogi/pieces/pawn.h"
#include "shogi/pieces/rook.h"
#include "shogi/pieces/silver.h"
#include "player/player.h"
#include "shogi/shogi_player.h"

void ShogiBoard::Reset() {
	Clear();
	SetPiece("a1", LANCE, BLACK);
	SetPiece("b1", KNIGHT, BLACK);
	SetPiece("c1", SILVER, BLACK);
	SetPiece("d1", GOLD, BLACK);
	SetPiece("e1", KING, BLACK);
	SetPiece("f1", GOLD, BLACK);
	SetPiece("g1", SILVER, BLACK);
	SetPiece("h1", KNIGHT, BLACK);
	SetPiece("i1", LANCE, BLACK);
	SetPiece("b2", BISHOP, BLACK);
	SetPiece("g2", ROOK, BLACK);
	for (char c = LEFT_COL; c <= RIGHT_COL; ++c) {
		SetPiece(std::string() + c + '3', PAWN, BLACK);
	}
	SetPiece("a9", LANCE, WHITE);
	SetPiece("b9", KNIGHT, WHITE);
	SetPiece("c9", SILVER, WHITE);
	SetPiece("d9", GOLD, WHITE);
	SetPiece("e9", KING, WHITE);
	SetPiece("f9", GOLD, WHITE);
	SetPiece("g9", SILVER, WHITE);
	SetPiece("h9", KNIGHT, WHITE);
	SetPiece("i9", LANCE, WHITE);
	SetPiece("b8", ROOK, WHITE);
	SetPiece("g8", BISHOP, WHITE);
	for (char c = LEFT_COL; c <= RIGHT_COL; ++c) {
		SetPiece(std::string() + c + '7', PAWN, WHITE);
	}
	king_loc[BLACK] = "e1";
	king_loc[WHITE] = "e9";
	captured_by[BLACK].clear();
	captured_by[WHITE].clear();
	game_over = false;
	draw = false;
	player = BLACK;
	opponent = WHITE;
}

void ShogiBoard::Clear() {
	for (char c = LEFT_COL; c <= RIGHT_COL; ++c) {
		for (char r = BOT_ROW; r <= TOP_ROW; ++r) {
			RemovePiece(std::string() + c + r);
		}
	}
	king_loc[player] = king_loc[opponent] = "";
}

void ShogiBoard::SetPiece(const std::string& loc, char name, char player) {
	RemovePiece(loc);
	switch (toupper(name))
	{
	case BISHOP:
		pieces[loc] = std::make_unique<ShogiBishop>(this, loc, player);
		break;
	case DRAGON:
		pieces[loc] = std::make_unique<ShogiDragon>(this, loc, player);
		break;
	case GOLD:
		pieces[loc] = std::make_unique<ShogiGold>(this, loc, player);
		break;
	case HORSE:
		pieces[loc] = std::make_unique<ShogiHorse>(this, loc, player);
		break;
	case KING:
		if (king_loc[player] != "") {
			throw _two_kings_{};
		}
		pieces[loc] = std::make_unique<ShogiKing>(this, loc, player);
		king_loc[player] = loc;
		break;
	case KNIGHT:
		pieces[loc] = std::make_unique<ShogiKnight>(this, loc, player);
		break;
	case LANCE:
		pieces[loc] = std::make_unique<ShogiLance>(this, loc, player);
		break;
	case PAWN:
		pieces[loc] = std::make_unique<ShogiPawn>(this, loc, player);
		break;
	case ROOK:
		pieces[loc] = std::make_unique<ShogiRook>(this, loc, player);
		break;
	case SILVER:
		pieces[loc] = std::make_unique<ShogiSilver>(this, loc, player);
		break;
	default:
		break;
	}
	refresh_vision();
}

void ShogiBoard::AddHumanPlayer() {
	players[player] = std::make_shared<ShogiPlayer>(this, player);
	std::swap(player, opponent);
}