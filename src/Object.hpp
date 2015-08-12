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
	
	void copy(Object& o)
	{
		visible = o.visible;
		visible_in_short_description = o.visible_in_short_description;
		friendly = o.friendly;
		mobile = o.mobile;
		playable = o.playable;
		open = o.open;
		holdable = o.holdable;
		object_container = o.object_container;
		room_container = o.room_container;
		hitpoints = o.hitpoints;
		attack = o.attack;
		hit_chance = o.hit_chance;
		objects = o.objects;
		description = o.description;
		name = o.name;
		scripts = o.scripts;
	}
	
	inline ECS::Handle get_handle() { return handle; }

	bool visible;
	bool visible_in_short_description;
	bool friendly;
	bool mobile;
	bool playable;
	bool open;
	bool holdable;
	
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
