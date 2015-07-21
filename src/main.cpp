//#include <ncurses.h>
#include <string>
#include <chrono>
#include "Level.hpp"
#include "Console.hpp"

void game_loop(Console& console)
{
	auto start_time = std::chrono::high_resolution_clock::now();
	unsigned long mcount = 0;
	unsigned long frame_period = 250;
	unsigned long next_frame = frame_period;
	
	for(int counter = 0;;++counter) {
		mcount = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count();
		
		unsigned long i = mcount / frame_period;
		std::string s = "000";
		s[0] += char((i%1000)/100);
		s[1] += char((i%100)/10);
		s[2] += char(i%10);
		
		
		if (mcount > next_frame)
		{
			console.set_fgcolor(Console::Color::White);
			console.set_bgcolor(Console::Color::Black);
			console.write_string(0,0,s,1);
			
			console.set_bgcolor(Console::Color::Green);
			for (int i = 0; i < console.get_width(1); ++i)
				console.write_character(console.get_height(1)-1,i,' ');
				
			for (int i = 0; i < console.get_width(3); ++i)
				console.write_character(0,i,' ');
			
			console.refresh();
		}
	}
}

int main()
{
	Console console;
	console.add_frame(5,-1,0,0,true,false,false);
	console.add_frame(console.get_height()-7,-1,0,5,false,false,true);
	console.add_frame(2,-1,0,console.get_height()-2,true,false,false);
	
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
