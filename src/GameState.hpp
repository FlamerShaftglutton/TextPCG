#pragma once
#include <string>
#include "Level.hpp"

struct GameState
{
	int frames_elapsed;
	int menu_index;
	bool menu_transition;
	Level* level;
	
	std::string main_text;
	bool main_text_dirty_flag;
};
