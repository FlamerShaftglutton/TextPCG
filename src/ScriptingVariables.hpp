#pragma once
#include <vector>
#include <string>
#include "Handle.hpp"
#include "combat_data.hpp"

struct ObjectMap
{
	ECS::Handle handle;
	
	bool* visible;
	bool* visible_in_short_description;
	bool* friendly;
	bool* mobile;
	bool* playable;
	bool* open;
	bool* holdable;
	
	int* hitpoints;
	int* attack;
	float* hit_chance;
	
	std::string* description;
	std::string* name;
};

struct RoomMap
{
	ECS::Handle handle;
	
	std::vector<ObjectMap> objects;
	
	std::string* description;
	std::string* short_description;
	std::string* minimap_symbol;
	
	bool* open_n;
	bool* open_e;
	bool* open_s;
	bool* open_w;
	
	bool* visited;
};

struct CombatMap
{
	bool active;
	
	CombatData::Position* player_position;
	
	bool* player_attacking;
	
	bool* enemy_vulnerable_sides;
	bool* enemy_attacking_sides;
};

struct ScriptingVariables
{
	CombatMap combat;
	RoomMap current_room;
	ObjectMap caller;
	ObjectMap player;
	ObjectMap object_iterator;
	std::string* main_text;
};
