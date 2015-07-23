#include "Serialize.hpp"
#include <fstream>
#include <string>
#include <vector>
#include <locale>
#include "GameState.hpp"
#include "string_utils.hpp"
#ifdef DEBUG
	#include "Log.hpp"
#endif

void Serialize::from_file(std::string fname, GameState& gs)
{
	//open the file
	std::ifstream infile(fname.c_str());
	char ESC = 4;
	std::string s;
	
	//some helper functions, conventiently accessible only within this function
	auto get_int = [&](){std::getline(infile,s,ESC); return StringUtils::stoi(s); };
	auto get_float = [&](){std::getline(infile,s,ESC); return StringUtils::stof(s); };
	auto get_bool = [&](){std::getline(infile,s,ESC); return StringUtils::stoi(s) == 1; };
	auto get_string = [&](){std::getline(infile,s,ESC); return s; };
	
	//first, the GameState stuff
	gs.frames_elapsed = get_int();
	gs.menu_index = get_int();
	gs.menu_transition = get_bool();
	gs.playable_character = (ECS::Handle)get_int();
	gs.main_text = get_string();
	gs.main_text_dirty_flag = get_bool();
	
	//now the level
	int level_width = get_int();
	int level_height = get_int();
	int num_rooms = get_int();
	int num_objects = get_int();
	gs.level = new Level(level_width,level_height);
	
	//now the rooms
	for (int i = 0; i < num_rooms; ++i)
	{
		//check the skip/ok flags
		if (get_bool())
		{
			//first, create a new room
			Room* r = gs.level->get_room(gs.level->create_room(i % level_width, i / level_width));

			//now set the exits
			std::string exits = get_string();
			for (int j = 0; j < 9;++j)
			{
				r->set_exit((Room::Exit)j,exits[j] == '1');
			}

			//then the special exits
			auto split_stuff = StringUtils::split(get_string(),' ');
			for (int j = 0; j < 9; ++j)
			{
				r->set_special_exit((Room::Exit)j,StringUtils::stoi(split_stuff[j]));
			}
			
			//then the strings
			r->set_short_description(get_string());
			r->set_description(get_string());
			r->set_minimap_symbol(get_string());
			
			split_stuff = StringUtils::split(get_string(),' ');
			for (unsigned j = 0; j < split_stuff.size(); ++j)
			{
				r->objects().push_back((ECS::Handle)StringUtils::stoi(split_stuff[j]));
			}
		}
	}
	
	//finally, get the objects
	for (int i = 0; i < num_objects; ++i)
	{
		//check the skip/ok flags
		ECS::Handle oh = gs.level->create_object();
		if (!get_bool())
		{
			//this ensures a blank (well, nullptr) space, which keeps the handles aligned
			gs.level->destroy_object(oh);
		}
		else
		{
			Object* o = gs.level->get_object(oh);
			
			o->visible = get_bool();
			o->visible_in_short_description = get_bool();
			o->friendly = get_bool();
			o->mobile = get_bool();
			o->playable = get_bool();
			
			o->object_container = (ECS::Handle)get_int();
			o->room_container = (ECS::Handle)get_int();
			
			o->hitpoints = get_int();
			o->attack = get_int();
			o->hit_chance = get_float();
			o->description = get_string();
			o->name = get_string();
			o->on_use = get_string();
			
			std::vector<std::string> objs = StringUtils::split(get_string(),' ');
			for (unsigned j = 0; j < objs.size(); ++j)
			{
				o->objects.push_back((ECS::Handle)StringUtils::stoi(objs[j]));
			}
		}
	}
}

void Serialize::to_file(std::string fname, GameState& gs)
{
	//open the file
	std::ofstream outfile(fname.c_str());
	char ESC = 4;
	
	if (!outfile.is_open())
	{
		#ifdef DEBUG
			Log::write("File '" + fname + "' not found. Serialization did not occur.");
		#endif
		return;
	}
	
	//the first lines will be the GameState stuff
	outfile << gs.frames_elapsed << ESC;
	outfile << gs.menu_index << ESC;
	outfile << (int)gs.menu_transition << ESC;
	outfile << (int)gs.playable_character << ESC;
	outfile << gs.main_text << ESC;
	outfile << gs.main_text_dirty_flag << ESC;
	
	//now output the level details
	unsigned num_rooms   = gs.level->get_num_rooms();
	unsigned num_objects = gs.level->get_num_objects();
	outfile << gs.level->get_width() << ESC;
	outfile << gs.level->get_height() << ESC;
	outfile << num_rooms << ESC;
	outfile << num_objects << ESC;
	
	//now output the rooms
	for (unsigned i = 0; i < num_rooms; ++i)
	{
		Room* r = gs.level->get_room((ECS::Handle)i);
		
		if (r == nullptr)
		{
			outfile << 0 << ESC;
		}
		else
		{
			outfile << 1 << ESC;
			
			//first the exits
			for (unsigned j = 0; j < 9; ++j)
			{
				outfile << (int)r->get_exit((Room::Exit)j);
			}
			outfile << ESC;
			
			//then the special exits
			for (unsigned j = 0; j < 9; ++j)
			{
				outfile << (int)r->get_special_exit((Room::Exit)j) << ' ';
			}
			outfile << ESC;
			
			//then the strings
			outfile << r->get_short_description() << ESC;
			outfile << r->get_description() << ESC;
			outfile << r->get_minimap_symbol() << ESC;
			
			//then the object handles the room contains
			for (unsigned j = 0; j < r->objects().size(); ++j)
			{
				outfile << (int)r->objects()[j] << ' ';
			}
			outfile << ESC;
		}
	}
	
	//now output the objects
	for (unsigned i = 0; i < num_objects; ++i)
	{
		Object* o = gs.level->get_object((ECS::Handle)i);
		
		if (o == nullptr)
		{
			outfile << 0 << ESC;
		}
		else
		{
			outfile << 1 << ESC;
			
			outfile << (int)o->visible << ESC;
			outfile << (int)o->visible_in_short_description << ESC;
			outfile << (int)o->friendly << ESC;
			outfile << (int)o->mobile << ESC;
			outfile << (int)o->playable << ESC;
			
			outfile << (int)o->object_container << ESC;
			outfile << (int)o->room_container << ESC;
			
			outfile << o->hitpoints << ESC;
			outfile << o->attack << ESC;
			outfile << o->hit_chance << ESC;
			outfile << o->description << ESC;
			outfile << o->name << ESC;
			outfile << o->on_use << ESC;
			
			for (unsigned j = 0; j < o->objects.size(); ++j)
			{
				outfile << (int)o->objects[j] << ' ';
			}
			outfile << ESC;
		}
	}
	
	outfile.close();
}