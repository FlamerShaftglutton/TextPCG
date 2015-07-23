#include "InputSystem.hpp"
#include "Console.hpp"
#include "GameState.hpp"
#include "string_utils.hpp"
#include <string>

#ifdef DEBUG
	#include "Log.hpp"
#endif

void InputSystem::command_look_room(Console& console, GameState& gs, ECS::Handle room, bool short_description)
{
	gs.main_text += "\n\n<fg=white>";
	Room* r = gs.level->get_room(room);
	if (short_description)
	{
		gs.main_text += "You see ";
		gs.main_text += r->get_short_description();
		
	}
	else
	{
		gs.main_text += r->get_description();
	}
	
	bool has_stuff = false;
	std::string contents = "<fg=white><bg=black>\n";
	contents += short_description ? "You can also see the following:" : "The room contains:";
	for (unsigned i = 0; i < r->objects().size(); ++i)
	{
		Object* o = gs.level->get_object(r->objects()[i]);
		if (!o->playable && (!short_description || o->visible_in_short_description))
		{
			has_stuff = true;
			std::string co = "<fg=white><bg=black>";
			if (o->mobile)
			{
				if (o->friendly)
				{
					co = "<fg=green><bg=black>";
				}
				else
				{
					co = "<fg=red><bg=black>";
				}
			}
				
			contents += co + "\n\t" + o->name;
		}
	}
	if (has_stuff)
	{
		gs.main_text += contents;
	}
	else if (!short_description)
	{
		gs.main_text += contents;
		gs.main_text += "\n\tNothing";
	}
}

void InputSystem::command_look_object(Console& console, GameState& gs, std::string desc, std::string type)
{
	//first, we gotta figure out what we're looking at/in
	Object* obj = nullptr;
	Object* player = gs.level->get_object(gs.playable_character);
	Room* current_room = gs.level->get_room(player->room_container);
	desc = StringUtils::trim(desc);
	type = StringUtils::trim(type);
	
	//look through the inventory
	for (ECS::Handle oh : player->objects)
	{
		Object* o = gs.level->get_object(oh);
		
		if (o->name.find(desc) != std::string::npos)
		{
			obj = o;
			break;
		}
	}
	
	//look through the room
	if (obj == nullptr)
	{
		for (ECS::Handle oh : current_room->objects())
		{
			Object* o = gs.level->get_object(oh);
			
			if (o->name.find(desc) != std::string::npos)
			{
				obj = o;
				break;
			}
		}
	}
	
	//if we couldn't find anything, spit out the error message
	if (obj == nullptr)
	{
		gs.main_text += "<fg=white><bg=black>\n\nYou don't see anything like '" + desc + "' here.";
	}
	//if we did find something, then the output will depend on the type: "look in ..." or "look at ..."
	else
	{
		if (type == "at")
		{
			gs.main_text += "<fg=white><bg=black>\n\n" + obj->name + "\n<fg=white><bg=black>" + obj->description;
		}
		else if (type == "in")
		{
			gs.main_text += "<fg=white><bg=black>\n\n" + obj->name + " contains the following:";
			
			if (obj->objects.empty())
			{
				gs.main_text += "\n\tNothing";
			}
			else
			{
				for (ECS::Handle oh : obj->objects)
				{
					gs.main_text += "<fg=white><bg=black>\n\t" + gs.level->get_object(oh)->name;
				}
			}
		}
		else
		{
			//output an error message I guess? This is probably handled in the do_work function already, but just to be safe...
			gs.main_text += "<fg=white><bg=black>\n\nLook in or at what?";
		}
	}
}

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
					command_look_room(console,gs,current_room_handle,false);
				}
				//if we're looking at something specific...
				else
				{
					std::string rest = StringUtils::trim(lower_input.substr(4));
					std::string type = StringUtils::trim(rest.substr(0,rest.find_first_of(' ')));
					//if we're looking at an object
					if (type == "at" || type == "in")
					{
						//split the rest into the type ("at" or "in") and the description
						std::string desc = StringUtils::trim(rest.substr(rest.find_first_of(' ')));
						
						//call the function
						command_look_object(console,gs,desc,type);
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
								command_look_room(console,gs,next_room,true);
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
					command_look_room(console,gs,next_room,false);
				}
			}
			else
			{
				gs.main_text += "\n<fg=white>Command not recognized.";
			}
			gs.main_text_dirty_flag = true;
		}
	}
}