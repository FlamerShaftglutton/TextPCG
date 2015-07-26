#pragma once
#include <vector>
#include <string>

struct ObjectMap
{
	bool* visible;
	bool* visible_in_short_description;
	bool* friendly;
	bool* mobile;
	
	int* hitpoints;
	int* attack;
	float* hit_chance;
	
	std::string* description;
	std::string* name;
};

struct RoomMap
{
	std::vector<ObjectMap> objects;
	
	std::string* description;
	std::string* short_description;
	std::string* minimap_symbol;
};

struct ScriptingVariables
{
	RoomMap current_room;
	std::string* main_text;
};