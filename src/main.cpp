//#include <ncurses.h>
#include <string>
#include <chrono>
#include "GameState.hpp"
#include "Console.hpp"
#include "Level.hpp"
#include <iostream>
#include "string_utils.hpp"

void outline_frame(Console& console, int frame, bool top, bool bottom, bool left, bool right)
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

void handle_input(Console& console, GameState& gs)
{
	std::string input = console.check_for_input();
	std::string lower_input = StringUtils::to_lowercase(input);
	
	//depends greatly on the current menu, but all menus accept "quit"
	if (lower_input == "q" || lower_input == "quit")
	{
		if (gs.menu_index == 0)
			gs.menu_index = -1;
		else
			gs.menu_index = 0;
	}
	else if (input != "")
	{
		if (gs.menu_index == 0)//main menu
		{
			if (lower_input == "1")//New Game
			{
				gs.menu_index = 1;
				console.clear();
			}
			else if (lower_input == "2") //Continue Game
			{
				gs.menu_index = 2;
				console.clear();
			}
			else if (lower_input == "3") //Restart Game
			{
				gs.menu_index = 1;//change this later
				console.clear();
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
			gs.main_text += "\n";
			gs.main_text += input;
			gs.main_text_dirty_flag = true;
		}
	}
}

void update_game(Console& console, GameState& gs)//normally I'd pass in a delta, but we're going to lock this to X frames per second (probably 5)
{
	//first off, the menu will be the real defining factor here
	if (gs.menu_index < 0)//shouldn't ever happen, but better safe than sorry
	{
		return;
	}
	else if (gs.menu_index == 0)//main menu
	{
		gs.main_text = "Please enter the number of your choice then press <fg=red>ENTER<fg=white>.\n";
		gs.main_text += "1. New Game\n";
		gs.main_text += "2. Continue Game\n";
		gs.main_text += "3. Restart Game\n";
		gs.main_text += "4. Quit\n";
	}
	else if (gs.menu_index == 1)//Game creation screen
	{
	
	}
	else if (gs.menu_index == 2)//the actual game
	{
		
	}
}

void draw_screen(Console& console, GameState& gs, int text_box_frame, int lower_bar_frame, int minimap_frame, int NPC_frame, int inventory_frame)
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
		//draw the borders for menu 1
		outline_frame(console,minimap_frame,false,true,true,false);
		outline_frame(console,NPC_frame,false,true,true,false);
		outline_frame(console,inventory_frame,false,false,true,false);
		
		//write the frame count for debugging
		console.write_string(0,0,StringUtils::to_string(gs.frames_elapsed),minimap_frame);
		
		//display some status bar text
		console.write_string(0,0,"Health: 85% Stamina: 100%",lower_bar_frame);
	}
	
	//change color and write out the text-box stuff
	console.set_fgcolor(Console::Color::White);
	console.set_bgcolor(Console::Color::Black);
	console.write_string(0,0,gs.main_text,text_box_frame);
	
	//refresh the console to display the changes
	console.refresh();
}

void game_loop(Console& console)
{
	//set up the console frames
	int tw = console.get_width()-8;
	int text_box_frame = console.add_frame(console.get_height() - 2, tw, 0, 0, false, false, true);
	int minimap_frame = console.add_frame(8,8,0,tw,false,false,false);
	int NPC_frame = console.add_frame(8,8,8,tw,false,false,false);
	int inventory_frame = console.add_frame(console.get_height() - 18, 8, 16, tw, false, false, false);
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
	gs.main_text = "<fg=Red>Welcome! <fg=white>Type your command then press ENTER.\n";
	gs.main_text_dirty_flag = true;
	gs.level = new Level(5,5);
	gs.frames_elapsed = 0;
	
	//loop until something calls 'break'
	for(int counter = 0;;++counter)
	{
		//check for input from the user
		handle_input(console,gs);
		
		//if the user quit, then kill the program
		if (gs.menu_index < 0)
			break;
	
		//if we're ready for another frame, fill it in!
		mcount = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count();
		if (mcount > next_frame)
		{
			++gs.frames_elapsed;
			next_frame += frame_period;
			
			update_game(console, gs);//pass in the number of ms since the last frame
		}
		
		//if we're ready for another draw, draw it!
		if (mcount > next_draw)
		{
			next_draw += draw_period;
			draw_screen(console, gs, text_box_frame, lower_bar_frame, minimap_frame, NPC_frame, inventory_frame);
		}
	}
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
