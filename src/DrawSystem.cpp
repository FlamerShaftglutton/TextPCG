#include "DrawSystem.hpp"
#include "Console.hpp"
#include "GameState.hpp"
#include "string_utils.hpp"
#include <string>
#include <utility>

DrawSystem::DrawSystem(int text_box_f, int lower_bar_f, int minimap_f, int NPC_f, int inventory_f)
{
	text_box_frame = text_box_f;
	lower_bar_frame = lower_bar_f;
	minimap_frame = minimap_f;
	NPC_frame = NPC_f;
	inventory_frame = inventory_f;
}

void DrawSystem::do_work(Console& console, GameState& gs)
{
	//if the main text window got new text, redraw it
	if (gs.main_text_dirty_flag)
	{
		//recalculate the string
		gs.main_text_dirty_flag = false;
		gs.main_text = console.get_last_n_lines(gs.main_text,console.get_height(text_box_frame),text_box_frame);
		
		//change color and write out the text-box stuff
		console.set_fgcolor(Console::Color::White);
		console.set_bgcolor(Console::Color::Black);
		console.write_string(0,0,gs.main_text,text_box_frame);
	}
	
	//draw the borders for any menu
	console.set_fgcolor(Console::Color::White);
	console.set_bgcolor(Console::Color::Blue);
	outline_frame(console,lower_bar_frame,true,false,false,false);
	
	if (gs.menu_index == 2)
	{
		//draw the borders only in the game
		outline_frame(console,minimap_frame,false,true,true,false);
		outline_frame(console,NPC_frame,false,true,true,false);
		outline_frame(console,inventory_frame,false,false,true,false);
		
		//write the frame count for debugging
		#ifdef DEBUG
			console.write_string(9,0,StringUtils::to_string(gs.frames_elapsed),minimap_frame);
		#endif
		
		//display some status bar text
		int player_health = gs.level->get_object(gs.playable_character)->hitpoints;
		std::string color = player_health < 30 ? "red" : "white";
		console.write_string(0,0,"<fg=" + color + ">Health: " + StringUtils::to_string(player_health) + "%",lower_bar_frame);
		
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
				if (rooms[pos.first][pos.second - 1] == -1)
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
				if (rooms[pos.first + 1][pos.second] == -1)
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
				if (rooms[pos.first][pos.second + 1] == -1)
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
				if (rooms[pos.first - 1][pos.second] == -1)
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
		
		//finally, draw the minimap
		console.write_string(0,1,mm,minimap_frame);
	}
	
	//change color and write out the text-box stuff
	console.set_fgcolor(Console::Color::White);
	console.set_bgcolor(Console::Color::Black);
	console.write_string(0,0,gs.main_text,text_box_frame);
	
	//refresh the console to display the changes
	console.refresh();
}

void DrawSystem::outline_frame(Console& console, int frame, bool top, bool bottom, bool left, bool right)
{
	for (int i = 0; i < console.get_width(frame); ++i)
	{
		if (top)
			console.write_character(0,i,' ',frame);
		
		if (bottom)
			console.write_character(console.get_height(frame) - 1, i, ' ', frame);
	}
	
	for (int i = 0; i < console.get_height(frame); ++i)
	{
		if (left)
			console.write_character(i,0,' ',frame);
		if (right)
			console.write_character(i,console.get_width(frame)-1,' ',frame);
	}
}