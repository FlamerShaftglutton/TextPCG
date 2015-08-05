#include "Console.hpp"
#include <ncurses.h>
#include <string>
#include <vector>
#include "string_utils.hpp"
#ifdef DEBUG
	#include "Log.hpp"
#endif

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
	int echo_frame;
	int echo_color_pair;
	bool echo_enabled;
	std::string echo_string;
	Console::Color saved_fg_color,
				   saved_bg_color;
	std::vector<std::string> commands;
	int command_spot;
};

Console::Console()
{
	//initialize the ncurses library
	initscr();
	start_color();
	clear();
	noecho();
	cbreak();
	keypad(stdscr, TRUE);
	nodelay(stdscr,TRUE);
	curs_set(0);

	//fill in private members pimpl
	private_members = new Console_Pimpl();
	private_members->current_bg_color = Console::Color::Black;
	private_members->current_fg_color = Console::Color::White;
	private_members->echo_frame = 0;
	set_echo_colors(Console::Color::White, Console::Color::Black);
	private_members->echo_enabled = false;
	private_members->echo_string = "";
	save_colors();
	private_members->command_spot = -1;
	
	//fill in the color table
	for (int i = 0; i < 64; ++i)
	{
		init_pair(i+1,i&7,i>>3);
	}
	
	#ifdef DEBUG
		Log::write("COLORS: " + StringUtils::to_string(COLORS) + ", COLOR_PAIRS: " + StringUtils::to_string(COLOR_PAIRS));
	#endif
}

Console::~Console()
{
	endwin();
	delete private_members;
}

std::string Console::check_for_input()
{
	std::string retval = "";
	int c = getch();
	
	if (private_members->echo_enabled)
	{
		//check for special keys!
		if (c == KEY_UP)
		{
			#ifdef DEBUG
				Log::write("User pressed the UP arrow.");
			#endif
			/*
			//grab the previous thing in the command list
			if (private_members->command_spot < 0)
			{
				private_members->command_spot = (int)private_members->commands.size();
			}
			
			if (private_members->command_spot > 0)
			{
				private_members->echo_string = private_members->commands[--private_members->command_spot];
			}
			*/
			
			retval = "<up_arrow_key>";
		}
		else if (c == KEY_DOWN)
		{
			#ifdef DEBUG
				Log::write("User pressed the DOWN arrow.");
			#endif
			/*
			//grab the next thing in the command list
			if (private_members->command_spot >= 0)
			{
				++private_members->command_spot;
				if (private_members->command_spot >= (int)private_members->commands.size())
				{
					private_members->echo_string = "";
					private_members->command_spot = (int)private_members->commands.size();
				}
				else
				{
					private_members->echo_string = private_members->commands[private_members->command_spot];
				}
			}
			*/
			
			retval = "<down_arrow_key>";
		}
		else if (c == KEY_LEFT)
		{
			#ifdef DEBUG
				Log::write("User pressed the LEFT arrow.");
			#endif
			
			retval = "<left_arrow_key>";
		}
		else if (c == KEY_RIGHT)
		{
			#ifdef DEBUG
				Log::write("User pressed the LEFT arrow.");
			#endif
			
			retval = "<right_arrow_key>";
		}
		//check for the backspace key
		else if (c == KEY_BACKSPACE || c == 127)
		{
			if (private_members->echo_string.length() > 0)
			{
				private_members->echo_string.pop_back();
			}
		}
		//check for the enter key
		else if (c == KEY_ENTER || c == '\n')
		{
			retval = private_members->echo_string;
			private_members->echo_string = "";
			
			if (private_members->commands.empty() || private_members->commands.back() != retval)
			{
				private_members->commands.push_back(retval);
			}
			private_members->command_spot = -1;
		}
		//check for normal keys
		else if (c >= 0)
		{
			private_members->echo_string += (char)c;
		}
	}
	else if (c >= 0)
	{
		retval += (char)c;
	}
	
	return retval;
}

std::string Console::get_last_n_lines(std::string input, unsigned n_lines, int frame)
{
	std::vector<std::string> lines = format_string_to_lines(input,get_width(frame), frame != 0 && private_members->frames[frame-1].word_wrap);
	std::string retval = "";
	
	//if the user requested more lines than the string has, just return the original string
	if (lines.size() <= n_lines)
		return input;
	
	//otherwise, grab only the last n lines
	for (unsigned i = lines.size() - n_lines; i < lines.size(); ++i)
	{
		retval += lines[i] + "\n";
	}
	
	//remove the trailing newline character
	retval.pop_back();
	
	//return the shortened string
	return retval;
}

void Console::write_character(int row, int column, char c, int frame)
{
	if (frame == 0)
		mvaddch(row, column, c);
	else
		mvaddch(row + private_members->frames[frame - 1].y, column + private_members->frames[frame - 1].x, c);
}

void Console::write_string(int row, int column, const std::string& input, int frame)
{
	//get the frame or window stats
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
	
	//save the current colors in case some formatting in the string changes them
	save_colors();
	
	//get a list of lines
	std::vector<std::string> lines = format_string_to_lines(input,f_width - column, word_wrap);
	
	//loop through each line
	for (unsigned l = 0; l < lines.size(); ++l)
	{
		//get the current line
		std::string line = lines[l];
		
		//loop through each character!
		for (unsigned i = 0; i < line.length(); ++i)
		{
			/*
			//if there's a new line, break!
			if (s[i] == '\n')
			{
				//first off, clear the rest of the line
				for (int j = current_x; j < f_width; ++j)
				{
					mvaddch(y_offset + row, x_offset + j, ' ');
				}
				
				//now actually skip to the next line
				++row;
				current_x = column;
				continue;
			}
			
			//check for color tags
			else */if (line[i] == '<' && (i == 0 || line[i-1] != '\\'))
			{
				//continue eating until we find the closing tag
				std::string tag_front = "";
				std::string tag_back  = "";
				
				//grab the front of the tag (everything to the left of the equal sign)
				for (++i;i<line.length() && line[i] != '>' && line[i] != '='; tag_front += line[i],++i);
				
				//if there is no equal sign, then this is a comment
				if (line[i] != '=')
				{
					//just skip to the closing bracket
					for(;i<line.length() && line[i] != '>'; ++i);
					
					//now skip past it
					continue;
				}
				//if this isn't a comment...
				else
				{
					//...then grab the rest of the tag
					for (++i;i<line.length() && line[i] != '>'; tag_back += line[i], ++i);
					
					//format the strings for easier parsing
					tag_front = StringUtils::to_lowercase(tag_front);
					tag_back = StringUtils::to_lowercase(tag_back);
					
					//and decide what to do with the tag
					if (tag_front == "fg" || tag_front == "bg")
					{
						//get the color name from the back of the tag
						bool valid_color = true;
						Console::Color co = Console::Color::Black;
						if (tag_back == "black")
							co = Console::Color::Black;
						else if (tag_back == "red")
							co = Console::Color::Red;
						else if (tag_back == "green")
							co = Console::Color::Green;
						else if (tag_back == "yellow")
							co = Console::Color::Yellow;
						else if (tag_back == "blue")
							co = Console::Color::Blue;
						else if (tag_back == "magenta" || tag_back == "purple")
							co = Console::Color::Magenta;
						else if (tag_back == "cyan")
							co = Console::Color::Cyan;
						else if (tag_back == "white")
							co = Console::Color::White;
						else
							valid_color = false;
						
						//if it's a valid color, do something with it!
						if (valid_color)
						{
							if (tag_front == "fg")
							{
								set_fgcolor(co);
							}
							else
							{
								set_bgcolor(co);
							}
						}
					}
					
					//continue to the next iteration of the loop
					continue;
				}
			}
			/*
			//if we've gone too far then check for word wrapping
			else if (current_x >= f_width)
			{
				if (word_wrap)
				{
					//first off, look backwards for a space to break on instead of this character
					std::size_t last_space_position = current_line.find_last_of(' ');
					
					if (last_space_position != std::string::npos)
					{
						//push the first chunk on as a line
						lines.push_back(current_line.substr(0,last_space_position));
						
						//if the space wasn't the last character
						if (last_space_position < (current_line.length()-1))
						{
							//then clip the current_line
							current_line = current_line.substr(last_space_position + 1);
							
							//and finally recalculate the length
							current_width = 0;
							for (std::size_t p = 0; p < current_line.length(); ++p)
							{
								//if it's a tag, skip ahead
								if (current_line[p] == '<' && (p == 0 || current_line.substr(p-1,2) != R"(\<)"))//check for an escaped bracket "\<"
								{
									for (;p<current_line.length() && current_line[p] != '>';++p);
								}
								//if it's not a tag then it won't be a newline, so log it
								else
								{
									++current_width;
								}
							}
						}
						//if the space was the last character then this is easy
						else
						{
							current_line = "";
							current_width = 0;
						}
					}
					else
					{
						lines.push_back(current_line);
						current_line = "";
						current_width = 0;
					}
				
					++row;
					current_x = column;
				}
				else
				{
					//no need to clear the rest of the line here, as we've already filled the row!
					continue;
				}
			}
			*/
			
			//if we've gone too far vertically then just quit
			if (row >= f_height)
			{
				break;
			}
			
			//actually write out the character!
			mvaddch(y_offset + row, x_offset + current_x, line[i]);
			++current_x;
		}
		
		//clear to the end of the line
		for (int j = current_x; j < f_width; ++j)
		{
			mvaddch(y_offset + row, x_offset + column + j, ' ');
		}
		
		//move to the next line
		if (l < (lines.size() - 1))
		{
			++row;
			current_x = column;
		}
	}
	
	//restore the colors from before
	restore_colors();
}

void Console::set_fgcolor(Color color)
{
	private_members->current_fg_color = color;
	attrset(COLOR_PAIR(1 + (((int)(private_members->current_bg_color)<<3) | (int)(private_members->current_fg_color))));
}

void Console::set_bgcolor(Color color)
{
	private_members->current_bg_color = color;
	attrset(COLOR_PAIR(1 + (((int)(private_members->current_bg_color)<<3) | (int)(private_members->current_fg_color))));
}

void Console::save_colors()
{
	private_members->saved_fg_color = private_members->current_fg_color;
	private_members->saved_bg_color = private_members->current_bg_color;
}

void Console::restore_colors()
{
	private_members->current_fg_color = private_members->saved_fg_color;
	set_bgcolor(private_members->saved_bg_color);
}

void Console::set_echo_frame(int frame)
{
	private_members->echo_enabled = true;
	private_members->echo_frame = frame;
}

void Console::set_echo_colors(Console::Color fg_color, Console::Color bg_color)
{
	private_members->echo_color_pair = 1 + (((int)(bg_color)<<3) | (int)(fg_color));
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
	//automatically echo first, if that's enabled
	if (private_members->echo_enabled)
	{
		//save the current colors
		save_colors();
	
		//set the colors
		attrset(COLOR_PAIR(private_members->echo_color_pair));
		
		//write the string
		write_string(0,0,private_members->echo_string,private_members->echo_frame);
		
		//blank the rest of the line
		for (int i = private_members->echo_string.length(); i < get_width(private_members->echo_frame); ++i)
		{
			write_character(0,i,' ',private_members->echo_frame);
		}
		
		//restore the colors
		restore_colors();
	}
	
	//actually refresh the screen
	wrefresh(stdscr);
}

void Console::clear(int frame)
{
	if (frame == 0)
	{
		erase();
	}
	else
	{
		for (int i = 0; i < get_width(frame); ++i)
		{
			for (int j = 0; j < get_height(frame); ++j)
			{
				write_character(j,i,' ',frame);
			}
		}
	}
}

int Console::add_frame(int height, int width, int y, int x, bool height_locked, bool width_locked, bool word_wrap)
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
	
	return (int)private_members->frames.size();
}

std::vector<std::string> Console::format_string_to_lines(std::string input, int frame_width, bool line_wraps)
{
	std::vector<std::string> lines;
	std::string current_line;
	//int frame_width = get_width(frame);
	int current_width = 0;
	//bool line_wraps = (frame != 0 && private_members->frames[frame-1].word_wrap);

	for (std::size_t i = 0; i < input.length(); ++i)
	{
		//first off, don't count any formatting tags (<fg=color> or <bg=color>)
		if (input[i] == '<' && (i == 0 || input.substr(i-1,2) != R"(\<)"))//check for an escaped bracket "\<"
		{
			//skip ahead to a closing brace
			for (;i<input.length() && input[i] != '>';current_line += input[i],++i);
			
			current_line += '>';//add the closing brace
		}
		//now check if it's a newline character
		else if (input[i] == '\n')
		{
			lines.push_back(current_line);
			current_line = "";
			current_width = 0;
		}
		//if it's not a tag or newline character, add it in
		else
		{
			//skip to the next line if necessary
			if (line_wraps && current_width >= frame_width)
			{
				//first off, look backwards for a space to break on instead of this character
				std::size_t last_space_position = current_line.find_last_of(' ');
				
				if (last_space_position != std::string::npos)
				{
					//push the first chunk on as a line
					lines.push_back(current_line.substr(0,last_space_position));
					
					//if the space wasn't the last character
					if (last_space_position < (current_line.length()-1))
					{
						//then clip the current_line
						current_line = current_line.substr(last_space_position + 1);
						
						//and finally recalculate the length
						current_width = 0;
						for (std::size_t p = 0; p < current_line.length(); ++p)
						{
							//if it's a tag, skip ahead
							if (current_line[p] == '<' && (p == 0 || current_line.substr(p-1,2) != R"(\<)"))//check for an escaped bracket "\<"
							{
								for (;p<current_line.length() && current_line[p] != '>';++p);
							}
							//if it's not a tag then it won't be a newline, so log it
							else
							{
								++current_width;
							}
						}
					}
					//if the space was the last character then this is easy
					else
					{
						current_line = "";
						current_width = 0;
					}
				}
				else
				{
					lines.push_back(current_line);
					current_line = "";
					current_width = 0;
				}
			}
			
			//now add the character to the current string
			++current_width;
			current_line += input[i];
		}
	}
	
	//at the end there may be more text still in the current_line variable
	lines.push_back(current_line);
	
	return lines;
}
