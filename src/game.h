#ifndef GAME_H
#define GAME_H

#include <string>

class Board;

class Game {
	std::string game_mode;

	public:
		// `graphics` false skips the X11 window entirely.
		void Start(Board* game_board, bool graphics = true);
};

#endif