#include "game.h"
#include "chessboard.h"
#include "shogi/shogi_board.h"
using namespace std;

int main(int argc, char** argv) {
	bool graphics = true;
	for (int i = 1; i < argc; ++i) {
		const string arg = argv[i];
		if (arg == "--no-graphics" || arg == "-n") graphics = false;
		else if (arg == "--help" || arg == "-h") {
			cout << "usage: chess [--no-graphics]\n"
			     << "  --no-graphics, -n   text board only, no X11 window" << endl;
			return 0;
		}
	}

	Game game;
	string board_game;
	cout << "Type \"chess\" to enter chess mode, type \"shogi\" to enter shogi mode" << endl;
	cin >> board_game;
	if (board_game.compare("chess") == 0) {
		ChessBoard board;
		game.Start(&board, graphics);
	} else
	if (board_game.compare("shogi") == 0) {
		ShogiBoard board;
		game.Start(&board, graphics);
	} else {
		cout << "Please choose one of chess or shogi to play!" << endl;
	}
}