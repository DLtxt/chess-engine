#include "graphics_ui.h"
#include <iostream>
#include <string>

#define WINDOW_WIDTH 648
#define WINDOW_HEIGHT 648
#define BOARD_SIZE 8
#define SQUARE_SIZE (WINDOW_WIDTH / BOARD_SIZE)

// Define colors for the light and dark squares
#define LIGHT_COLOR 0xeeeed2
#define DARK_COLOR 0x769656
#define WHITE_PIECE 0xFFCACA
#define BLACK_PIECE 0x4b4847
#define WHITE_CAPPED_PIECE 0xE97451
#define BLACK_CAPPED_PIECE 0x702963

#define PIECE_SIZE 6

const char *king_piece[] = {
	"K    K",
	"K   K ",
	"K K   ",
	"K  K  ",
	"K   K ",
	"K    K"
};

const char *queen_piece[] = {
	" QQQ  ",
	"Q    Q",
	"Q    Q",
	"Q    Q",
	" Q  Q ",
	"   QQQ"
};

const char *knight_piece[] = {
	"N    N",
	"NN   N",
	"N N  N",
	"N  N N",
	"N   NN",
	"N    N"
};

const char *bishop_piece[] = {
	"B BBB ",
	"B    B",
	"B BBB ",
	"B    B",
	"B    B",
	"B BB  "
};


const char *rook_piece[] = {
	"R RRR ",
	"R    R",
	"R   R ",
	"R RR  ",
	"R   R ",
	"R    R"
};

const char *pawn_piece[] = {
	"P PPP ",
	"P    P",
	"P    P",
	"P PP  ",
	"P     ",
	"P     "
};

const char *dragon_piece[] = {
  "D DD  ",
  "D    D",
  "D    D",
  "D    D",
  "D   D ",
  "D DD  "
};

const char *gold_piece[] = {
  "  GGG ",
  "G     ",
  "G     ",
  "G  GGG",
  "G    G",
  "  GGG "
};

const char *horse_piece[] = {
  "H    H",
  "H    H",
  "H    H",
  "H HH H",
  "H    H",
  "H    H"
};

const char *lance_piece[] = {
  "L     ",
  "L     ",
  "L     ",
  "L     ",
  "L     ",
  " LLLLL",
};

const char *silver_piece[] = {
  "  SSSS",
  "S     ",
  " SS   ",
  "    SS",
  "     S",
  "SSSS  "
};

GraphicsUI::GraphicsUI(Board* board) : board{board}, display{nullptr}, window{0}, gc{0}, board_is_drawn{false} {
  board->Attach(this);
  display = XOpenDisplay(nullptr);
  if (display == nullptr) {
    std::cerr << "Cannot open display\n";
    exit(1);
  }
  screen = DefaultScreen(display);
  width = WINDOW_WIDTH;
  height = WINDOW_HEIGHT;
  cell_size = width / board->BoardSize();
  square_size = WINDOW_WIDTH / board->BoardSize();

  window = XCreateSimpleWindow(display, RootWindow(display, screen), 10, 10, width, height, 1, BlackPixel(display, screen), WhitePixel(display, screen));
  XSelectInput(display, window, ExposureMask | KeyPressMask);
  XMapWindow(display, window);
  gc = DefaultGC(display, screen);

  XClearWindow(display, window);
  DrawBoard();
  XFlush(display);
}

GraphicsUI::~GraphicsUI() {
  XCloseDisplay(display);
}

void GraphicsUI::Notify() {
  DrawBoard();
  board_is_drawn = true;
  DrawPieces();
  XFlush(display);
}

void GraphicsUI::DrawBoard() {
  for (char r = board->TopRow(); r >= board->BotRow(); --r) {
    for (char c = board->LeftCol(); c <= board->RightCol(); ++c) {
      std::string loc = std::string() + c + r;
      int col = c - board->LeftCol();
      int row = board->TopRow() - r;
      int x = col * square_size;
      int y = row * square_size;

      char player = board->GetPiecePlayer(loc);

      // if (!board_is_drawn || (!was_empty[loc] && board->Empty(loc)) || board_player[loc] != board->GetPiecePlayer(loc) || board_name[loc] != board->GetPieceName(loc)) {
      if (!board_is_drawn || board_player[loc] != board->GetPiecePlayer(loc) || board_name[loc] != board->GetPieceName(loc) || capture_status[loc] != board->CanBeCaptured(loc, player) || move_status[loc] != board->LastMoved(loc)) {
        // Alternate colors for the squares
        if ((row + col) % 2 == 0) {
          XSetForeground(display, gc, LIGHT_COLOR);
          if (board->LastMoved(loc)) {
            XSetForeground(display, gc, LIGHT_COLOR);
          }
        } else {
          XSetForeground(display, gc, DARK_COLOR);
        }

        XFillRectangle(display, window, gc, x, y, square_size, square_size);
      }
    }
  }

}

void GraphicsUI::DrawPieces() {
  for (char r = board->TopRow(); r >= board->BotRow(); --r) {
    for (char c = board->LeftCol(); c <= board->RightCol(); ++c) {
      std::string loc = std::string() + c + r;
      char name = board->GetPieceName(loc);
      char player = board->GetPiecePlayer(loc);

      // if (name != EMPTY && (board_player[loc] != board->GetPiecePlayer(loc) || capture_status[loc] != board->CanBeCaptured(loc, player) || board_name[loc] != board->GetPieceName(loc))) {
      if (name != EMPTY && (board_player[loc] != board->GetPiecePlayer(loc) || board_name[loc] != board->GetPieceName(loc) || capture_status[loc] != board->CanBeCaptured(loc, player) || move_status[loc] != board->LastMoved(loc))) {
        DrawPiece(name, player, c - board->LeftCol(), board->TopRow() - r);
      }

      board_player[loc] = board->GetPiecePlayer(loc);
      board_name[loc] = board->GetPieceName(loc);
      capture_status[loc] = board->CanBeCaptured(loc, player);
      move_status[loc] = board->LastMoved(loc);
    }
  }
}

void GraphicsUI::DrawPiece(char piece, char player, int x, int y) {

  int piece_height = PIECE_SIZE;
  int piece_width = PIECE_SIZE;
  int x_offset = x * square_size + (square_size - piece_width * 10) / 2;
  int y_offset = y * square_size + (square_size - piece_height * 10) / 2;

  auto piece_display = king_piece;

  switch (toupper(piece)) {
    case KING:
      piece_display = king_piece;
      break;
    case QUEEN:
      piece_display = queen_piece;
      break;
    case BISHOP:
      piece_display = bishop_piece;
      break;
    case ROOK:
      piece_display = rook_piece;
      break;
    case PAWN:
      piece_display = pawn_piece;
      break;
    case KNIGHT:
      piece_display = knight_piece;
      break;
    case DRAGON:
      piece_display = dragon_piece;
      break;
    case GOLD:
      piece_display = gold_piece;
      break;
    case HORSE:
      piece_display = horse_piece;
      break;
    case LANCE:
      piece_display = lance_piece;
      break;
    case SILVER:
      piece_display = silver_piece;
      break;
    default:
      break;
  }

  if (player == BLACK) XSetForeground(display, gc, BLACK_PIECE);
  if (player == WHITE) XSetForeground(display, gc, WHITE_PIECE);
  char c = board->LeftCol() + x;
  char r = board->TopRow() - y;
  if (board->CanBeCaptured(std::string() + c + r, player)) {
    XSetForeground(display, gc, player == WHITE ? WHITE_CAPPED_PIECE : BLACK_CAPPED_PIECE);
  }

  for (int i = 0; i < piece_height; i++) {
    for (int j = 0; j < piece_width; j++) {
      if (piece_display[i][j] != ' ') {
        int x = x_offset + j * 10;
        int y = y_offset + i * 10;
        XFillRectangle(display, window, gc, x, y, 10, 10);
      }
    }
  }

}