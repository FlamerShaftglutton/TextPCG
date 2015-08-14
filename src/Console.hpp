#pragma once

#include <string>
#include <vector>


class Console
{
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
	
	struct Frame
	{
		std::string name;
		int height,width;
		int y,x;
		
		bool height_locked, width_locked,
			 word_wrap;
		
		Frame();
		Frame(std::string p_name, int p_height, int p_width, int p_y, int p_x, bool p_height_locked, bool p_width_locked, bool p_word_wrap);
	};

	class FrameSet
	{
		std::vector<Frame> frames;
		std::string name;
		
		int echo_frame;
		bool echo_enabled;
		std::string echo_text;
		
		bool command_history_enabled;
		std::vector<std::string> commands;
		int command_spot;
		
		Console::Color current_fg_color,
					   current_bg_color;

		Console::Color saved_fg_color,
					   saved_bg_color;
					   
		Console::Color echo_fg_color,
					   echo_bg_color;
	public:
		FrameSet(std::string p_name);
		FrameSet();
		FrameSet(const FrameSet& fs);
		FrameSet(FrameSet&&) = default;
		FrameSet& operator=(const FrameSet& fs) &;
		FrameSet& operator=(FrameSet&&) & = default;  
		
		std::string get_name() const;
		
		Frame& operator[](int i);
		
		int get_frame_index_by_name(std::string name) const;
		int add_frame(const Frame& frame);
		
		void set_echo_frame(int frame, Console::Color fg_color, Console::Color bg_color);
		int get_echo_frame() const;
		std::string get_echo_text() const;
		void set_echo_text(std::string text);
		
		void set_command_history_enabled(bool enabled);
		bool get_command_history_enabled() const;
		void previous_command();
		void next_command();
		void push_command(std::string command);
		
		void write_character(int row, int column, char c, int frame = -1);
		void write_string(int row, int column, const std::string& input, int frame = -1);
		
		void clear(int frame = -1);
		
		void set_fgcolor(Color color);
		void set_bgcolor(Color color);
		
		void save_colors();
		void restore_colors();
		
		void refresh();
	};

	Console();
	~Console();

	std::string check_for_input();
	static std::string get_last_n_lines(std::string input, unsigned n_lines, int width, bool word_wrap);
	static std::vector<std::string> format_string_to_lines(std::string input, int frame_width, bool line_wraps);
	
	int get_frameset_by_name(std::string name) const;
	int get_current_frameset_index() const;
	FrameSet& get_current_frameset();
	FrameSet& get_frameset(int index);
	void switch_to_frameset(int index);
	int add_frameset(FrameSet fs);

	int get_width();
	int get_height();
private:
	std::vector<FrameSet> framesets;
	int frameset_index;
	int t_x,t_y;
};
