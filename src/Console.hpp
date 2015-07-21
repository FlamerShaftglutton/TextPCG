#pragma once

#include <string>

struct Console_Pimpl; //A pimpl is used so that the ncurses C library is
					  //perfectly encapsulated in the cpp file and can't
					  //leak out of this header file. This is desired
					  //since the ncurses library uses a lot of macros,
					  //global functions, and global variables.

class Console
{
	Console_Pimpl* private_members;
public:
	enum class Color
	{
		Black,
        Red,
        Green,
        Yellow,
        Blue,
        Magenta,
        Cyan,
        White
	};

	Console();
	~Console();
	
	void write_character(int row, int column, char c, int frame = 0);
	void write_string(int row, int column, const std::string & s, int frame = 0);
	
	void set_fgcolor(Color color);
	void set_bgcolor(Color color);
	
	int get_width(int frame = 0);
	int get_height(int frame = 0);
	
	void refresh();
	
	void add_frame(int height, int width, int x, int y, bool height_locked, bool width_locked, bool word_wrap);
};
