# Builds two binaries from one source tree:
#
#   chess    the interactive game: text + X11 UI, chess and shogi, players 1-5
#   engine   the standalone search engine: UCI, perft, bench, self-play
#
# The engine has no X11 dependency, so it builds and runs headless.

########## Variables ##########

CXX       = g++
OPT       = -O2
WARN      = -Wall -Wextra -Wno-unused-parameter
CXXFLAGS  = -std=c++20 $(OPT) $(WARN) -MMD -MP -Isrc
X11FLAGS  = -I/usr/X11/include
X11LIBS   = -L/usr/X11/lib -lX11

SRC_DIR   = src
BUILD_DIR = build

# Search engine: pure computation, no UI, no X11.
ENGINE_FILES = \
	engine/bitboard.cc \
	engine/zobrist.cc \
	engine/position.cc \
	engine/movegen.cc \
	engine/eval.cc \
	engine/tt.cc \
	engine/search.cc \
	engine/perft.cc \
	engine/uci.cc \
	engine/selfplay.cc \
	engine/engine.cc

# The interactive game, including the bridge that lets it call the engine.
GAME_FILES = \
	board.cc \
	shogi/shogi_board.cc \
	shogi/shogi_player.cc \
	moves/drop.cc \
	shogi/pieces/shogi_piece.cc \
	shogi/pieces/knight.cc \
	shogi/pieces/lance.cc \
	shogi/pieces/pawn.cc \
	moves/move.cc \
	moves/king_move.cc \
	moves/promotion.cc \
	player/parser.cc \
	player/vision.cc \
	player/player.cc \
	player/human_player.cc \
	player/computer_player.cc \
	player/computer_level_1.cc \
	player/computer_level_2.cc \
	player/computer_level_3.cc \
	player/computer_level_4.cc \
	player/computer_level_5.cc \
	pieces/piece.cc \
	pieces/king.cc \
	pieces/pawn.cc \
	subject.cc \
	pieces/iterators/slide_iterator.cc \
	pieces/iterators/jump_iterator.cc \
	pieces/iterators/pawn_iterator.cc \
	text_ui.cc \
	graphics_ui.cc \
	controller.cc \
	game.cc \
	main.cc

ENGINE_OBJS     = $(addprefix $(BUILD_DIR)/, $(ENGINE_FILES:.cc=.o))
ENGINE_MAIN_OBJ = $(BUILD_DIR)/engine/engine_main.o
GAME_OBJS       = $(addprefix $(BUILD_DIR)/, $(GAME_FILES:.cc=.o))

DEPENDS = $(ENGINE_OBJS:.o=.d) $(GAME_OBJS:.o=.d) $(ENGINE_MAIN_OBJ:.o=.d)

GAME_EXEC   = chess
ENGINE_EXEC = engine

########## Targets ##########

.PHONY: all clean test bench debug help

all: $(GAME_EXEC) $(ENGINE_EXEC)

# The game links the engine objects so that computer5 can call the search.
$(GAME_EXEC): $(GAME_OBJS) $(ENGINE_OBJS)
	$(CXX) $^ -o $@ $(X11LIBS) -pthread

$(ENGINE_EXEC): $(ENGINE_OBJS) $(ENGINE_MAIN_OBJ)
	$(CXX) $^ -o $@ -pthread

# Only the UI translation unit needs the X11 include path.
$(BUILD_DIR)/graphics_ui.o: $(SRC_DIR)/graphics_ui.cc
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(X11FLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Verify the move generator against published perft counts.
test: $(ENGINE_EXEC)
	./$(ENGINE_EXEC) perfttest

# Fixed workload, for comparing two builds' speed and node counts.
bench: $(ENGINE_EXEC)
	./$(ENGINE_EXEC) bench 8

# Unoptimised build with debug symbols and sanitizers.
debug:
	$(MAKE) OPT="-O0 -g -fsanitize=address,undefined" all

help:
	@echo "make            build both chess and engine"
	@echo "make chess      the interactive game (needs X11)"
	@echo "make engine     the standalone UCI engine (headless)"
	@echo "make test       run the perft move-generator suite"
	@echo "make bench      fixed-depth benchmark"
	@echo "make debug      -O0 with address and UB sanitizers"
	@echo "make clean      remove build output"

clean:
	rm -rf $(BUILD_DIR) $(GAME_EXEC) $(ENGINE_EXEC)

-include $(DEPENDS)
