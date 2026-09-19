#include "board.h"
#include "moves/move.h"
#include "pieces/king.h"
#include "pieces/queen.h"
#include "pieces/bishop.h"
#include "pieces/knight.h"
#include "pieces/pawn.h"
#include "pieces/rook.h"
#include "player/player.h"
#include "player/human_player.h"
#include "player/computer_player.h"
#include "shogi/shogi_player.h"
#include <iostream>

void Board::SetPiece(const std::string& loc, char name, char player) {
	RemovePiece(loc);
	switch(toupper(name)) {
		case KING:
			if (king_loc[player] != "") {
				throw _two_kings_{};
			}
			pieces[loc] = std::make_unique<King>(this, loc, player);
			king_loc[player] = loc;
			break;
		case QUEEN:
			pieces[loc] = std::make_unique<Queen>(this, loc, player);
			break;
		case BISHOP:
			pieces[loc] = std::make_unique<Bishop>(this, loc, player);
			break;
		case KNIGHT:
			pieces[loc] = std::make_unique<Knight>(this, loc, player);
			break;
		case PAWN:
			if (loc[1] == BOT_ROW || loc[1] == TOP_ROW) {
				throw _pawn_first_rank_{};
			}
			pieces[loc] = std::make_unique<Pawn>(this, loc, player);
			break;
		case ROOK:
			pieces[loc] = std::make_unique<Rook>(this, loc, player);
			break;
	}
	refresh_vision();
}

void Board::refresh_vision() {
	for (const auto& [team, player] : players) {
		if (player != nullptr) player->RefreshVision();
	}
}

char Board::GetPieceName(const std::string& loc) { 
	if (Empty(loc)) return ' ';
	return pieces[loc]->Name();
}

char Board::GetPiecePlayer(const std::string& loc) {
	if (Empty(loc)) return ' ';
	return pieces[loc]->Player();
}

char Board::PrintPieceName(const std::string& loc) {
	if (Empty(loc)) return ' ';
	return pieces[loc]->Name();
}

void Board::RemovePiece(const std::string& loc) {
	if (pieces[loc] != BLANK && pieces[loc]->IsKing()) king_loc[pieces[loc]->Player()] = "";
	pieces[loc] = BLANK;
}

void Board::PlayerMovesNext(char next_player) {
	if (next_player == WHITE) { player = WHITE; opponent = BLACK; }
	if (next_player == BLACK) { player = BLACK; opponent = WHITE; }
}

void Board::Clear() {
	for (char c = LEFT_COL; c <= RIGHT_COL; ++c) {
		for (char r = BOT_ROW; r <= TOP_ROW; ++r) {
			RemovePiece(std::string() + c + r);
		}
	}
	king_loc[player] = king_loc[opponent] = "";
	while (!moves.empty()) moves.pop();
	captured_by[WHITE].clear();
	captured_by[BLACK].clear();
}

void Board::Print() { 
	refresh_vision();
	NotifyObservers(); 
}

void Board::SetUpDone() { 
	if (king_loc[WHITE] == "") {
		throw _missing_white_king_{};
	}
	if (king_loc[BLACK] == "") {
		throw _missing_black_king_{};
	}
}

void Board::Reset() {
	Clear();
	SetPiece("a1", ROOK, WHITE);
	SetPiece("b1", KNIGHT, WHITE);
	SetPiece("c1", BISHOP, WHITE);
	SetPiece("d1", QUEEN, WHITE);
	SetPiece("e1", KING, WHITE);
	SetPiece("f1", BISHOP, WHITE);
	SetPiece("g1", KNIGHT, WHITE);
	SetPiece("h1", ROOK, WHITE);
	for (char c = 'a'; c <= 'h'; ++c) {
		SetPiece(std::string() + c + '2', PAWN, WHITE);
	}
	SetPiece("a8", ROOK, BLACK);
	SetPiece("b8", KNIGHT, BLACK);
	SetPiece("c8", BISHOP, BLACK);
	SetPiece("d8", QUEEN, BLACK);
	SetPiece("e8", KING, BLACK);
	SetPiece("f8", BISHOP, BLACK);
	SetPiece("g8", KNIGHT, BLACK);
	SetPiece("h8", ROOK, BLACK);
	for (char c = 'a'; c <= 'h'; ++c) {
		SetPiece(std::string() + c + '7', PAWN, BLACK);
	}
	king_loc[WHITE] = "e1";
	king_loc[BLACK] = "e8";
	game_over = false;
	draw = false;
	player = WHITE;
	opponent = BLACK;
}

void Board::AddHumanPlayer() { 
	players[player] = std::make_shared<HumanPlayer>(this, player);
	std::swap(player, opponent);
}

void Board::AddComputerPlayer(int level) {
	players[player] = std::make_shared<ComputerPlayer>(this, player, level);
	std::swap(player, opponent);
}

std::vector<std::shared_ptr<Piece>> Board::GetHand(char player) {
	std::vector<std::shared_ptr<Piece>> hand;
	for (const auto& [loc, piece] : pieces) {
		if (piece != nullptr && piece->Player() == player) {
			hand.push_back(piece);
			if (piece->Location() != loc) {
				std::cout << "Piece is on " << loc << " but it thinks it's on " << piece->Location() << std::endl;
			}
		}
	}
	std::shuffle(hand.begin(), hand.end(), gen);
	return hand;
}

void Board::PlayerMakeMove() {
	if (player == WHITE) {
		std::cout << "White to move." << std::endl;
	} else {
		std::cout << "Black to move." << std::endl;
	}
	try {
		players[player]->TakeAction();
	} catch (...) {
		throw;
	}
	std::swap(player, opponent);
}

void Board::DisplayScores() {
	std::cout << "White score: " << scores[WHITE] << std::endl;
	std::cout << "Black score: " << scores[BLACK] << std::endl;
}

void Board::MovePiece(const std::string& from, const std::string& to) noexcept {
	pieces[to] = std::move(pieces[from]);
	move_origin.push(from);
	move_path.push(to);
}

void Board::UndoMove(const std::string& from, const std::string& to) noexcept {
	pieces[from] = std::move(pieces[to]);
	move_origin.pop();
	move_path.pop();
}

bool Board::LastMoved(const std::string& loc) {
	if (move_origin.empty()) return false;
	return loc == move_origin.top() || loc == move_path.top();
}

void Board::KingIsHere(const std::string& loc) {
	if (pieces[loc] == nullptr) return;
	king_loc[pieces[loc]->Player()] = loc;
}

void Board::Place(std::shared_ptr<Piece> piece) {
	if (piece != nullptr) pieces[piece->Location()] = std::move(piece);
}

std::shared_ptr<Piece> Board::Retrieve(const std::string& loc) {
	if (loc.empty()) return nullptr;
	return std::move(pieces[loc]);
}

void Board::Capture(char piece_name, char player) {
	char opponent = player == WHITE ? BLACK : WHITE;
	if (toupper(piece_name) == KING) {
		game_over = true;
		++scores[opponent];
	}
	captured_by[opponent].push_back(piece_name);
}

void Board::RemoveCapture(char piece_name, char player) {
	captured_by[player].erase(std::find(captured_by[player].begin(), captured_by[player].end(), piece_name));
}

void Board::Release(char player) {
	char opponent = player == WHITE ? BLACK : WHITE;
	if (toupper(captured_by[opponent].back()) == KING) {
		game_over = false;
		--scores[opponent];
	}
	captured_by[opponent].pop_back();
}

bool Board::CanBeCaptured(const std::string& loc, char player) {
	char opponent = player == WHITE ? BLACK : WHITE;
	return players[opponent] != nullptr ? players[opponent]->CanCapture(loc) : false;
}

bool Board::CanBeSeen(const std::string& loc, char player) {
	char opponent = player == WHITE ? BLACK : WHITE;
	return players[opponent]->CanSee(loc);
}

std::vector<char> Board::CapturedBy(char player) {
	return captured_by[player];
}

void Board::ApplyMove(std::unique_ptr<AbstractMove> move) {

	if (move == nullptr) throw _invalid_move_{"Can't pass a null move."};
	
	try {
		move->MakeMoveOn(this);
		moves.emplace(std::move(move));
		refresh_vision();
	} catch (...) {
		throw;
	}

}

void Board::MakeMove(std::unique_ptr<AbstractMove> move) {

	try {
		ApplyMove(std::move(move));
	} catch (...) {
		throw;
	}

	if (game_over) return;
	
	if (CheckMate()) {
		std::cout << "Checkmate!" << std::endl;
		++scores[player];
		game_over = true;
	} else 
	if (Check()) {
		std::cout << "Check!" << std::endl;
	} else
	if (StaleMate()) {
		std::cout << "Stalemate!" << std::endl;
		scores[player] += 0.5;
		scores[opponent] += 0.5;
		game_over = true;
	}
}

void Board::Undo() {
	if (moves.empty()) {
		throw _invalid_move_{"Can't undo anymore!"};
	}
	moves.top()->Undo();
	moves.pop();
	refresh_vision();
}

bool Board::IsRevealingKing(Piece* piece, const std::string& to) {
	auto opponent = piece->Player() == WHITE ? BLACK : WHITE;
	auto tmp = piece->Location();
	auto capture = std::move(pieces[to]);
	piece->TakeMove(to);
	if (piece->IsKing()) KingIsHere(to);
	players[opponent]->RefreshVision();
	bool king_under_attack = players[opponent]->CanSee(king_loc[piece->Player()]);
	piece->UndoMove(tmp);
	if (piece->IsKing()) KingIsHere(tmp);
	pieces[to] = std::move(capture);
	players[opponent]->RefreshVision();
	return king_under_attack;
}

std::string Board::FindSafePlace(const std::shared_ptr<Piece>& piece) {
	std::string escape_move = "";
	int max_diff = -1;
	for (const auto& to : *piece) {
		if (!piece->CanMove(to)) continue;
		auto move = players[player]->ParseCommand(piece->Location(), to);
		ApplyMove(std::move(move));
		int vision_diff = players[player]->CanSee(to) - players[opponent]->CanSee(to);
		if (vision_diff > max_diff) {
			max_diff = vision_diff;
			escape_move = to;
		}
		Undo();
	}
	return escape_move;
}

bool Board::Check() {
	return players[player]->CanSee(king_loc[opponent]);
}

bool Board::Checked() {
	return players[opponent]->CanSee(king_loc[player]);
}

bool Board::CheckMate() {
	
	// can't be a checkmate if not a check
	if (!Check()) return false; 
	
	// not a checkmate if piece can be captured by king
	auto last_moved = move_path.top();
	// if (CanBeCaptured(last_moved, player)) return false;
	if (players[opponent]->CanCapture(last_moved)) return false;
	// if (pieces[king_loc[player]]->CanCapture(last_moved)) return false;
	
	// not a checkmate if king can escape
	const auto& oppo_king = pieces[king_loc[opponent]];
	for (const auto& move : *oppo_king) {
		if (oppo_king->CanMove(move)) { // king can escape here?
			
			return false; 

			ApplyMove(players[opponent]->ParseCommand(king_loc[opponent], move));
			
			bool escaped = !players[player]->CanCapture(move);
			
			Undo();
			
			if (escaped) return false;
		}
	}
	// not a checkmate if friendly pieces can block the attack
	char name = pieces[last_moved]->Name();
	if (toupper(name) == ROOK || toupper(name) == QUEEN || toupper(name) == BISHOP) {
		int cdir = king_loc[opponent][0] - last_moved[0];
		if (cdir != 0) cdir /= abs(cdir);
		int rdir = king_loc[opponent][1] - last_moved[1];
		if (rdir != 0) rdir /= abs(rdir);
		std::string block = last_moved;
		block[0] += cdir;
		block[1] += rdir;
		while (block != king_loc[opponent]) {
			if (Distance(block, king_loc[opponent]) > 1 && players[opponent]->CanSee(block) > 0) return false;
			if (Distance(block, king_loc[opponent]) == 1 && players[opponent]->CanSee(block) > 1) return false;
			block[0] += cdir;
			block[1] += rdir;
		}
	}
	return true;
}

bool Board::StaleMate() {
	for (const auto& [loc, piece] : pieces) {
		if (piece != BLANK && piece->Player() == opponent && !pieces[loc]->IsKing()) return false;
	}
	const auto& opponent_king = pieces[king_loc[opponent]];
	for (const auto move : *opponent_king) {
		if (move != king_loc[opponent] && !players[player]->CanSee(move)) return false;
	}
	return true;
}

int Board::BoardScore() {
	int score = 0;
	for (char r = TOP_ROW; r >= BOT_ROW; --r) {
		for (char c = LEFT_COL; c <= RIGHT_COL; ++c) {
			std::string loc = std::string() + c + r;
			int advantage = players[WHITE]->CanSee(loc) - players[BLACK]->CanSee(loc);
			int location_weight = (std::min(abs(r - TopRow()), abs(r - BotRow())) + std::min(abs(c - LeftCol()), abs(c - RightCol())));
			if (pieces[loc] != nullptr) {
				if (pieces[loc]->Player() == WHITE) score += pieces[loc]->Priority();
				else score -= pieces[loc]->Priority();
			} else {
				if (player == WHITE && advantage <= 0) --advantage;
				if (player == BLACK && advantage >= 0) ++advantage;
				score += advantage * location_weight;
			}
		}
	}
	if (player == WHITE) {
		if (Check()) score += CHECK_SCORE;
		if (CheckMate()) score = MAX_SCORE;
		if (StaleMate()) score += STALE_SCORE;
	} else {
		if (Check()) score -= CHECK_SCORE;
		if (CheckMate()) score = -MAX_SCORE;
		if (StaleMate()) score -= STALE_SCORE;
	}
	return score;
}