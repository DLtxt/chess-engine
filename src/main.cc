#include "game.h"
#include "chessboard.h"
#include "shogi/shogi_board.h"
using namespace std;

int main() {
	Game game;
	string board_game;
	cout << "Type \"chess\" to enter chess mode, type \"shogi\" to enter shogi mode" << endl;
	cin >> board_game;
	if (board_game.compare("chess") == 0) {
		ChessBoard board;
		game.Start(&board);
	} else
	if (board_game.compare("shogi") == 0) {
		ShogiBoard board;
		game.Start(&board);
	} else {
		cout << "Please choose one of chess or shogi to play!" << endl;
	}
}