#pragma once
#include <string>
#include "Level.hpp"

struct GameState
{
	int frames_elapsed;
	int menu_index;
	Level* level;
	
	std::string main_text;
	bool main_text_dirty_flag;
};