#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <iostream>

struct _missing_white_king_ {
	_missing_white_king_() { std::cerr << "You need a white king on the board!\n"; }
};

struct _missing_black_king_ {
	_missing_black_king_() { std::cerr << "You need a black king on the board!\n"; }
};

struct _parsing_error_ {
	_parsing_error_(const std::string& message) { std::cerr << "Parsing syntax error. " << message << std::endl; }
};

struct _pawn_first_rank_ {
	_pawn_first_rank_() { std::cerr << "You can't put your pawn on first rank!\n"; }
};

struct _two_kings_ {
	_two_kings_() { std::cerr << "You can't have two kings!\n"; }
};

struct _end_of_line_ {};
struct _no_moves_found_ {};

struct _unknown_error_ {
	_unknown_error_(const std::string& message = "") { std::cerr << "Something is wrong. " << message << std::endl; }
};

struct _invalid_move_ {
	_invalid_move_(const std::string& message) { std::cerr << "Invalid move detected. " << message << std::endl; }
};

#endif