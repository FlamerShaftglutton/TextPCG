#include "UpdateSystem.hpp"
#include "Console.hpp"
#include "GameState.hpp"
#include <string>
#include "Serialize.hpp"
#include "Level.hpp"
#include "Room.hpp"
#include "Object.hpp"
#include "Scripting.hpp"
#include "CombatData.hpp"

UpdateSystem::UpdateSystem(GameState& gs)
{
	current_room = -1;
	previous_menu = -1;
}

void UpdateSystem::do_work(Console& console, GameState& gs)
{
	int main_menu_index = console.get_frameset_by_name("main_menu");
	int new_game_index = console.get_frameset_by_name("new_game");
	int continue_game_index = console.get_frameset_by_name("continue_game");
	int in_game_index = console.get_frameset_by_name("in_game");
	int current_menu = console.get_current_frameset_index();
	bool menu_transition = current_menu != previous_menu;

	//first off, the menu will be the real defining factor here
	if (current_menu < 0)//shouldn't ever happen, but better safe than sorry
	{
		return;
	}
	else if (current_menu == main_menu_index)
	{
		if (menu_transition)
		{
			//if we just quit from the game, save it!
			if (previous_menu == in_game_index)
			{
				Serialize::to_file("savedgame.tsf", gs);
			}
		
			console.get_current_frameset().clear();
			gs.main_text = "Please enter the number of your choice then press <fg=red>ENTER<fg=white>.\n";
			gs.main_text += "1. New Game\n";
			gs.main_text += "2. Continue Game\n";
			gs.main_text += "3. Restart Game\n";
			gs.main_text += "4. Quit\n";
			gs.main_text_dirty_flag = true;
		}
	}
	else if (current_menu == new_game_index)
	{
		if (menu_transition)
		{
			//menu_transition = false;
			//console.clear();
			//gs.main_text = "Derp!";
			
			//fill this in later so that the user can create a whole new game
			Serialize::from_file("newgame.tsf", gs);
			console.switch_to_frameset(console.get_frameset_by_name("in_game"));
		}
	}
	else if (current_menu == continue_game_index)
	{
		//fill this in later so the user can pick a game
		//menu_transition = false;
		Serialize::from_file("savedgame.tsf",gs);
		console.switch_to_frameset(console.get_frameset_by_name("in_game"));
	}
	else if (current_menu == in_game_index)//the actual game
	{
		if (menu_transition)
		{
			console.get_current_frameset().clear();
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
				
				//call the on_sight function for every object in the room
				auto &os = cr->objects();
				for (unsigned i = 0; i < os.size(); ++i)
				{
					//get the object
					Object* o = gs.level->get_object(os[i]);
					
					//call the on_sight script of this object
					o->scripts.execute_on_sight(gs, os[i]);
				}
				
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
			else if (gs.combat_data != nullptr && combat_tick++ == 9)//we only run this once every two seconds or so
			{
				combat_tick = 0;
			
				//get the attacker
				Object* o = gs.level->get_object(gs.combat_data->enemy_queue.back());
				
				//reset the attacking/defending stuff
				gs.combat_data->vulnerable_to_attack = gs.combat_data->player_attacking && gs.combat_data->enemy_vulnerable_sides[(unsigned)gs.combat_data->player_position];
				
				gs.combat_data->enemy_attacking_sides[0] =
				gs.combat_data->enemy_attacking_sides[1] =
				gs.combat_data->enemy_attacking_sides[2] =
				gs.combat_data->enemy_attacking_sides[3] = false;
				
				gs.combat_data->enemy_vulnerable_sides[0] = 
				gs.combat_data->enemy_vulnerable_sides[1] = 
				gs.combat_data->enemy_vulnerable_sides[2] = 
				gs.combat_data->enemy_vulnerable_sides[3] = true;
				
				//call the attack script of the first (actually last) enemy
				o->scripts.execute_on_attack_step(gs, o->get_handle());

				gs.main_text_dirty_flag = true;
				
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
							if (!o->objects.empty())
							{
								gs.main_text += "<fg=white><bg=black>\nIt dropped the following loot:";
								for (ECS::Handle oh : o->objects)
								{
									Object* oo = gs.level->get_object(oh);
									oo->object_container = -1;
									oo->room_container = current_room;
									cr->objects().push_back(oh);
									
									gs.main_text += "<fg=white><bg=black>\n\t" + oo->name;
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
							
							//set the dirty flag
							gs.main_text_dirty_flag = true;
						}
					}
					//if the player hit an invulnerable side, just display a message
					else
					{
						gs.main_text += "<fg=white><bg=black>\nYour attack bounces off!";
						gs.main_text_dirty_flag = true;
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
					gs.combat_data->player_attacking = false;
				}
			}
			
			//call the level's cleanup function to destroy any objects marked for delayed deletion
			gs.level->cleanup_objects();
			
			//check if the player died from anything
			if (player->hitpoints < 0)
			{
				gs.main_text += "<fg=red><bg=black>\n\nYou died. Please quit and start a new game.";
				gs.main_text_dirty_flag = true;
			}
		}
	}
	
	previous_menu = current_menu;
}
