#include "player.h"
#include "parser.h"
#include "board.h"
#include "moves/move.h"
#include "vision.h"

Player::Player(Board* board, char player) : board{board}, player{player}, parser{std::make_unique<Parser>()}, vision{std::make_unique<Vision>()} {}

Player::~Player() {}

void Player::RefreshVision() {
	vision->Refresh(board, player);
}

int Player::CanSee(const std::string& loc) const {
	return vision->CanSee(loc);
}

bool Player::CanCapture(const std::string& to) const {
	for (const auto& piece : board->GetHand(player)) {
		if (piece->CanCapture(to)) return true;
	}
	return false;
}

std::unique_ptr<AbstractMove> Player::ParseCommand(const std::string& from, const std::string& to, char promotion) noexcept {
	return parser->ParseCommand(board, from, to, promotion);
}
