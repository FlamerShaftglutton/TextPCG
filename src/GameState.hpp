#pragma once
#include <string>
#include "Handle.hpp"

//forward declarations
class Level;
struct CombatData;

struct GameState
{
	int frames_elapsed;
	ECS::Handle playable_character;
	Level* level = nullptr;
	
	std::string main_text;
	bool main_text_dirty_flag;
	
	CombatData* combat_data = nullptr;
};
