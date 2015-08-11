#include "DrawSystem.hpp"
#include "Console.hpp"
#include "GameState.hpp"
#include "string_utils.hpp"
#include <string>
#include <utility>

void DrawSystem::do_work(Console& console, GameState& gs)
{
	Console::FrameSet& fs = console.get_current_frameset();
	int text_box_frame = fs.get_frame_index_by_name("text_box"), 
		lower_bar_frame = fs.get_frame_index_by_name("lower_bar"), 
		minimap_frame = fs.get_frame_index_by_name("minimap"), 
		NPC_frame = fs.get_frame_index_by_name("NPC"), 
		inventory_frame = fs.get_frame_index_by_name("inventory");
	

	//if the main text window got new text, redraw it
	if (gs.main_text_dirty_flag)
	{
		//recalculate the string
		gs.main_text_dirty_flag = false;
		gs.main_text = Console::get_last_n_lines(gs.main_text,fs[text_box_frame].height,fs[text_box_frame].width,fs[text_box_frame].word_wrap);
		
		//change color and write out the text-box stuff
		fs.set_fgcolor(Console::Color::White);
		fs.set_bgcolor(Console::Color::Black);
		fs.write_string(0,0,gs.main_text,text_box_frame);
	}
	
	//draw the borders for any menu
	fs.set_fgcolor(Console::Color::White);
	fs.set_bgcolor(Console::Color::Blue);
	outline_frame(console,lower_bar_frame,true,false,false,false);
	
	if (gs.menu_index == UI_State::In_Game)
	{
		//write the frame count for debugging
		#ifdef DEBUG
			fs.write_string(9,0,StringUtils::to_string(gs.frames_elapsed),minimap_frame);
		#endif
		
		//display some status bar text
		int player_health = gs.level->get_object(gs.playable_character)->hitpoints;
		std::string color = player_health < 30 ? "red" : "white";
		fs.write_string(0,0,"<fg=" + color + ">Health: " + StringUtils::to_string(player_health) + "%",lower_bar_frame);
		
		//draw the minimap frame
		draw_minimap(console,gs,minimap_frame);
		
		//draw the NPC frame
		draw_NPCs(console,gs,NPC_frame);
		
		//draw the inventory frame
		draw_inventory(console,gs,inventory_frame);
	}
	
	//change color and write out the text-box stuff
	fs.set_fgcolor(Console::Color::White);
	fs.set_bgcolor(Console::Color::Black);
	fs.write_string(0,0,gs.main_text,text_box_frame);
	
	//refresh the console to display the changes
	fs.refresh();
}

void DrawSystem::draw_minimap(Console& console, GameState& gs, int minimap_frame)
{
	//draw the minimap
	std::string symbols[9][9];
	for (int i = 0; i < 81; ++i)
	{
		symbols[i/9][i%9] = "<bg=black> ";
	}
	symbols[4][4] = "<bg=black><fg=red>@";
	
	ECS::Handle rooms[5][5] = {{-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1}};
	rooms[2][2] = gs.level->get_object(gs.playable_character)->room_container;
	
	std::vector<std::pair<int,int>> queue;
	queue.push_back({2,2});
	
	//do a flood fill of the minimap
	while (!queue.empty())
	{
		//get the next item
		auto pos = queue.back();
		ECS::Handle top = rooms[pos.first][pos.second];
		queue.pop_back();

		//check all of the surrounding spaces
		ECS::Handle n = pos.second == 0 ? -1 : gs.level->get_open_neighbor(top,Room::Exit::NORTH);
		ECS::Handle e = pos.first == 4 ? -1 : gs.level->get_open_neighbor(top,Room::Exit::EAST);
		ECS::Handle s = pos.second == 4 ? -1 : gs.level->get_open_neighbor(top,Room::Exit::SOUTH);
		ECS::Handle w = pos.first == 0 ? -1 : gs.level->get_open_neighbor(top,Room::Exit::WEST);
		
		if (n != -1)
		{
			//draw a little connector symbol
			symbols[2 * pos.first][2 * pos.second - 1] = "<fg=white><bg=black>|";
			
			//add the next room to the queue if it hasn't already been done
			if (rooms[pos.first][pos.second - 1] == -1 && gs.level->get_room(n)->get_visited())
			{
				symbols[2 * pos.first][2 * pos.second - 2] = gs.level->get_room(n)->get_minimap_symbol();
				rooms[pos.first][pos.second - 1] = n;
				queue.push_back(std::make_pair(pos.first,pos.second - 1));
			}
		}
		
		if (e != -1)
		{
			//draw a little connector symbol
			symbols[2 * pos.first + 1][2 * pos.second] = "<fg=white><bg=black>-";
			
			//add the next room to the queue if it hasn't already been done
			if (rooms[pos.first + 1][pos.second] == -1 && gs.level->get_room(e)->get_visited())
			{
				symbols[2 * pos.first + 2][2 * pos.second] = gs.level->get_room(e)->get_minimap_symbol();
				rooms[pos.first + 1][pos.second] = e;
				queue.push_back(std::make_pair(pos.first + 1,pos.second));
			}
		}
		if (s != -1)
		{
			//draw a little connector symbol
			symbols[2 * pos.first][2 * pos.second + 1] = "<fg=white><bg=black>|";
			
			//add the next room to the queue if it hasn't already been done
			if (rooms[pos.first][pos.second + 1] == -1 && gs.level->get_room(s)->get_visited())
			{
				symbols[2 * pos.first][2 * pos.second + 2] = gs.level->get_room(s)->get_minimap_symbol();
				rooms[pos.first][pos.second + 1] = s;
				queue.push_back(std::make_pair(pos.first,pos.second + 1));
			}
		}
		if (w != -1)
		{
			//draw a little connector symbol
			symbols[2 * pos.first - 1][2 * pos.second] = "<fg=white><bg=black>-";
			
			//add the next room to the queue if it hasn't already been done
			if (rooms[pos.first - 1][pos.second] == -1 && gs.level->get_room(w)->get_visited())
			{
				symbols[2 * pos.first - 2][2 * pos.second] = gs.level->get_room(w)->get_minimap_symbol();
				rooms[pos.first - 1][pos.second] = w;
				queue.push_back(std::make_pair(pos.first - 1,pos.second));
			}
		}
	}
	
	//now arrange the data into a drawable string
	std::string mm = "";
	for (int i = 0; i < 9; ++i)
	{
		for (int j = 0; j < 9; ++j)
		{
			mm += symbols[j][i];
		}
		
		if (i < 8)
		{
			mm += "\n";
		}
	}
	
	//actually draw the minimap
	Console::FrameSet& fs = console.get_current_frameset();
	fs.write_string(0,1,mm,minimap_frame);
	
	//finally, draw the borders
	fs.set_bgcolor(Console::Color::Blue);
	outline_frame(console,minimap_frame,false,true,true,false);
}

void DrawSystem::draw_NPCs(Console& console, GameState& gs, int NPC_frame)
{
	//first, clear out the old text
	Console::FrameSet& fs = console.get_current_frameset();
	fs.set_bgcolor(Console::Color::Black);
	fs.clear(NPC_frame);

	//now get the NPCs for the current room (including color tags)
	Room* r = gs.level->get_room(gs.level->get_object(gs.playable_character)->room_container);
	std::vector<std::string> content;
	
	for (unsigned i = 0; i < r->objects().size(); ++i)
	{
		Object* o = gs.level->get_object(r->objects()[i]);
		if (!o->playable && o->mobile)
		{
			std::string color = o->friendly ? "<fg=green>" : "<fg=red>";
			content.push_back("<bg=black>" + color + o->name);
		}
	}
	
	//if there are too many, say "+ X others"
	if ((int)content.size() > fs[NPC_frame].height - 1)
	{
		int i;
		for (i = 0; i < fs[NPC_frame].height - 2; ++i)
		{
			fs.write_string(i,1,content[i],NPC_frame);
		}
		fs.write_string(i, 1, "+ " + StringUtils::to_string(1 + (int)content.size() - i) + " others", NPC_frame);
	}
	//if there aren't too many, draw the names!
	else
	{
		for (unsigned i = 0; i < content.size(); ++i)
		{
			fs.write_string(i,1,content[i],NPC_frame);
		}
	}
	
	//draw the border
	fs.set_bgcolor(Console::Color::Blue);
	outline_frame(console,NPC_frame,false,true,true,false);
}

void DrawSystem::draw_inventory(Console& console, GameState& gs, int inventory_frame)
{
	//first, clear out the old text
	Console::FrameSet& fs = console.get_current_frameset();
	fs.set_bgcolor(Console::Color::Black);
	fs.clear(inventory_frame);

	//now get the inventory
	Object* player = gs.level->get_object(gs.playable_character);
	std::vector<std::string> content;
	
	auto& r = player->objects;
	for (unsigned i = 0; i < r.size(); ++i)
	{
		Object* o = gs.level->get_object(r[i]);
		content.push_back(o->name);
	}
	
	//if there are too many, say "+ X others"
	if ((int)content.size() > fs[inventory_frame].height - 1)
	{
		int i;
		for (i = 0; i < fs[inventory_frame].height - 2; ++i)
		{
			fs.write_string(i,1,content[i],inventory_frame);
		}
		fs.write_string(i, 1, "+ " + StringUtils::to_string(1 + (int)content.size() - i) + " others", inventory_frame);
	}
	//if there aren't too many, draw the names!
	else
	{
		for (unsigned i = 0; i < content.size(); ++i)
		{
			fs.write_string(i,1,content[i],inventory_frame);
		}
	}
	
	//draw the border
	fs.set_bgcolor(Console::Color::Blue);
	outline_frame(console,inventory_frame,false,false,true,false);
}

void DrawSystem::outline_frame(Console& console, int frame, bool top, bool bottom, bool left, bool right)
{
	Console::FrameSet& fs = console.get_current_frameset();
	for (int i = 0; i < fs[frame].width; ++i)
	{
		if (top)
			fs.write_character(0, i, ' ', frame);
		
		if (bottom)
			fs.write_character(fs[frame].height - 1, i, ' ', frame);
	}
	
	for (int i = 0; i < fs[frame].height; ++i)
	{
		if (left)
			fs.write_character(i, 0, ' ', frame);
		if (right)
			fs.write_character(i, fs[frame].width - 1, ' ', frame);
	}
}
