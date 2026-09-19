#ifndef TEXTUI_H
#define TEXTUI_H

#include "observer.h"
#include "board.h"

#define GREEN_ "42"
#define GREY_ "100"
#define BLACK_ "30"
#define WHITE_ "97"
#define DARK_BLUE_ "46"
#define LIGHT_BLUE_ "101"

class TextUI : public Observer {
	Board *board;
	public:
		TextUI(Board* board);
		void Notify() override;
};

#endif