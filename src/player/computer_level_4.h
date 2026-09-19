#ifndef COMPUTER_LEVEL_4
#define COMPUTER_LEVEL_4

#include <memory>

class Board;
class ComputerPlayer;
class AbstractMove;

class ComputerLevel4 {

	Board *board;
	std::string best_from, best_move;
	char player, opponent;

	size_t START_LEVEL;

	int alpha_beta_max(ComputerPlayer* comp_player, size_t level, int alpha, int beta);
	int alpha_beta_min(ComputerPlayer* comp_player, size_t level, int alpha, int beta);

	int move_count;

	public:

		ComputerLevel4(Board* board) : board{board}, move_count{0} {}
	
		void MakeMove(ComputerPlayer* player);
};

#endif