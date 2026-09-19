#ifndef GRAPHICS_UI_H
#define GRAPHICS_UI_H

#include "observer.h"
#include "board.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <memory>

class GraphicsUI : public Observer {
	Board* board;
	Display* display;
	Window window;
	GC gc;
	int screen;
	unsigned int width;
	unsigned int height;
	unsigned int cell_size, square_size;

	void DrawBoard();
	void DrawPieces();
	void DrawPiece(char piece, char player, int x, int y);

	std::map<std::string, char> board_player, board_name;
	std::map<std::string, bool> capture_status, move_status;
	
	bool board_is_drawn;
	// False when there is no display, or graphics were switched off. The UI
	// then attaches to nothing and every entry point is a no-op.
	bool active;
	
	public:
  
	  GraphicsUI(Board* board, bool enabled = true);
    ~GraphicsUI();
    void Notify() override;
};

#endif