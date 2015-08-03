#pragma once
#include <vector>
#include <string>
#include "Handle.hpp"

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

struct ScriptingVariables
{
	RoomMap current_room;
	ObjectMap caller;
	ObjectMap player;
	ObjectMap object_iterator;
	std::string* main_text;
};
