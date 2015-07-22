//#include <ncurses.h>
#include <string>
#include <chrono>
#include "GameState.hpp"
#include "Console.hpp"
#include "Level.hpp"
#include "DrawSystem.hpp"
#include "InputSystem.hpp"
#include "UpdateSystem.hpp"

void game_loop(Console& console)
{
	//set up the console frames
	int tw = console.get_width()-10;
	int text_box_frame = console.add_frame(console.get_height() - 2, tw, 0, 0, false, false, true);
	int minimap_frame = console.add_frame(10,10,0,tw,false,false,false);
	int NPC_frame = console.add_frame(6,10,10,tw,false,false,false);
	int inventory_frame = console.add_frame(console.get_height() - 18, 10, 16, tw, false, false, false);
	int lower_bar_frame = console.add_frame(1,-1,console.get_height()-2,0,true,false,false);
	int echo_frame = console.add_frame(1,-1,console.get_height()-1,0,true,false,false);
	
	//set up the console echo frame (where the user input is displayed until ENTER is hit)
	console.set_echo_frame(echo_frame);
	console.set_echo_colors(Console::Color::White, Console::Color::Black);

	//set up the timing stuff
	auto start_time = std::chrono::high_resolution_clock::now();
	unsigned long mcount = 0;
	unsigned long frame_period = 1000/5;//5 FPS (updates)
	unsigned long next_frame = 0;
	unsigned long draw_period = 1000/30;//30 FPS (draws)
	unsigned long next_draw = 0;
	
	
	//set up the game state
	GameState gs;
	gs.menu_index = 0;
	gs.main_text = "";
	gs.main_text_dirty_flag = true;
	gs.level = new Level(9,9);
	gs.frames_elapsed = 0;
	gs.menu_transition = true;
	
	//Debugging, create dummy level
	{
		for (int i = 1; i <4; ++i)
		{
			for (int j = 1; j < 4; ++j)
			{
				ECS::Handle h = gs.level->create_room(i,j);
				gs.level->get_room(h)->set_short_description("A Small Room");
				gs.level->get_room(h)->set_minimap_symbol("<fg=yellow><bg=yellow> ");
			}
		}
	
		gs.level->get_room(1,1)->set_exit(Room::Exit::EAST,true);
		gs.level->get_room(1,1)->set_exit(Room::Exit::SOUTH,true);
		gs.level->get_room(1,1)->set_description("<fg=white>Room 1,1.\n<fg=green>Lawful Good!");
		
		gs.level->get_room(2,1)->set_exit(Room::Exit::WEST,true);
		gs.level->get_room(2,1)->set_exit(Room::Exit::SOUTH,true);
		gs.level->get_room(2,1)->set_description("<fg=white>Room 2,1.\n<fg=green>Lawful Neutral!");
		
		gs.level->get_room(3,1)->set_exit(Room::Exit::SOUTH,true);
		gs.level->get_room(3,1)->set_description("<fg=white>Room 3,1.\n<fg=green>Lawful Evil!");
		
		gs.level->get_room(1,2)->set_exit(Room::Exit::NORTH,true);
		gs.level->get_room(1,2)->set_exit(Room::Exit::SOUTH,true);
		gs.level->get_room(1,2)->set_description("<fg=white>Room 1,2.\n<fg=yellow>Neutral Good!");
		
		gs.level->get_room(2,2)->set_exit(Room::Exit::NORTH,true);
		gs.level->get_room(2,2)->set_exit(Room::Exit::SOUTH,true);
		gs.level->get_room(2,2)->set_description("<fg=white>Room 2,2.\n<fg=yellow>True neutral!");
		
		gs.level->get_room(3,2)->set_exit(Room::Exit::NORTH,true);
		gs.level->get_room(3,2)->set_exit(Room::Exit::SOUTH,true);
		gs.level->get_room(3,2)->set_description("<fg=white>Room 3,2.\n<fg=yellow>Evil neutral!");
		
		gs.level->get_room(1,3)->set_exit(Room::Exit::NORTH,true);
		gs.level->get_room(1,3)->set_description("<fg=white>Room 1,3.\n<fg=red>Chaotic Good!");
		
		gs.level->get_room(2,3)->set_exit(Room::Exit::EAST,true);
		gs.level->get_room(2,3)->set_exit(Room::Exit::NORTH,true);
		gs.level->get_room(2,3)->set_description("<fg=white>Room 2,3.\n<fg=red>Chaotic Neutral!");
		
		gs.level->get_room(3,3)->set_exit(Room::Exit::WEST,true);
		gs.level->get_room(3,3)->set_exit(Room::Exit::NORTH,true);
		gs.level->get_room(3,3)->set_exit(Room::Exit::EAST,true);
		gs.level->get_room(3,3)->set_description("<fg=white>Room 3,3.\n<fg=red>Chaotic Evil!");
		
		ECS::Handle h = gs.level->create_room(4,3);
		Room* r = gs.level->get_room(h);
		r->set_short_description("A circular entryway");
		r->set_minimap_symbol("<fg=yellow><bg=white> ");
		r->set_description("<fg=white>An <fg=purple>odd<fg=white> circular entryway. The walls and floors are cold masonry, and on all sides are stone archways leading to more rooms.");
		r->set_exit(Room::Exit::WEST,true);
		r->set_exit(Room::Exit::NORTH,true);
		r->set_exit(Room::Exit::EAST,true);
		r->set_exit(Room::Exit::SOUTH,true);
		
		h = gs.level->create_room(4,2);
		r = gs.level->get_room(h);
		r->set_short_description("A dead end");
		r->set_minimap_symbol("<fg=yellow><bg=white> ");
		r->set_description("<fg=white>A small rectangular room with no visible exit but from the way you came in. The walls and floors are cold masonry, but the corners are dark and hard to see.");
		r->set_exit(Room::Exit::SOUTH,true);
		
		h = gs.level->create_room(5,3);
		r = gs.level->get_room(h);
		r->set_short_description("A dead end");
		r->set_minimap_symbol("<fg=yellow><bg=white> ");
		r->set_description("<fg=white>A small rectangular room with no visible exit but from the way you came in. The walls and floors are cold masonry, but the corners are dark and hard to see.");
		r->set_exit(Room::Exit::WEST,true);
		
		h = gs.level->create_room(4,4);
		r = gs.level->get_room(h);
		r->set_short_description("A dead end");
		r->set_minimap_symbol("<fg=yellow><bg=white> ");
		r->set_description("<fg=white>A small rectangular room with no visible exit but from the way you came in. The walls and floors are cold masonry, but the corners are dark and hard to see.");
		r->set_exit(Room::Exit::NORTH,true);
		
		gs.playable_character = gs.level->create_object();
		Object* o = gs.level->get_object(gs.playable_character);
		o->visible = true;
		o->friendly = true;
		o->mobile = true;
		o->playable = true;
		o->room_container = gs.level->get_room(2,2)->get_handle();
		o->object_container = -1;
		o->hitpoints = 100;
		o->attack = 10;
		o->hit_chance = 0.75f;
		o->name = "You";
		o->description = "A <fg=red>hideous<fg=white> looking human. Possibly beaten, or possibly just always ugly. Hard to tell.";
		gs.level->get_room(2,2)->objects().push_back(o->get_handle());
	}
	
	//set up our systems
	InputSystem input_system;
	UpdateSystem update_system;
	DrawSystem draw_system(text_box_frame, lower_bar_frame, minimap_frame, NPC_frame, inventory_frame);
	
	
	//loop until something calls 'break'
	for(int counter = 0;;++counter)
	{
		//check for input from the user
		input_system.do_work(console,gs);
		
		//if the user quit, then kill the program
		if (gs.menu_index < 0)
			break;
	
		//if we're ready for another frame, fill it in!
		mcount = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count();
		if (mcount > next_frame)
		{
			++gs.frames_elapsed;
			next_frame += frame_period;
			
			update_system.do_work(console, gs);//pass in the number of ms since the last frame
		}
		
		//if we're ready for another draw, draw it!
		if (mcount > next_draw)
		{
			next_draw += draw_period;
			draw_system.do_work(console, gs);
		}
	}
	
	//clean up some stuff
	delete gs.level;
}

int main()
{
	//set up the console with the settings we want
	Console console;
	
	game_loop(console);
	
	return 0;
}

/*
void game_loop(char main_char, int row, int col, int ch) {
	// Check if the user wishes to play the game
	if(ch == 'q' || ch =='Q') return;

	// Show the main character on the screen
	//mvaddch(row, col, main_char);
	//refresh();
	
	int last_move = KEY_RIGHT;
	int this_move = KEY_RIGHT;
	
	auto start_time = std::chrono::high_resolution_clock::now();
	unsigned long mcount = 0;
	unsigned long frame_period = 250;
	unsigned long next_frame = frame_period;
	
	std::string te = "";
	std::string text_stream = "Type your commands, then hit enter.";
	
	init_pair(1,COLOR_BLACK,COLOR_GREEN);
	
	for(int counter = 0;;++counter) {
		ch = getch();

		if(ch == KEY_LEFT && last_move != KEY_RIGHT)
			this_move = KEY_LEFT;
		else if(ch == KEY_RIGHT && last_move != KEY_LEFT)
			this_move = KEY_RIGHT;
		else if(ch == KEY_UP && last_move != KEY_DOWN)
			this_move = KEY_UP;
		else if(ch == KEY_DOWN && last_move != KEY_UP)
			this_move = KEY_DOWN;
		else if(ch == 'q' || ch == 'Q')
			break;
		else if (ch == KEY_BACKSPACE && te.length() > 0)
			te.pop_back();
		else if (ch == KEY_ENTER || ch == '\n')
		{
			wprintw(windowText, "\n");
			wprintw(windowText, te.c_str());
			te = "";
		}
		else if (ch >= 0)
			te += char(ch);
		
		mcount = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count();
		
		unsigned long i = mcount / frame_period;
		std::string s = "000";
		s[0] += char((i%1000)/100);
		s[1] += char((i%100)/10);
		s[2] += char(i%10);
		
		if (mcount > next_frame)
		{
			erase(row,col);
			switch (this_move)
			{
				case KEY_LEFT: --col; break;
				case KEY_RIGHT: ++col; break;
				case KEY_UP: --row; break;
				case KEY_DOWN: ++row; break;
			}
			if (dead(row,col))
			{
				//clear the screen and start over
				break;
			}
			else
			{
				mvaddch(row,col,main_char);
				next_frame += frame_period;
				last_move = this_move;
			}
		}
		
		
		int y,x;
		getmaxyx(windowBottom,y,x);
		
		attrset(COLOR_PAIR(1));
		wmove(windowBottom,0,0);
		for (y = 0; y < x; ++y)
			mvwaddch(windowBottom,0, y, ' ');
		//wmove(windowBottom,0,0);
		//wprintw(windowBottom,"HP: 87 %");
		
		//attrset(A_NORMAL);
		//wmove(windowBottom,1,0);
		//wclrtoeol(windowBottom);
		
		//printw(s.c_str());
		//wprintw(windowBottom,te.c_str());
		wrefresh(windowBottom);
		
		
		outline_window(windowTop);
		//outline_window(windowText);
		//outline_window(windowBottom);
	}
}

void init()
{
	initscr();
	clear();
	noecho();
	cbreak();
	keypad(stdscr, TRUE);
	nodelay(stdscr,TRUE);
	curs_set(0);
	start_color();
}

int main() {
	// Define main character symbol and initial position
	int row = 10, col = 10;
	char main_char = '@';

	// Start ncurses
	init();

	// Print a welcome message on screen
	//printw("Welcome to the RR game.\nPress any key to start.\nIf you want to quit press \"q\" or \"Q\"");

	// Wait until the user press a key
	int ch = getch();

	// Clear the screen
	clear();

	// Start the game loop
	game_loop(main_char, row, col, ch);
	
	// Destroy window objects
	delwin(windowTop);
	delwin(windowText);
	delwin(windowBottom);

	// Clear ncurses data structures
	endwin();

	return 0;
}
*/

/*
#include <iostream>

namespace Console
{
    enum Color {
        FG_RED      = 31,
        FG_GREEN    = 32,
		FG_YELLOW   = 33,
        FG_BLUE     = 34,
		FG_PURPLE   = 35,
		FG_CYAN     = 36,
		FG_WHITE    = 37,
        FG_DEFAULT  = 39,
        BG_RED      = 41,
        BG_GREEN    = 42,
        BG_BLUE     = 44,
        BG_DEFAULT  = 49
    };
	
	//http://ascii-table.com/ansi-escape-sequences.php
	std::ostream& operator<<(std::ostream& os, const Color mod) 
	{
		return os << "\033[" << (int)mod << "m"; //gotta typecast to int or it recursively tries to print itself
	}
	
	blank_console()
	{
		std::cout << "\033[2J";
	}
	
	save_cursor()
	{
		std::cout << "\033[s";
	}
	
	restore_cursor()
	{
		std::cout << "\033[u";
	}
};



int main() {
    //Color::Modifier red(Color::FG_RED);
    //Color::Modifier def(Color::FG_DEFAULT);
	char c;
	Console::blank_console();
	Console::save_cursor();
    std::cout << "This ->" << Console::Color::FG_RED << "word" << Console::Color::FG_DEFAULT << "<- is red." << std::endl;
	Console::restore_cursor();
	std::cin >> c;
	std::cout << c;
}
*/
