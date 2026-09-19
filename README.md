# Chess Engine

A command-line chess engine written in C++20, with a second game mode for **Shogi** (Japanese chess). Built as a CS246 term project.

The engine renders to two displays at once — an ANSI-coloured board in your terminal and an X11 graphics window — kept in sync through an Observer pattern. It supports full chess rules (castling, en passant, pawn promotion, check/checkmate/stalemate detection), unlimited undo, a free-form board setup mode, and four levels of computer opponent ranging from random play to depth-3 alpha-beta search.

---

## Requirements

| Requirement | Notes |
|---|---|
| **C++20 compiler** | `g++`, or Apple `clang++` aliased as `g++` |
| **X11 development headers and libraries** | The Makefile compiles with `-I/usr/X11/include` and links `-L/usr/X11/lib -lX11` |
| **A running X display** | **Mandatory at runtime**, not just at build time — see below |
| **GNU Make** | |

### The X display is not optional

`GraphicsUI` opens a window in its constructor and calls `exit(1)` if `XOpenDisplay` returns null:

```
Cannot open display
```

The graphics window is created unconditionally on startup, so the program **cannot run headless** as written. Make sure a display is available before launching.

**macOS** — install [XQuartz](https://www.xquartz.org/) and launch it before running:

```bash
brew install --cask xquartz
open -a XQuartz
```

**Debian / Ubuntu** — install the X11 headers; any normal desktop session provides the display:

```bash
sudo apt install build-essential libx11-dev
```

**Headless Linux / CI** — wrap the run in a virtual framebuffer:

```bash
sudo apt install xvfb
xvfb-run ./chess
```

If the X11 headers live somewhere other than `/usr/X11` on your system (common on Linux, where they are usually under `/usr/include/X11`), adjust `CXXFLAGS` and the link line in the `Makefile` accordingly.

---

## Building

```bash
make
```

This compiles every source file listed in the Makefile's `FILE_LIST` into `build/` and links the executable `chess` in the project root. Object files and dependency files are written to `build/`, which the Makefile creates automatically.

To remove build output:

```bash
make clean
```

---

## Running

```bash
./chess
```

The program starts by asking which game to play. Type exactly one of:

```
chess
shogi
```

You then land at the top-level prompt, which accepts two commands:

```
setup                      Enter board setup mode
game [player1] [player2]   Start a match
```

`player1` plays **White** and moves first; `player2` plays **Black**. Each may be:

| Argument | Opponent |
|---|---|
| `human` | Interactive player |
| `computer1` | Level 1 |
| `computer2` | Level 2 |
| `computer3` | Level 3 |
| `computer4` | Level 4 |

For example, to play White against the strongest engine:

```
chess
game human computer4
```

Or to watch the engine play itself:

```
chess
game computer4 computer4
```

When a game ends, the score is printed and the board resets, returning you to the top-level prompt so you can start another match. Scores accumulate across games in a session: a win is worth 1 point, a draw or stalemate 0.5 to each side. `Ctrl-D` exits and prints the final score.

---

## Game commands

### Human player

| Command | Effect |
|---|---|
| `move <from> <to>` | Move a piece, e.g. `move e2 e4` |
| `move <from> <to> <piece>` | Pawn promotion — the fourth argument is required when a pawn reaches the last rank. Use `Q`, `R`, `B`, or `N`; king and pawn are rejected. Example: `move e7 e8 Q` |
| `resign` | Concede the game; the opponent scores a point |
| `draw` | End the game in a draw, half a point each |
| `undo` | Take back the previous move. Can be repeated back to the start of the game |

Castling is expressed as a two-square king move — `move e1 g1` for kingside. En passant is expressed as the normal diagonal pawn capture. Both are detected automatically.

### Computer player

**The engine does not move on its own.** `ComputerPlayer::TakeAction` reads a line from standard input exactly like a human player does, so on the computer's turn you type:

```
move
```

with no arguments, and the engine then chooses and plays its move. `resign`, `draw`, and `undo` also work on the computer's turn.

This is what makes the engine-vs-engine test files work — they are just a sequence of bare `move` lines.

### Shogi mode

Shogi uses a 9×9 board with files `a`–`i` and ranks `1`–`9`. In addition to the commands above:

| Command | Effect |
|---|---|
| `drop <square> <piece>` | Drop a captured piece back onto the board, e.g. `drop e5 P` |
| `promote <from> <to>` | Move and promote in one command |

Piece letters: `K` king, `R` rook, `B` bishop, `G` gold, `S` silver, `N` knight, `L` lance, `P` pawn, and the promoted `D` dragon and `H` horse.

---

## Setup mode

`setup` clears the board and lets you build an arbitrary position:

| Command | Effect |
|---|---|
| `+ <piece> <square>` | Place a piece. **Uppercase is White, lowercase is Black.** `+ K e1` places a white king; `+ k e8` places a black one |
| `- <square>` | Remove whatever is on that square |
| `= <colour>` | Set which side moves first, e.g. `= white` or `= black` |
| `done` | Validate and leave setup mode |

`done` refuses to exit unless the position is legal: exactly one king per side, and no pawn on the first or last rank. Attempting to place a second king of the same colour is rejected at the moment you type it.

---

## Computer opponents

The four levels form a **fall-through chain** rather than four independent engines. `ComputerPlayer::MakeMove` tries the highest enabled level first; each strategy throws when it has no opinion about the position, and control drops to the next simpler one. Level 1 always produces a move, and if even that fails the engine resigns.

| Level | Strategy |
|---|---|
| **1** | Random. Picks a random piece and a random legal move whose destination the opponent cannot capture. |
| **2** | Greedy one-ply. Tries every legal move and prefers **checkmate > capture > check**. Captures maximise the value taken, tie-broken toward using the cheapest attacker, and are only accepted if the capture outvalues the moving piece or the destination is undefended. |
| **3** | Defensive. Finds the most valuable piece currently under attack and either moves it to safety or finds another piece that can block or capture the attacker — accepting the rescue only if the rescuer ends up safe, or is cheaper than what it saves. |
| **4** | **Alpha-beta minimax at depth 3.** Falls back to the lower levels for its first move. |

Because the levels chain, `computer4` is really "level 4, else level 3, else level 2, else level 1" — a level-4 opponent still plays a level-2 capture when the search declines to commit.

### Evaluation function

`Board::BoardScore` scores a position from White's perspective:

- **Material**, White-positive and Black-negative: pawn 100, knight 200, bishop 300, rook 500, queen 1000, king 2000.
- **Square control** — for every *empty* square, `(white attackers − black attackers) × location_weight`, where the weight rewards distance from the edge. Central control is worth more than control of the rim. A ±1 nudge favours the side to move.
- **±100** for check, **±100** for stalemate, **±10,000,000** for checkmate.

Attacker counts come from `Vision`, a per-player attack map rebuilt after every move and undo. `Vision::CanSee(square)` returns *how many* of that player's pieces attack the square, which is also how check is detected — a check is simply the opponent's king square being visible to the side that just moved.

---

## Running the tests

`test/` contains 48 scripted games as plain stdin transcripts. Feed one to the executable:

```bash
xvfb-run ./chess < test/ai4-1.in     # headless
./chess < test/ai4-1.in              # with a display already running
```

**The chess test files are missing their leading mode line.** They were written before the `chess` / `shogi` prompt was added and begin directly with `game ...`, so the mode prompt eats the first line and the program exits. Prepend the mode when running them:

```bash
{ echo chess; cat test/ai4-1.in; } | ./chess
```

The Shogi tests (`drop.in`, `shogi_king.in`, `shogi-promotion.in`) already start with `shogi` and can be piped in directly.

---

## Project layout

```
src/
  board.{h,cc}            Core board: piece map, move stack, check/mate detection, evaluation
  chessboard.h            8x8 chess board
  controller.cc           Top-level command loop, setup mode
  game.cc                 Wires up the board, both UIs, and the controller
  text_ui.cc              ANSI-coloured terminal board
  graphics_ui.cc          X11 window
  subject.cc, observer.h  Observer pattern connecting board to UIs

  pieces/                 Piece hierarchy
    iterators/            Move generation: SlideIterator (Q/R/B), JumpIterator (N/K), PawnIterator

  moves/                  Command pattern: Move, KingMove (castling), Promotion, Drop
                          Each knows how to apply and undo itself

  player/
    human_player.cc       Parses interactive commands
    computer_player.cc    The level fall-through chain
    computer_level_1..4   The four strategies
    vision.{h,cc}         Per-player attack map

  shogi/                  9x9 board, Shogi pieces, drop logic

test/                     48 scripted games as stdin transcripts
```

Move generation is exposed through a custom iterator, so a piece's candidate squares are enumerated with a range-for:

```cpp
for (const auto& to : *piece) {
    if (piece->CanMove(to)) { /* ... */ }
}
```

Search relies on the `AbstractMove` command objects: `Board::ApplyMove` pushes a move onto an undo stack and `Board::Undo` pops and reverses it, which is what lets the alpha-beta search explore and unwind lines on the live board.
