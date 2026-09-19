#ifndef GAME_H
#define GAME_H

#include <string>

class Board;

class Game {
	std::string game_mode;

	public:
		void Start(Board* game_board);
};

#endif