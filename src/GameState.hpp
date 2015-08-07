#pragma once
#include <string>
#include "Level.hpp"
#include "combat_data.hpp"
#include "UIConstants.hpp"

struct GameState
{
	int frames_elapsed;
	UI_State menu_index;
	bool menu_transition;
	ECS::Handle playable_character;
	Level* level = nullptr;
	
	std::string main_text;
	bool main_text_dirty_flag;
	
	CombatData* combat_data = nullptr;
};
