#include "Console.hpp"
#include <ncurses.h>
#include <string>
#include <vector>

struct Frame //this class doesn't exist outside of this file
{
	int height,width;
	int y,x;
	
	bool height_locked, width_locked,
		 word_wrap;
};

struct Console_Pimpl
{
	Console::Color current_fg_color,
				   current_bg_color;
	
	std::vector<Frame> frames;
};

Console::Console()
{
	//initialize the ncurses library
	initscr();
	clear();
	noecho();
	cbreak();
	keypad(stdscr, TRUE);
	nodelay(stdscr,TRUE);
	curs_set(0);
	start_color();
	
	//fill in private members pimpl
	private_members = new Console_Pimpl();
	private_members->current_bg_color = Console::Color::Black;
	private_members->current_fg_color = Console::Color::White;
}

Console::~Console()
{
	endwin();
	delete private_members;
}

void Console::write_character(int row, int column, char c, int frame)
{
	if (frame == 0)
		mvaddch(row, column, c);
	else
		mvaddch(row + private_members->frames[frame - 1].y, column + private_members->frames[frame - 1].x, c);
}

void Console::write_string(int row, int column, const std::string & s, int frame)
{
	int x_offset = 0,
		y_offset = 0;
	
	int f_width, f_height;
	
	int current_x = column;
	
	bool word_wrap = false;
	
	if (frame == 0)
	{
		getmaxyx(stdscr,f_height,f_width);
	}
	else
	{
		Frame& f = private_members->frames[frame - 1];
		
		f_width = f.width;
		f_height = f.height;
		
		x_offset = f.x;
		y_offset = f.y;
		
		word_wrap = f.word_wrap;
	}
	
	for (unsigned i = 0; i < s.length(); ++i)
	{
		//if there's a new line, break!
		if (s[i] == '\n')
		{
			++row;
			continue;
		}
		//if we've gone too far then check for word wrapping
		else if (current_x >= f_width)
		{
			if (word_wrap)
			{
				++row;
				current_x = column;
			}
			else
			{
				continue;
			}
		}
		//if we've gone too far vertically then just quit
		if (row >= f_height)
		{
			break;
		}
		
		//actually write out the character!
		mvaddch(y_offset + row, x_offset + current_x, s[i]);
		++current_x;
	}
}

void Console::set_fgcolor(Color color)
{
	private_members->current_fg_color = color;
	init_pair(1,(int)color,(int)(private_members->current_bg_color));
	attrset(1);
}

void Console::set_bgcolor(Color color)
{
	private_members->current_bg_color = color;
	init_pair(1,(int)(private_members->current_fg_color),(int)color);
	attrset(1);
}

int Console::get_width(int frame)
{
	if (frame == 0)
	{
		int x;
		getmaxyx(stdscr,frame,x);
		
		return x;
	}
	else if (frame <= (int)private_members->frames.size())
	{
		return private_members->frames[frame - 1].width;
	}
	else
	{
		return -1;
	}
}

int Console::get_height(int frame)
{
	if (frame == 0)
	{
		int y;
		getmaxyx(stdscr,y,frame);
		
		return y;
	}
	else if (frame <= (int)private_members->frames.size())
	{
		return private_members->frames[frame - 1].height;
	}
	else
	{
		return -1;
	}
}

void Console::refresh()
{
	wrefresh(stdscr);
}

void Console::add_frame(int height, int width, int x, int y, bool height_locked, bool width_locked, bool word_wrap)
{
	int wh,ww;
	getmaxyx(stdscr,wh,ww);
	Frame f;
	if (height < 0)
		f.height = wh;
	else
		f.height = height;
	
	if (width < 0)
		f.width = ww;
	else
		f.width = width;
		
	//f.height = height;
	//f.width = width;
	f.x = x;
	f.y = y;
	f.height_locked = height_locked;
	f.width_locked = width_locked;
	f.word_wrap = word_wrap;
	
	private_members->frames.push_back(f);
}
