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
	
	std::string check_for_input();
	std::string get_last_n_lines(std::string input, unsigned n_lines, int frame = 0);
	
	void write_character(int row, int column, char c, int frame = 0);
	void write_string(int row, int column, const std::string & s, int frame = 0);
	
	void set_fgcolor(Color color);
	void set_bgcolor(Color color);
	
	void save_colors();
	void restore_colors();
	
	void set_echo_frame(int frame);
	void set_echo_colors(Color fg_color, Color bg_color);
	
	int get_width(int frame = 0);
	int get_height(int frame = 0);
	
	void refresh();
	void clear(int frame = 0);
	
	int add_frame(int height, int width, int x, int y, bool height_locked, bool width_locked, bool word_wrap);
};
