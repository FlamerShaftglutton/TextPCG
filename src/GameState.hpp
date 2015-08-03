#pragma once
#include <string>
#include "Level.hpp"
#include "combat_data.hpp"

struct GameState
{
	int frames_elapsed;
	int menu_index;
	bool menu_transition;
	ECS::Handle playable_character;
	Level* level;
	
	std::string main_text;
	bool main_text_dirty_flag;
	
	CombatData* combat_data;
};
