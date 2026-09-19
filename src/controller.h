#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <memory>
#include <iostream>
#include <stack>

class Board;

class Controller {
	Board* board;

	public:
		Controller(Board* board) : board{board} {}
		~Controller();

		void StartGame(); // start game flow
		void RunGame(); // run a match until ctrl + D
		void Setup(); // setup mode

		void GameOver();
};

#endif