#include "InputSystem.hpp"
#include "Console.hpp"
#include "GameState.hpp"
#include "string_utils.hpp"
#include <string>

#ifdef DEBUG
	#include "Log.hpp"
#endif

void InputSystem::do_work(Console& console, GameState& gs)
{
	std::string input = StringUtils::trim(console.check_for_input());
	std::string lower_input = StringUtils::to_lowercase(input);
	
	//depends greatly on the current menu, but all menus accept "quit"
	if (input != "")
	{
		#ifdef DEBUG
			Log::write("User input: " + input + ", lowered_input read as: " + lower_input);
		#endif
	
		if (lower_input == "q" || lower_input == "quit")
		{
			if (gs.menu_index == 0)
			{
				gs.menu_index = -1;
				gs.menu_transition = true;
			}
			else
			{
				gs.menu_index = 0;
				gs.menu_transition = true;
			}
		}
		else if (gs.menu_index == 0)//main menu
		{
			if (lower_input == "1")//New Game
			{
				gs.menu_index = 1;
				gs.menu_transition = true;
			}
			else if (lower_input == "2") //Continue Game
			{
				gs.menu_index = 2;
				gs.menu_transition = true;
			}
			else if (lower_input == "3") //Restart Game
			{
				gs.menu_index = 1;//change this later
				gs.menu_transition = true;
			}
			else if (lower_input == "4") //Quit
			{
				gs.menu_index = -1;
			}
		}
		else if (gs.menu_index == 1)//Game creation screen
		{
			
		}
		else if (gs.menu_index == 2)//the actual game
		{
			#ifdef DEBUG
				Log::write("\tMenu level 2: actual game.");
			#endif
			//first off, get the handle of the room the player's in
			ECS::Handle current_room_handle = gs.level->get_object(gs.playable_character)->room_container;
			
			//check out the command
			if (lower_input.substr(0,4) == "look")
			{
				#ifdef DEBUG
					Log::write("\tLook command recognized.");
				#endif
				
				//if we're just looking around
				if (lower_input.length() < 5)
				{
					//get the description of the room
					gs.main_text += "\n\n";
					gs.main_text += gs.level->get_room(current_room_handle)->get_description();
				}
				//if we're looking at something specific...
				else
				{
					std::string rest = StringUtils::trim(lower_input.substr(4));
					//if we're looking at an object
					if (rest.substr(0,2) == "at")
					{
						//figure out how to find that object!
					}
					//if we're looking in a direction
					else
					{
						Room::Exit e = Room::Exit::INVALID;
						if (rest == "n" || rest == "north")
							e = Room::Exit::NORTH;
						else if (rest == "e" || rest == "east")
							e = Room::Exit::EAST;
						else if (rest == "s" || rest == "south")
							e = Room::Exit::SOUTH;
						else if (rest == "w" || rest == "west")
							e = Room::Exit::WEST;
						
						if (e == Room::Exit::INVALID)
						{
							gs.main_text += "\n<fg=white>Look where?";
						}
						else
						{
							ECS::Handle next_room = gs.level->get_open_neighbor(current_room_handle,e);
							
							//if there is no room that direction...
							if (next_room < 0)
							{
								gs.main_text += "\n<fg=white>You see nothing significant.";
							}
							//if there is a room that direction...
							else
							{
								gs.main_text += "\n\n<fg=white>You see " + gs.level->get_room(next_room)->get_short_description();
							}
						}
					}
				}
			}
			else if (lower_input == "inv" || lower_input == "inventory")
			{
				#ifdef DEBUG
					Log::write("\tInventory command recognized.");
				#endif
				
				std::vector<ECS::Handle>& inv = gs.level->get_object(gs.playable_character)->objects;
				gs.main_text += "\n<fg=white>Your inventory contains the following:";
				if (inv.empty())
				{
					gs.main_text += "\n<fg=white>Nothing";
				}
				else
				{
					for (unsigned i = 0; i < inv.size(); ++i)
					{
						gs.main_text += "\n";
						gs.main_text += gs.level->get_object(inv[i])->name;
					}
				}
			}
			//if the player wants to move in a direction
			else if (lower_input == "n" || lower_input == "north" 
				  || lower_input == "e" || lower_input == "east"
				  || lower_input == "s" || lower_input == "south"
				  || lower_input == "w" || lower_input == "west")
			{
				//what direction is this?
				Room::Exit e = Room::Exit::NORTH;
				if (lower_input[0] == 'w')
					e = Room::Exit::WEST;
				else if (lower_input[0] == 'e')
					e = Room::Exit::EAST;
				else if (lower_input[0] == 's')
					e = Room::Exit::SOUTH;
				
				#ifdef DEBUG
					Log::write("\tMovement command recognized as direction " + StringUtils::to_string((int)e));
				#endif
				
				ECS::Handle next_room = gs.level->get_open_neighbor(current_room_handle,e);
							
				//if there is no room that direction...
				if (next_room < 0)
				{
					gs.main_text += "\n<fg=white>You can't go that way.";
				}
				//if there is a room that direction...
				else
				{
					//remove the player from the old room
					std::vector<ECS::Handle>& cro = gs.level->get_room(current_room_handle)->objects();
					for (unsigned i = 0; i < cro.size(); ++i)
					{
						if (cro[i] == gs.playable_character)
						{
							cro[i] = cro.back();
							cro.pop_back();
							break;
						}
					}
					
					//then add it to the new room
					gs.level->get_room(next_room)->objects().push_back(gs.playable_character);
					gs.level->get_object(gs.playable_character)->room_container = next_room;
					
					//get the description of the room
					gs.main_text += "\n<fg=white>";
					gs.main_text += gs.level->get_room(next_room)->get_description();
				}
				/*
				//can the player move that direction?
				if(gs.level->get_room(current_room_handle)->get_exit(e))
				{
					//if so, remove it from the old room
					std::vector<ECS::Handle>& cro = gs.level->get_room(current_room_handle)->objects();
					for (unsigned i = 0; i < cro.size(); ++i)
					{
						if (cro[i] == gs.playable_character)
						{
							cro[i] = cro.back();
							cro.pop_back();
							break;
						}
					}
					
					//then add it to the new room
					ECS::Handle next_room = gs.level->get_room(current_room_handle)->get_special_exit(e);
					if (next_room < 0)
					{
						int current_room_x, current_room_y;
						gs.level->get_room(current_room_handle)->get_xy(current_room_x, current_room_y);
						
						int x_modifier = e == Room::Exit::SOUTH || e == Room::Exit::NORTH ? 0 : e == Room::Exit::EAST ? 1 : -1;
						int y_modifier = e == Room::Exit::EAST || e == Room::Exit::WEST ? 0 : e == Room::Exit::SOUTH ? 1 : -1;
						
						next_room = gs.level->get_room(current_room_x + x_modifier, current_room_y + y_modifier)->get_handle();
						
					}
					if (next_room < 0)
					{
						//alert the programmer that there is an exit with no valid endpoint
						#ifdef DEBUG
							Log::write("\t\tWARNING: the room specified can't be found!");
						#endif
						
						//put the player back into this room for the time being
						cro.push_back(gs.playable_character);
					}
					else
					{
						gs.level->get_room(next_room)->objects().push_back(gs.playable_character);
						gs.level->get_object(gs.playable_character)->room_container = next_room;
						
						//get the description of the room
						gs.main_text += "\n<fg=white>";
						gs.main_text += gs.level->get_room(next_room)->get_description();
					}
				}
				else
				{
					gs.main_text += "\n<fg=white>You can't go that way.";
				}
				*/
			}
			else
			{
				gs.main_text += "\n<fg=white>Command not recognized.";
			}
			gs.main_text_dirty_flag = true;
		}
	}
}