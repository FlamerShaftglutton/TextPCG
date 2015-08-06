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
					gs.combat_data = nullptr;
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
				
				//finally, if there are any enemies they should start attacking
				if (gs.combat_data != nullptr)
				{
					delete gs.combat_data;
				}
				
				std::vector<ECS::Handle> enemies;
				for (unsigned i = 0; i < os.size(); ++i)
				{
					//get the object
					Object* o = gs.level->get_object(os[i]);
					
					if (o->mobile && !o->friendly && !o->playable)
					{
						enemies.push_back(o->get_handle());
					}
				}
				
				if (!enemies.empty())
				{
					gs.combat_data = new CombatData;
					gs.combat_data->player_position = CombatData::Position::far_front;
					gs.combat_data->player_attacking = false;
					gs.combat_data->enemy_queue.insert(gs.combat_data->enemy_queue.begin(),enemies.begin(),enemies.end());
					combat_tick = 0;
				}
			}
			//continue combat if necessary
			else if (gs.combat_data != nullptr && combat_tick++ == 9)//we only run this once every second or so
			{
				#ifdef DEBUG
					Log::write("Advancing to the next combat beat, calling the object script.");
				#endif
				
				combat_tick = 0;
			
				//call the attack script of the first (actually last) enemy
				ScriptingVariables sv;
				Object* o = gs.level->get_object(gs.combat_data->enemy_queue.back());
				fill_scripting_variables(gs, sv, cr, o);
				o->scripts.execute_on_attack_step(sv);
				
				#ifdef DEBUG
				{
					std::string names[] = {"left","right","front","far front"};
					std::string ll = gs.combat_data->enemy_vulnerable_sides[0] ? "left " : "";
					ll += gs.combat_data->enemy_vulnerable_sides[1] ? "right " : "";
					ll += gs.combat_data->enemy_vulnerable_sides[2] ? "front " : "";
					ll += gs.combat_data->enemy_vulnerable_sides[3] ? "far front" : "";
					Log::write("\tVulnerabilities: " + ll);
					
					Log::write("\tPlayer Pos: " + names[(unsigned)gs.combat_data->player_position]);
				}
				#endif
				
				unfill_scripting_variables(gs, sv, cr);
				gs.main_text_dirty_flag = true;
				
				#ifdef DEBUG
					Log::write("\tCombat script completed.");
				#endif 
				
				#ifdef DEBUG
				{
					std::string names[] = {"left","right","front","far front"};
					std::string ll = gs.combat_data->enemy_vulnerable_sides[0] ? "left " : "";
					ll += gs.combat_data->enemy_vulnerable_sides[1] ? "right " : "";
					ll += gs.combat_data->enemy_vulnerable_sides[2] ? "front " : "";
					ll += gs.combat_data->enemy_vulnerable_sides[3] ? "far front" : "";
					Log::write("\tVulnerabilities: " + ll);
					
					Log::write("\tPlayer Pos: " + names[(unsigned)gs.combat_data->player_position]);
				}
				#endif
				
				//now that the enemy did something, figure out what happened to the enemy
				if (gs.combat_data->player_attacking)
				{	
					//if we hit them and they were vulnerable on that side...
					if (gs.combat_data->enemy_vulnerable_sides[(unsigned)gs.combat_data->player_position])
					{
						//first, hit the enemy!
						o->hitpoints -= player->attack;
						
						//if the enemy died...
						if (o->hitpoints <= 0)
						{
							//display a message
							gs.main_text += "<fg=white><bg=black>\nYou killed <fg=red>" + o->name + "<fg=white>!";
							
							//make it drop all of its loot
							for (ECS::Handle oh : o->objects)
							{
								Object* oo = gs.level->get_object(oh);
								oo->object_container = -1;
								oo->room_container = current_room;
								cr->objects().push_back(oh);
							}
							
							//remove it from the room
							for (auto iter = cr->objects().begin(); iter != cr->objects().end(); ++iter)
							{
								if (*iter == o->get_handle())
								{
									cr->objects().erase(iter);
									break;
								}
							}
							
							//delete it from existence!
							gs.level->destroy_object(o->get_handle());
							o = nullptr;
							
							//remove it from the list of enemies!
							gs.combat_data->enemy_queue.pop_back();
							
							//finally, if we've killed everything in the room, destroy the combat data object
							if (gs.combat_data->enemy_queue.empty())
							{
								delete gs.combat_data;
								gs.combat_data = nullptr;
							}
							//if we haven't killed everything, set up the next one
							else
							{
								gs.combat_data->player_position = CombatData::Position::far_front;
							}
						}
					}
					//if the player hit an invulnerable side, just display a message
					else
					{
						gs.main_text += "<fg=white><bg=black>\nYour attack bounces off!";
					}
				}
				
				//figure out what happened to the player
				if (o != nullptr)
				{
					//if the enemy is attacking...
					if (gs.combat_data->enemy_attacking_sides[0] || gs.combat_data->enemy_attacking_sides[1] || gs.combat_data->enemy_attacking_sides[2] || gs.combat_data->enemy_attacking_sides[3])
					{
						//did they attack the player's position?
						if (gs.combat_data->enemy_attacking_sides[(unsigned)gs.combat_data->player_position])
						{
							//right now this just removes health from the player, but in the future things like invulnerability potions may change this
							player->hitpoints -= o->attack;
						}
						//if they missed then they should display text about that
					}
				}
				
				//reset the variables
				if (gs.combat_data != nullptr)
				{
					gs.combat_data->enemy_attacking_sides[0] =
					gs.combat_data->enemy_attacking_sides[1] =
					gs.combat_data->enemy_attacking_sides[2] =
					gs.combat_data->enemy_attacking_sides[3] = false;
					
					gs.combat_data->enemy_vulnerable_sides[0] = 
					gs.combat_data->enemy_vulnerable_sides[1] = 
					gs.combat_data->enemy_vulnerable_sides[2] = 
					gs.combat_data->enemy_vulnerable_sides[3] = true;
					
					gs.combat_data->player_attacking = false;
				}
			}
			
			//check if the player died from anything
			if (player->hitpoints < 0)
			{
				gs.main_text += "<fg=red><bg=black>\n\nYou died. Please quit and start a new game.";
			}
		}
	}
}
