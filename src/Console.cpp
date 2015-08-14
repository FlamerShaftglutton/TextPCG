#include "Console.hpp"
#include <ncurses.h>
#include <string>
#include <vector>
#include "string_utils.hpp"
#ifdef DEBUG
	#include "Log.hpp"
#endif


///////////////////////////////////////////////////////////////////////////////
//
// Console
//
///////////////////////////////////////////////////////////////////////////////
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
	
	//fill in the color table
	for (int i = 0; i < 64; ++i)
	{
		init_pair(i+1,i&7,i>>3);
	}
	
	//set the index for which frameset we're currently displaying
	frameset_index = 0;
}

Console::~Console()
{
	//clean up ncurses stuff
	endwin();
}

std::string Console::check_for_input()
{
	std::string retval = "";
	int c = getch();
	Console::FrameSet& fs = framesets[frameset_index];
	
	if (fs.get_echo_frame() >= 0)
	{
		//check for special keys!
		if (c == KEY_UP)
		{
			#ifdef DEBUG
				Log::write("User pressed the UP arrow.");
			#endif
			
			if (fs.get_command_history_enabled())
			{
				fs.previous_command();
			}
			else
			{
				retval = "<up_arrow_key>";
			}
		}
		else if (c == KEY_DOWN)
		{
			#ifdef DEBUG
				Log::write("User pressed the DOWN arrow.");
			#endif
			
			if (fs.get_command_history_enabled())
			{
				fs.next_command();
			}
			else
			{
				retval = "<down_arrow_key>";
			}
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
			std::string s = fs.get_echo_text();
			if (s.length() > 0)
			{
				s.pop_back();
				fs.set_echo_text(s);
			}
		}
		//check for the enter key
		else if (c == KEY_ENTER || c == '\n')
		{
			retval = fs.get_echo_text();
			fs.set_echo_text("");

			fs.push_command(retval);
		}
		//check for normal keys
		else if (c >= 0)
		{
			fs.set_echo_text(fs.get_echo_text() + (char)c);
		}
	}
	else if (c >= 0)
	{
		retval += (char)c;
	}
	
	return retval;
}

std::string Console::get_last_n_lines(std::string input, unsigned n_lines, int width, bool word_wrap)
{
	std::vector<std::string> lines = format_string_to_lines(input,width, word_wrap);
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

std::vector<std::string> Console::format_string_to_lines(std::string input, int frame_width, bool line_wraps)
{
	std::vector<std::string> lines;
	std::string current_line;
	//int frame_width = get_width(frame);
	int current_width = 0;
	//bool line_wraps = (frame != 0 && frames[frame-1].word_wrap);

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

int Console::get_frameset_by_name(std::string name) const
{
	for (int i = 0; i < (int)framesets.size(); ++i)
	{
		if (framesets[i].get_name() == name)
		{
			return i;
		}
	}
	
	return -1;
}

int Console::get_current_frameset_index() const
{
	return frameset_index;
}

Console::FrameSet& Console::get_current_frameset()
{
	return framesets[frameset_index];
}

Console::FrameSet& Console::get_frameset(int index)
{
	return framesets[index];
}

void Console::switch_to_frameset(int index)
{
	if (index < (int)framesets.size())
	{
		frameset_index = index;
	}
	#ifdef DEBUG
	else
	{
		Log::write("Warning: tried to access an undefined frameset. Check that the index '" + StringUtils::to_string(index) + "' is valid.");
	}
	#endif
}

int Console::add_frameset(Console::FrameSet fs)
{
	int retval = (int)framesets.size();
	
	framesets.push_back(fs);
	
	return retval;
}

int Console::get_width()
{
	getmaxyx(stdscr,t_y,t_x);
	return t_x;
}

int Console::get_height()
{
	getmaxyx(stdscr,t_y,t_x);
	return t_y;
}


///////////////////////////////////////////////////////////////////////////////
//
// Console::FrameSet
//
///////////////////////////////////////////////////////////////////////////////

Console::FrameSet::FrameSet(std::string p_name)
{
	name = p_name;
	
	current_bg_color = saved_bg_color = echo_bg_color = Console::Color::Black;
	current_fg_color = saved_fg_color = echo_fg_color = Console::Color::White;
	echo_frame = 0;
	echo_enabled = false;
	echo_text = "";
	command_spot = -1;
	command_history_enabled = false;
}

Console::FrameSet::FrameSet(const Console::FrameSet& fs)
{
	for (unsigned i = 0; i < fs.frames.size(); ++i)
	{
		frames.push_back(fs.frames[i]);
	}
	
	for (unsigned i = 0; i < fs.commands.size(); ++i)
	{
		commands.push_back(fs.commands[i]);
	}
	
	name = fs.name;
	echo_frame = fs.echo_frame;
	echo_enabled = fs.echo_enabled;
	echo_text = fs.echo_text;
	command_history_enabled = fs.command_history_enabled;
	command_spot = fs.command_spot;
	current_fg_color = fs.current_fg_color;
	current_bg_color = fs.current_bg_color;
	saved_fg_color = fs.saved_fg_color;
	saved_bg_color = fs.saved_bg_color;
	echo_fg_color = fs.echo_fg_color;
	echo_bg_color = fs.echo_bg_color;
}

Console::FrameSet& Console::FrameSet::operator=(const Console::FrameSet& fs) &
{
	for (unsigned i = 0; i < fs.frames.size(); ++i)
	{
		frames.push_back(fs.frames[i]);
	}
	
	for (unsigned i = 0; i < fs.commands.size(); ++i)
	{
		commands.push_back(fs.commands[i]);
	}
	
	name = fs.name;
	echo_frame = fs.echo_frame;
	echo_enabled = fs.echo_enabled;
	echo_text = fs.echo_text;
	command_history_enabled = fs.command_history_enabled;
	command_spot = fs.command_spot;
	current_fg_color = fs.current_fg_color;
	current_bg_color = fs.current_bg_color;
	saved_fg_color = fs.saved_fg_color;
	saved_bg_color = fs.saved_bg_color;
	echo_fg_color = fs.echo_fg_color;
	echo_bg_color = fs.echo_bg_color;
	
	return *this;
}
		
std::string Console::FrameSet::get_name() const
{
	return name;
}

Console::Frame& Console::FrameSet::operator[](int i)
{
	return frames[i];
}

int Console::FrameSet::get_frame_index_by_name(std::string name) const
{
	for (int i = 0; i < (int)frames.size(); ++i)
	{
		if (frames[i].name == name)
		{
			return i;
		}
	}
	
	return -1;
}

int Console::FrameSet::add_frame(const Console::Frame & frame)
{
	int retval = (int)frames.size();
	
	Console::Frame f(frame.name, frame.height, frame.width, frame.y, frame.x, frame.height_locked, frame.width_locked, frame.word_wrap);
	
	frames.push_back(f);
	
	return retval;
}

void Console::FrameSet::set_echo_frame(int frame, Console::Color fg_color, Console::Color bg_color)
{
	echo_enabled = true;
	echo_frame = frame;
	echo_fg_color = fg_color;
	echo_bg_color = bg_color;
}

int Console::FrameSet::get_echo_frame() const
{
	return echo_enabled ? echo_frame : -1;
}

std::string Console::FrameSet::get_echo_text() const
{
	return echo_text;
}

void Console::FrameSet::set_echo_text(std::string text)
{
	echo_text = text;
}

void Console::FrameSet::set_command_history_enabled(bool enabled)
{
	command_history_enabled = enabled;
}

bool Console::FrameSet::get_command_history_enabled() const
{
	return command_history_enabled;
}

void Console::FrameSet::previous_command()
{
	//grab the previous thing in the command list
	if (command_spot < 0)
	{
		command_spot = (int)commands.size();
	}
	
	if (command_spot > 0)
	{
		echo_text = commands[--command_spot];
	}
}

void Console::FrameSet::next_command()
{
	++command_spot;
	
	if (command_spot >= (int)commands.size())
	{
		echo_text = "";
		command_spot = (int)commands.size();
	}
	else
	{
		echo_text = commands[command_spot];
	}
}

void Console::FrameSet::push_command(std::string command)
{
	if (commands.empty() || commands.back() != command)
	{
		commands.push_back(command);
	}
	command_spot = -1;
}

void Console::FrameSet::write_character(int row, int column, char c, int frame)
{
	if (frame < 0)
		mvaddch(row, column, c);
	else
		mvaddch(row + frames[frame].y, column + frames[frame].x, c);
}

void Console::FrameSet::write_string(int row, int column, const std::string& input, int frame)
{
	//get the frame or window stats
	int x_offset = 0,
		y_offset = 0;
	
	int f_width, f_height;
	
	int current_x = column;
	
	bool word_wrap = false;
	
	if (frame < 0)
	{
		getmaxyx(stdscr,f_height,f_width);
	}
	else
	{
		Console::Frame& f = frames[frame];
		
		f_width = f.width;
		f_height = f.height;
		
		x_offset = f.x;
		y_offset = f.y;
		
		word_wrap = f.word_wrap;
	}
	
	//save the current colors in case some formatting in the string changes them
	save_colors();
	
	//get a list of lines
	std::vector<std::string> lines = Console::format_string_to_lines(input,f_width - column, word_wrap);
	
	//loop through each line
	for (unsigned l = 0; l < lines.size(); ++l)
	{
		//get the current line
		std::string line = lines[l];
		
		//loop through each character!
		for (unsigned i = 0; i < line.length(); ++i)
		{
			//check for color tags
			if (line[i] == '<' && (i == 0 || line[i-1] != '\\'))
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
						#ifdef DEBUG
						else
						{
							Log::write("Warning: tried to change " + tag_front + " to color '" + tag_back + "' which hasn't been implemented. No colors changed.");
						}
						#endif
					}
					#ifdef DEBUG
					else
					{
						Log::write("Warning: Tag '<" + tag_front + "=" + tag_back + ">' is not a comment and couldn't be parsed.");
					}
					#endif
					
					//continue to the next iteration of the loop
					continue;
				}
			}
			
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

void Console::FrameSet::clear(int frame)
{
	if (frame < 0)
	{
		erase();
	}
	else
	{
		for (int i = 0; i < frames[frame].width; ++i)
		{
			for (int j = 0; j < frames[frame].height; ++j)
			{
				write_character(j,i,' ',frame);
			}
		}
	}
}

void Console::FrameSet::set_fgcolor(Console::Color color)
{
	current_fg_color = color;
	attrset(COLOR_PAIR(1 + (((int)(current_bg_color)<<3) | (int)(current_fg_color))));
}

void Console::FrameSet::set_bgcolor(Console::Color color)
{
	current_bg_color = color;
	attrset(COLOR_PAIR(1 + (((int)(current_bg_color)<<3) | (int)(current_fg_color))));
}

void Console::FrameSet::save_colors()
{
	saved_fg_color = current_fg_color;
	saved_bg_color = current_bg_color;
}

void Console::FrameSet::restore_colors()
{
	current_fg_color = saved_fg_color;
	set_bgcolor(saved_bg_color);
}

void Console::FrameSet::refresh()
{
	//automatically echo first, if that's enabled
	if (echo_enabled)
	{
		//save the current colors
		save_colors();
	
		//set the colors
		set_fgcolor(echo_fg_color);
		set_bgcolor(echo_bg_color);
		
		//write the string
		std::string o = echo_text;
		write_string(0,0,o.append(frames[echo_frame].width - echo_text.length(),' '), echo_frame);//pads it with spaces to blank out whatever was already there
		
		//restore the colors
		restore_colors();
	}
	
	//actually refresh the screen
	wrefresh(stdscr);
}


///////////////////////////////////////////////////////////////////////////////
//
// Frame
//
///////////////////////////////////////////////////////////////////////////////
Console::Frame::Frame()
{
	name = "";
	height = 0;
	width = 0;
	y = 0;
	x = 0;
	height_locked =
	width_locked = 
	word_wrap = false;
}

Console::Frame::Frame(std::string p_name, int p_height, int p_width, int p_y, int p_x, bool p_height_locked, bool p_width_locked, bool p_word_wrap)
{
	name = p_name;
	height = p_height;
	width = p_width;
	y = p_y;
	x = p_x;
	height_locked = p_height_locked;
	width_locked = p_width_locked;
	word_wrap = p_word_wrap;
}



























/*

void Console::write_character(int row, int column, char c, int frame)
{
	if (frame == 0)
		mvaddch(row, column, c);
	else
		mvaddch(row + frames[frame - 1].y, column + frames[frame - 1].x, c);
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
		Frame& f = frames[frame - 1];
		
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
			//check for color tags
			if (line[i] == '<' && (i == 0 || line[i-1] != '\\'))
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

void Console::refresh()
{
	//automatically echo first, if that's enabled
	if (echo_enabled)
	{
		//save the current colors
		save_colors();
	
		//set the colors
		attrset(COLOR_PAIR(echo_color_pair));
		
		//write the string
		write_string(0,0,echo_text,echo_frame);
		
		//blank the rest of the line
		for (int i = echo_text.length(); i < get_width(echo_frame); ++i)
		{
			write_character(0,i,' ',echo_frame);
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
	
	frames.push_back(f);
	
	return (int)frames.size();
}
*/