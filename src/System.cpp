#include "System.hpp"
#include "GameState.hpp"
#include "ScriptingVariables.hpp"

void System::fill_ObjectMap(Object* o, ObjectMap& om)
{
	om.handle = o->get_handle();
	om.visible = &(o->visible);
	om.visible_in_short_description = &(o->visible_in_short_description);
	om.friendly = &(o->friendly);
	om.mobile = &(o->mobile);
	om.open = &(o->open);
	om.holdable = &(o->holdable);
	om.hitpoints = &(o->hitpoints);
	om.hit_chance = &(o->hit_chance);
	om.description = &(o->description);
	om.name = &(o->name);
}

void System::fill_scripting_variables(GameState& gs, ScriptingVariables& sv, Room* current_room, Object* caller)
{
	//grab some variables first
	Object* player = gs.level->get_object(gs.playable_character);
	Room* cr = gs.level->get_room(player->room_container);
	
	//fill in the global variable
	sv.main_text = &(gs.main_text);
	
	//fill in the current room
	sv.current_room.handle = cr->get_handle();
	sv.current_room.description = new std::string(cr->get_description());
	sv.current_room.short_description = new std::string(cr->get_short_description());
	sv.current_room.minimap_symbol = new std::string(cr->get_minimap_symbol());
	sv.current_room.visited = new bool(cr->get_visited());
	sv.current_room.open_n = new bool(cr->get_exit(Room::Exit::NORTH));
	sv.current_room.open_e = new bool(cr->get_exit(Room::Exit::EAST));
	sv.current_room.open_s = new bool(cr->get_exit(Room::Exit::SOUTH));
	sv.current_room.open_w = new bool(cr->get_exit(Room::Exit::WEST));
	
	//fill in the two global objects
	fill_ObjectMap(player,sv.player);
	fill_ObjectMap(caller,sv.caller);
	
	//fill in the list of objects in the room
	auto &os = cr->objects();
	for (unsigned i = 0; i < os.size(); ++i)
	{
		//get the object
		Object* o = gs.level->get_object(os[i]);
		ObjectMap om;
	
		//fill in the object map
		fill_ObjectMap(o,om);
		
		//put this object map into the list
		sv.current_room.objects.push_back(om);
	}
}

void System::unfill_scripting_variables(GameState& gs, ScriptingVariables& sv, Room* current_room)
{
	//the room variables need to be fed back into the room class and then deleted
	Room* cr = gs.level->get_room(gs.level->get_object(gs.playable_character)->room_container);
	cr->set_description(*(sv.current_room.description));
	cr->set_short_description(*(sv.current_room.short_description));
	cr->set_minimap_symbol(*(sv.current_room.minimap_symbol));
	cr->set_visited(*(sv.current_room.visited));
	cr->set_exit(Room::Exit::NORTH, *(sv.current_room.open_n));
	cr->set_exit(Room::Exit::EAST, *(sv.current_room.open_e));
	cr->set_exit(Room::Exit::SOUTH, *(sv.current_room.open_s));
	cr->set_exit(Room::Exit::WEST, *(sv.current_room.open_w));
	
	delete sv.current_room.description;
	delete sv.current_room.short_description;
	delete sv.current_room.minimap_symbol;
	delete sv.current_room.visited;
	delete sv.current_room.open_n;
	delete sv.current_room.open_e;
	delete sv.current_room.open_s;
	delete sv.current_room.open_w;
}
