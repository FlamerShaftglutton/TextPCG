#include <ncurses.h>
#include <string>
#include <chrono>
#include "Level.hpp"

void erase (int y, int x) {
	mvaddch(y, x, '#');
}

bool dead(int y, int x)
{
	char ch = mvinch(y,x) & A_CHARTEXT;
	int h,w;
	getmaxyx(stdscr,h,w);
	
	return (ch == '#' || y < 0 || y >= h || x < 0 || x >= w);
}

void game_loop(char main_char, int row, int col, int ch) {
	// Check if the user wishes to play the game
	if(ch == 'q' || ch =='Q') return;

	// Show the main character on the screen
	mvaddch(row, col, main_char);
	refresh();
	
	int last_move = KEY_RIGHT;
	int this_move = KEY_RIGHT;
	
	auto start_time = std::chrono::high_resolution_clock::now();
	unsigned long mcount = 0;
	unsigned long frame_period = 250;
	unsigned long next_frame = frame_period;
	
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
			refresh();
		}
		
		move(0,0);
		printw(s.c_str());
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
}

int main() {
	// Define main character symbol and initial position
	int row = 10, col = 10;
	char main_char = '@';

	// Start ncurses
	init();

	// Print a welcome message on screen
	printw("Welcome to the RR game.\nPress any key to start.\nIf you want to quit press \"q\" or \"Q\"");

	// Wait until the user press a key
	int ch = getch();

	// Clear the screen
	clear();	

	// Start the game loop
	game_loop(main_char, row, col, ch);

	// Clear ncurses data structures
	endwin();

	return 0;
}

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
