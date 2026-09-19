# Universal makefile for single C++ program
#
# Use gcc flag -MMD (user) or -MD (user/system) to generate dependencies amo    ng source files.
# Use make default rules for commonly used file-name suffixes and make varia    bles names.
#
# % make [ a.out ]

########## Variables ##########

CXX = g++          # compiler
CXXFLAGS = -std=c++20 -g -Wall -MMD -Isrc -I/usr/X11/include  # compiler flags
MAKEFILE_NAME = ${Makefile ${MAKEFILE_LIST}}  # makefile name
SORCE_DIR = src
BUILD_DIR = build
BUILD_DIRS = ${BUILD_DIR} ${BUILD_DIR}/pieces ${BUILD_DIR}/pieces/iterators ${BUILD_DIR}/moves ${BUILD_DIR}/player ${BUILD_DIR}/shogi ${BUILD_DIR}/shogi/move ${BUILD_DIR}/shogi/pieces
FILE_LIST = board.cc shogi/shogi_board.cc shogi/shogi_player.cc moves/drop.cc shogi/pieces/shogi_piece.cc shogi/pieces/knight.cc shogi/pieces/lance.cc shogi/pieces/pawn.cc shogi/shogi_player.cc moves/move.cc moves/king_move.cc moves/promotion.cc player/parser.cc player/vision.cc player/player.cc player/human_player.cc player/computer_player.cc player/computer_level_1.cc player/computer_level_2.cc player/computer_level_3.cc player/computer_level_4.cc pieces/piece.cc pieces/king.cc pieces/pawn.cc subject.cc pieces/iterators/slide_iterator.cc pieces/iterators/jump_iterator.cc pieces/iterators/pawn_iterator.cc text_ui.cc graphics_ui.cc controller.cc game.cc main.cc
SOURCES = ${addprefix ${SORCE_DIR}/, ${FILE_LIST}}     # source files (*.cc)
OBJECTS = ${addprefix ${BUILD_DIR}/, ${FILE_LIST:.cc=.o}}     # object files forming executable
DEPENDS = ${OBJECTS:.o=.d}      # substitute ".o" with ".d"
EXEC = chess # executable name

########## Targets ##########

.PHONY : clean          # not file names

$(shell mkdir -p $(BUILD_DIRS))

${EXEC} : ${OBJECTS}        # link step
	${CXX} $^ -o $@ -L/usr/X11/lib -lX11  # additional object files before $^

${BUILD_DIR}/%.o: ${SORCE_DIR}/%.cc
	${CXX} ${CXXFLAGS} -c -o $@ $<

${OBJECTS} : ${MAKEFILE_NAME}     # OPTIONAL : changes to this file => recom    pile

# make implicitly generates rules to compile C++ files that generate .o file    s

-include ${DEPENDS}       # include *.d files containing program dependences

clean :           # remove files that can be regenerated
	rm -f ${OBJECTS} ${DEPENDS}
