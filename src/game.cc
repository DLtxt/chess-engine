#include "game.h"
#include "text_ui.h"
#include "graphics_ui.h"
#include "controller.h"
#include <iostream>

void Game::Start(Board* game_board) {
	TextUI ui{game_board};
	GraphicsUI gui{game_board};
	Controller controller{game_board};
	controller.StartGame();
}
