# Chess Engine

A command-line chess engine written in C++20, with a second game mode for **Shogi** (Japanese chess). Built as a CS246 term project.

The engine renders to two displays at once — an ANSI-coloured board in your terminal and an X11 graphics window — kept in sync through an Observer pattern. It supports full chess rules (castling, en passant, pawn promotion, check/checkmate/stalemate detection), unlimited undo, a free-form board setup mode, and five levels of computer opponent.

Levels 1–4 are the original heuristic players, from random moves up to a depth-3 alpha-beta search over the object graph. **Level 5 is a separate bitboard search engine** living in `src/engine`: iterative deepening, principal variation search with null-move pruning and late move reductions, a transposition table, quiescence search with static exchange evaluation, and a tapered evaluation with piece-square tables, pawn structure and king safety. It also builds as a **standalone UCI engine** that runs headless and plays in any chess GUI.

---

## Requirements

| Requirement | Notes |
|---|---|
| **C++20 compiler** | `g++`, or Apple `clang++` aliased as `g++` |
| **X11 development headers and libraries** | The Makefile compiles with `-I/usr/X11/include` and links `-L/usr/X11/lib -lX11` |
| **A running X display** | Needed by the `chess` game binary only — the `engine` binary is headless |
| **GNU Make** | |

### The X display, and how to avoid needing one

The `engine` binary has no X11 dependency at all — build and run it anywhere. The `chess` game binary does: `GraphicsUI` opens a window in its constructor and calls `exit(1)` if `XOpenDisplay` returns null:

```
Cannot open display
```

The graphics window is created unconditionally, so `./chess` needs a display. `./engine` does not.

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

This produces **two** binaries:

| Binary | What it is | Needs X11 |
|---|---|---|
| `chess` | The interactive game: both UIs, chess and shogi, players 1–5 | Yes |
| `engine` | The standalone search engine: UCI, perft, bench, self-play | No |

Build just one with `make chess` or `make engine`. Other targets:

```bash
make test      # run the perft move-generator suite
make bench     # fixed-depth benchmark, for comparing builds
make debug     # -O0 with address and undefined-behaviour sanitizers
make clean     # remove build output
make help      # list every target
```

The build is `-O2` by default. Optimisation matters more than usual here: search speed is strength, and an `-O0` build searches several times shallower in the same time.

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
| `computer5` | Level 5 — the bitboard search engine |

For example, to play White against the strongest engine:

```
chess
game human computer5
```

Or to watch it play itself:

```
chess
game computer5 computer5
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

Levels 1–4 form a **fall-through chain** rather than four independent engines. `ComputerPlayer::MakeMove` tries the highest enabled level first; each strategy throws when it has no opinion about the position, and control drops to the next simpler one. Level 1 always produces a move.

| Level | Strategy |
|---|---|
| **1** | Random. Picks a random piece and a random legal move whose destination the opponent cannot capture. |
| **2** | Greedy one-ply, preferring **checkmate > capture > check**. Captures maximise the value taken, tie-broken toward the cheapest attacker. |
| **3** | Defensive. Finds the most valuable piece under attack and either moves it to safety or finds another piece that can block or capture the attacker. |
| **4** | Alpha-beta minimax at depth 3 over the object graph, using the original `Board::BoardScore` evaluation. |
| **5** | The bitboard search engine described below. It does not fall through — the search always returns a legal move. |

Level 5 converts the live board to a FEN string, hands it to the engine, and plays the move that comes back. Because the engine is 8×8 chess only, a level-5 player in Shogi falls through to level 4.

---

## The search engine

`src/engine` is a self-contained engine that shares no code with the game's object model. It represents the board as bitboards — one 64-bit word per piece type per colour — so a square lookup is a bit test rather than a red-black tree descent through string keys.

### Search

Negamax with alpha-beta, wrapped in iterative deepening and aspiration windows:

- **Quiescence search** at the horizon, extending only captures and promotions until the position is quiet, with static exchange evaluation and delta pruning to discard losing captures. This is what stops the engine from walking into a recapture one ply past its horizon.
- **Move ordering** — transposition-table move, then captures scored by MVV-LVA and split by SEE into winning and losing, then killer moves, counter moves, and a history heuristic. Ordering is what makes alpha-beta approach its theoretical best case.
- **Transposition table** with Zobrist hashing, four-entry buckets, depth-and-age replacement, and mate scores stored relative to the node that found them.
- **Pruning and reductions** — null-move pruning, reverse futility, late move pruning, late move reductions, SEE pruning of bad captures, mate-distance pruning, and principal variation search.
- **Check extensions**, so forcing lines are searched deeper than quiet ones.
- **Time management** that budgets from the clock and increment, and refuses to start an iteration it cannot finish.

### Evaluation

A **tapered** evaluation: every term is computed twice, once for the middlegame and once for the endgame, then blended by how much material is left on the board. A rook is worth more in an endgame, and a king wants the corner in the middlegame and the centre in the endgame.

- Material and **piece-square tables** for every piece, in both phases.
- **Pawn structure** — passed pawns scaled by rank, isolated, doubled, backward and connected pawns — cached in a dedicated pawn hash table keyed on pawn positions alone.
- **King safety** — a pawn shield term, and a penalty that grows with the number and weight of enemy pieces attacking the king's zone.
- **Mobility** per piece, counting only squares not controlled by enemy pawns.
- Bishop pair, rooks on open and semi-open files, rooks on the seventh.

### Draw detection

Threefold repetition (walking back through the position keys to the last irreversible move), the fifty-move rule, and insufficient material. Checkmate and stalemate come from a single test: the side to move has no legal reply, and the only question is whether it is in check.

---

## Using the engine directly

The `engine` binary speaks **UCI**, so it plays in any standard GUI — Arena, CuteChess, BanksiaGUI — and can be matched against other engines.

```bash
./engine                       # UCI on stdin/stdout
```

```
uci
position startpos moves e2e4 e7e5
go movetime 3000
```

It also accepts several non-standard commands, useful on their own:

| Command | Effect |
|---|---|
| `./engine perfttest` | Run the move generator against published node counts |
| `./engine perft <fen> <depth>` | Per-move node breakdown for one position |
| `./engine bench [depth]` | Fixed workload across six positions — node count and nps |
| `d` | Print the current board and its FEN |
| `eval` | Static evaluation of the current position |

### Verifying the move generator

**Run this first.** `perft` counts leaf nodes of the legal move tree, and the expected counts are published, so any disagreement localises a bug precisely rather than leaving you guessing why the engine plays badly:

```bash
make test
```

The suite covers the six standard positions, which between them exercise castling, en passant, promotion, pins, discovered check and double check.

### Measuring whether a change helped

Chess engine changes cannot be evaluated by eye — a tweak that looks obviously better often loses. The engine plays matches against itself to settle it:

```bash
./engine selfplay -games 200 -deptha 6 -depthb 5
```

Each game starts from a few random plies so the match is not one game repeated, colours alternate, and hopeless positions are adjudicated. The report gives the record, the score rate, an Elo estimate with a 95% confidence interval, and the likelihood that A is genuinely stronger:

```
games   200
record  +138 =31 -31
score   76.8%
elo     +209.3   [176.4, 244.1] 95%
LOS     100.0%
verdict A is stronger.
```

Use `-depth` to set both sides equally, or `-movetime` to compare at a fixed time per move instead of a fixed depth.

---

## Running the game test scripts

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
  board.{h,cc}            Core board: piece map, move stack, check/mate detection
  chessboard.h            8x8 chess board
  controller.cc           Top-level command loop, setup mode
  game.cc                 Wires up the board, both UIs, and the controller
  text_ui.cc              ANSI-coloured terminal board
  graphics_ui.cc          X11 window
  subject.cc, observer.h  Observer pattern connecting board to UIs

  pieces/                 Piece hierarchy
    iterators/            SlideIterator (Q/R/B), JumpIterator (N/K), PawnIterator

  moves/                  Command pattern: Move, KingMove, Promotion, Drop

  player/
    human_player.cc       Parses interactive commands
    computer_player.cc    The level fall-through chain
    computer_level_1..4   The original heuristic strategies
    computer_level_5.cc   Bridges the live board to the search engine via FEN
    vision.{h,cc}         Per-player attack map used by levels 1-4

  engine/                 The search engine -- shares no code with the above
    types.h               Squares, pieces, 16-bit packed moves, score constants
    bitboard.{h,cc}       Bitboard primitives and attack tables
    zobrist.{h,cc}        Hash keys
    position.{h,cc}       Board state, make/unmake, legality, SEE, draw rules
    movegen.{h,cc}        Legal move generation
    eval.{h,cc}           Tapered evaluation
    tt.{h,cc}             Transposition table
    search.{h,cc}         Iterative deepening, PVS, quiescence, pruning
    perft.{h,cc}          Move generator verification
    uci.{h,cc}            UCI protocol and bench
    selfplay.{h,cc}       Self-play match runner with Elo and LOS reporting

  shogi/                  9x9 board, Shogi pieces, drop logic

test/                     48 scripted games as stdin transcripts
```

The two engines generate moves in deliberately different ways.

The **game's object model** exposes a piece's candidate squares through a custom iterator, so they read as a range-for:

```cpp
for (const auto& to : *piece) {
    if (piece->CanMove(to)) { /* ... */ }
}
```

Levels 1–4 search by mutating the live board: `Board::ApplyMove` pushes an `AbstractMove` command onto an undo stack and `Board::Undo` pops and reverses it.

The **search engine** never touches that object graph. It works on a packed `Position` — twelve bitboards, a 64-entry mailbox, and an incrementally updated Zobrist key — where a move is a single 16-bit integer and make/unmake restores the previous state wholesale from a saved `StateInfo`:

```cpp
pos.do_move(m);
score = -negamax(pos, depth - 1, -beta, -alpha, ply + 1, is_pv);
pos.undo_move(m);
```

That difference is the whole point. The object model allocates, chases pointers, and rebuilds a full-board attack map after every move; the bitboard position does none of it, which is what lets the same search algorithm run orders of magnitude deeper in the same time.
