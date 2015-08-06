#include "InputSystem.hpp"
#include "Console.hpp"
#include "GameState.hpp"
#include "string_utils.hpp"
#include <string>
#include "UIConstants.hpp"

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
	desc = StringUtils::trim(desc);
	type = StringUtils::trim(type);
	Object* obj = find_object(gs, desc);
	
	//if we couldn't find anything, spit out the error message
	if (obj == nullptr)
	{
		gs.main_text += "<fg=white><bg=black>\n\nYou don't see anything like '" + desc + "' here.";
	}
	else
	{
		//if we did find something, then the output will depend on the type: "look in ..." or "look at ..."
		if (type == "at")
		{
			gs.main_text += "<fg=white><bg=black>\n\n" + obj->name + "\n<fg=white><bg=black>" + obj->description;
		}
		else if (type == "in")
		{
			if (!obj->open)
			{
				if (obj->mobile)
				{
					gs.main_text += "<fg=white><bg=black>\n\nYou can't look in that.";
				}
				else
				{
					gs.main_text += "<fg=white><bg=black>\n\nIt is closed.";
				}
			}
			else
			{
				gs.main_text += "<fg=white><bg=black>\n\n" + obj->name + " contains the following:";
				std::vector<std::string> things;
				for (ECS::Handle oh : obj->objects)
				{
					Object* o = gs.level->get_object(oh);
					
					if (o->visible)
					{
						things.push_back("<fg=white><bg=black>\n\t" + o->name);
					}
				}
				
				if (things.empty())
				{
					gs.main_text += "\n\tNothing";
				}
				else
				{
					for (std::string s : things)
					{
						gs.main_text += s;
					}
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

void InputSystem::command_use_object(Console& console, GameState& gs, std::string desc)
{
	Object* obj = find_object(gs, desc);
	
	//if we couldn't find anything, spit out the error message
	if (obj == nullptr)
	{
		gs.main_text += "<fg=white><bg=black>\n\nYou don't see anything like '" + desc + "' here.";
	}
	else
	{
		//if we did find something, call the object's on_use function
		Room* cr = gs.level->get_room(gs.level->get_object(gs.playable_character)->room_container);
		
		//create a ScriptingVariables object to pass into the objects
		ScriptingVariables sv;
		fill_scripting_variables(gs,sv,cr,obj);
		
		obj->scripts.execute_on_use(sv);
		
		unfill_scripting_variables(gs,sv,cr);
	}
}

void InputSystem::command_get_object(Console& console, GameState& gs, std::string desc)
{
	Object* obj = find_object(gs, desc);
	
	//if we couldn't find anything, spit out the error message
	if (obj == nullptr)
	{
		gs.main_text += "<fg=white><bg=black>\n\nYou don't see anything like '" + desc + "' here.";
	}
	else
	{
		//first off, if we are already holding it then don't try to pick it up again
		if (obj->object_container == gs.playable_character)
		{
			gs.main_text += "<fg=white><bg=black>\n\nYou already have that in your inventory";
		}
		//if we don't already have it, find out if it's even holdable
		else if (obj->holdable)
		{
			//if it is holdable, pick it up.
			Object* p = gs.level->get_object(gs.playable_character);
			p->objects.push_back(obj->get_handle());
			
			//now remove it from wherever it currently resides
			if (obj->object_container != -1)
			{
				auto& l = gs.level->get_object(obj->object_container)->objects;
				for (auto i = l.begin(); i != l.end(); ++i)
				{
					if (*i == obj->get_handle())
					{
						l.erase(i);
						break;
					}
				}
			}
			if (obj->room_container != -1)
			{
				auto& l = gs.level->get_room(obj->room_container)->objects();
				for (auto i = l.begin(); i != l.end(); ++i)
				{
					if (*i == obj->get_handle())
					{
						l.erase(i);
						break;
					}
				}
			}
			
			//now mark it as residing only in the inventory
			obj->object_container = gs.playable_character;
			obj->room_container = -1;
			
			//finally, let the user know they successfully picked it up
			gs.main_text += "<fg=white><bg=black>\n\nYou pick up " + obj->name + "<fg=white><bg=black> and put it in your inventory.";
		}
		else
		{
			gs.main_text += "<fg=white><bg=black>\n\nYou can't hold that.";
		}
	}
}

Object* InputSystem::find_object(GameState& gs, std::string desc)
{
	//first, we gotta figure out what we're looking at/in
	Object* obj = nullptr;
	Object* player = gs.level->get_object(gs.playable_character);
	Room* current_room = gs.level->get_room(player->room_container);
	
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
			
			if (StringUtils::to_lowercase(o->name).find(desc) != std::string::npos)
			{
				obj = o;
				break;
			}
			else if (o->open)
			{
				for (ECS::Handle ooh : o->objects)
				{
					Object* oo = gs.level->get_object(ooh);
					
					if (StringUtils::to_lowercase(oo->name).find(desc) != std::string::npos)
					{
						obj = oo;
						break;
					}
				}
				
				if (obj != nullptr)
				{
					break;
				}
			}
		}
	}
	
	return obj;
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
			if (gs.menu_index == UI_State::Main_Menu)
			{
				gs.menu_index = UI_State::Exit;
				gs.menu_transition = true;
			}
			else
			{
				gs.menu_index = UI_State::Main_Menu;
				gs.menu_transition = true;
			}
		}
		else if (gs.menu_index == UI_State::Main_Menu)//main menu
		{
			if (lower_input == "1")//New Game
			{
				gs.menu_index = UI_State::New_Game;
				gs.menu_transition = true;
			}
			else if (lower_input == "2") //Continue Game
			{
				gs.menu_index = UI_State::Load_Game;
				gs.menu_transition = true;
			}
			else if (lower_input == "3") //Restart Game
			{
				gs.menu_index = UI_State::New_Game;//change this later
				gs.menu_transition = true;
			}
			else if (lower_input == "4") //Quit
			{
				gs.menu_index = UI_State::Exit;
			}
		}
		else if (gs.menu_index == UI_State::New_Game)//Game creation screen
		{
			
		}
		else if (gs.menu_index == UI_State::In_Game)//the actual game
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
			else if (lower_input.substr(0,3) == "use")
			{
				#ifdef DEBUG
					Log::write("\tUse command recognized.");
				#endif
				
				std::string rest = StringUtils::trim(lower_input.substr(3));
				
				command_use_object(console,gs,rest);
			}
			else if (lower_input.substr(0,3) == "get")
			{
				#ifdef DEBUG
					Log::write("\tGet command recognized.");
				#endif
				
				std::string rest = StringUtils::trim(lower_input.substr(3));
				
				command_get_object(console,gs,rest);
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
			//if we're in combat, we have extra commands
			else if (gs.combat_data != nullptr)
			{
				if (lower_input == "l" || lower_input == "left" || lower_input == "dodge left" || lower_input == "<left_arrow_key>")
				{
					switch (gs.combat_data->player_position)
					{
						case CombatData::Position::left:
							gs.main_text += "\nYou are already on the enemy's left side.";
							break;
						case CombatData::Position::right:
							gs.combat_data->player_position = CombatData::Position::front;
							gs.main_text += "\nYou roll in front of the enemy.";
							break;
						case CombatData::Position::front:
						case CombatData::Position::far_front:
							gs.combat_data->player_position = CombatData::Position::left;
							gs.main_text += "\nYou roll to the enemy's left side.";
					}
				}
				else if (lower_input == "r" || lower_input == "right" || lower_input == "dodge right" || lower_input == "<right_arrow_key>")
				{
					switch (gs.combat_data->player_position)
					{
						case CombatData::Position::right:
							gs.main_text += "\nYou are already on the enemy's right side.";
							break;
						case CombatData::Position::left:
							gs.combat_data->player_position = CombatData::Position::front;
							gs.main_text += "\nYou roll in front of the enemy.";
							break;
						case CombatData::Position::front:
						case CombatData::Position::far_front:
							gs.combat_data->player_position = CombatData::Position::right;
							gs.main_text += "\nYou roll to the enemy's right side.";
					}
				}
				else if (lower_input == "<up_arrow_key>" || lower_input == "attack" || lower_input == "a")
				{
					if (gs.combat_data->player_position == CombatData::Position::far_front)
					{
						gs.combat_data->player_position = CombatData::Position::front;
						gs.main_text += "\nYou jump forward in front of the enemy.";
					}
					else if (!gs.combat_data->player_attacking)
					{
						gs.combat_data->player_attacking = true;
						gs.main_text += "\nYou attempt to attack the enemy.";
					}
				}
				else if (lower_input == "<down_arrow_key>" || lower_input == "b" || lower_input == "back")
				{
					if (gs.combat_data->player_position == CombatData::Position::front)
					{
						gs.combat_data->player_position = CombatData::Position::far_front;
						gs.main_text += "\nYou jump back from the enemy.";
					}
				}
			}
			/*//for now combat is entered automatically
			//if we're not in combat yet but want to be
			else if (lower_input.substr(0,4) == "kill" || lower_input.substr(0,6) == "attack")
			{
				#ifdef DEBUG
					Log::write("\tAttack command recognized.");
				#endif
				
				std::string rest = StringUtils::trim(lower_input.substr(3));
				Object* obj = find_object(gs, rest);
				
				if (obj != nullptr && !obj->friendly)
				{
					gs.combat_data = new CombatData;
					gs.combat_data-> = obj->get_handle();
					gs.combat_data->player_position = CombatData::Position::far_front;
					gs.combat_data->player_attacking = false;
					
					gs.combat_data->enemy_vulnerable_sides[CombatData::Position::left] = 
					gs.combat_data->enemy_vulnerable_sides[CombatData::Position::right] = 
					gs.combat_data->enemy_vulnerable_sides[CombatData::Position::front] = 
					gs.combat_data->enemy_vulnerable_sides[CombatData::Position::far_front] = false;
					
					gs.combat_data->enemy_attacking_sides[CombatData::Position::left] = 
					gs.combat_data->enemy_attacking_sides[CombatData::Position::right] = 
					gs.combat_data->enemy_attacking_sides[CombatData::Position::front] = 
					gs.combat_data->enemy_attacking_sides[CombatData::Position::far_front] = false;
				}
			}
			*/
			else
			{
				gs.main_text += "\n<fg=white>Command not recognized.";
			}
			gs.main_text_dirty_flag = true;
		}
	}
}
