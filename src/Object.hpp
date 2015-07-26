#pragma once

#include "Handle.hpp"
#include <string>
#include <vector>
#include "Scripting.hpp"

class Object
{
	ECS::Handle handle;

public:
	Object(ECS::Handle h) : handle(h) { }
	
	inline ECS::Handle get_handle() { return handle; }

	bool visible;
	bool visible_in_short_description;
	bool friendly;
	bool mobile;
	bool playable;
	
	ECS::Handle object_container;
	ECS::Handle room_container;
	
	int hitpoints;
	int attack;
	float hit_chance;
	
	std::vector<ECS::Handle> objects;
	
	std::string description;
	std::string name;
	
	ScriptSet scripts;
};
