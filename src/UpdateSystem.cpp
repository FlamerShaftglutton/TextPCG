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
			gs.main_text = "";
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
			Room* cr = gs.level->get_room(new_current_room);
			if (new_current_room != current_room)
			{
				//update the current room variable
				current_room = new_current_room;
				
				//update the 'visited' flag on the room
				cr->set_visited(true);
				
				//destroy the combat information if it exists
				if (gs.combat_data != nullptr)
				{
					delete gs.combat_data;
				}
				
				//create a ScriptingVariables object to pass into the objects
				ScriptingVariables sv;
				std::string old_main_text = gs.main_text;
				fill_scripting_variables(gs, sv, cr, player);
				
				//call the on_sight function for every object in the room
				auto &os = cr->objects();
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
				
				//unfill the scripting variables
				unfill_scripting_variables(gs,sv,cr);
			}
			//continue combat if necessary
			else if (gs.combat_data != nullptr)
			{
				//call the attack script of the enemy
				ScriptingVariables sv;
				fill_scripting_variables(gs, sv, cr, player);
				
				gs.level->get_object(gs.combat_data->other)->scripts.execute_on_attack_step(sv);
				
				unfill_scripting_variables(gs, sv, cr);
				gs.main_text_dirty_flag = true;
			}
		}
	}
}
