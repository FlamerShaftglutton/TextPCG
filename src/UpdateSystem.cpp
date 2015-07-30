#include "UpdateSystem.hpp"
#include "Console.hpp"
#include "GameState.hpp"
#include <string>
#include "ScriptingVariables.hpp"

UpdateSystem::UpdateSystem(GameState& gs)
{
	current_room = -1;
	ScriptingVariables sv;
	
	for (ECS::Handle i = 0; i < (ECS::Handle)gs.level->get_num_objects(); ++i)
	{
		gs.level->get_object(i)->scripts.execute_on_creation(sv);
	}
}

void UpdateSystem::do_work(Console& console, GameState& gs)
{
	//first off, the menu will be the real defining factor here
	if (gs.menu_index < 0)//shouldn't ever happen, but better safe than sorry
	{
		return;
	}
	else if (gs.menu_index == 0)//main menu
	{
		if (gs.menu_transition)
		{
			gs.menu_transition = false;
			console.clear();
			gs.main_text = "Please enter the number of your choice then press <fg=red>ENTER<fg=white>.\n";
			gs.main_text += "1. New Game\n";
			gs.main_text += "2. Continue Game\n";
			gs.main_text += "3. Restart Game\n";
			gs.main_text += "4. Quit\n";
			gs.main_text_dirty_flag = true;
		}
	}
	else if (gs.menu_index == 1)//Game creation screen
	{
		if (gs.menu_transition)
		{
			gs.menu_transition = false;
			console.clear();
			//put some text into gs.main_text
		}
	}
	else if (gs.menu_index == 2)//the actual game
	{
		if (gs.menu_transition)
		{
			gs.menu_transition = false;
			console.clear();
			gs.main_text = "<fg=Red>Welcome! <fg=white>Type your command then press ENTER.\n";
			gs.main_text_dirty_flag = true;
		}
		else
		{
			//if we're transitioning to a new room...
			Object* player = gs.level->get_object(gs.playable_character);
			ECS::Handle new_current_room = player->room_container;
			if (new_current_room != current_room)
			{
				//update the current room variable
				current_room = new_current_room;
				
				//create a ScriptingVariables object to pass in
				ScriptingVariables sv;
				sv.main_text = &(gs.main_text);
				fill_ObjectMap(player,sv.player);
				std::string old_main_text = gs.main_text;
				
				//fill in the list of objects in the room
				auto &os = gs.level->get_room(current_room)->objects();
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
				
				//call the on_sight function for every object in the room
				for (unsigned i = 0; i < os.size(); ++i)
				{
					//get the object
					Object* o = gs.level->get_object(os[i]);
					
					//fill in the caller variable
					sv.caller = sv.current_room.objects[i];
					
					//call the on_sight script of this object
					o->scripts.execute_on_sight(sv);
				}
				
				//mark the dirty flag if anything changed the main_text variable
				if (old_main_text != gs.main_text)
				{
					gs.main_text_dirty_flag = true;
				}
			}
		}
	}
}

void UpdateSystem::fill_ObjectMap(Object* o, ObjectMap& om)
{
	om.visible = &(o->visible);
	om.visible_in_short_description = &(o->visible_in_short_description);
	om.friendly = &(o->friendly);
	om.mobile = &(o->mobile);
	om.hitpoints = &(o->hitpoints);
	om.hit_chance = &(o->hit_chance);
	om.description = &(o->description);
	om.name = &(o->name);
}