#ifndef VISION_H
#define VISION_H

#include <string>
#include <map>
#include <memory>
#include <vector>

class Board;

class Vision {
	std::map<std::string, int> vision;
	public:
		void Refresh(Board*, char);
		int CanSee(const std::string&);
};

#endif