#ifndef PARSER_H
#define PARSER_H

#include "board.h"
#include <memory>

class AbstractMove;

class Parser {
	public:
		std::unique_ptr<AbstractMove> ParseCommand(Board*, const std::string&, const std::string&, char promotion = QUEEN) noexcept;
};

#endif