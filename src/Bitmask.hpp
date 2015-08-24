#pragma once

#include <vector>
#include <string>

struct Position
{
	int x = 0;
	int y = 0;
	
	Position() { }
	Position(int p_x, int p_y) : x(p_x), y(p_y) { }
};

class Bitmask
{
	Position offset;
	std::vector<bool> values;
	int width;
	int height;
public:
	Bitmask();
	Bitmask(std::string mask, Position p_offset);
	Bitmask(int p_width, int p_height, Position p_offset);
	
	Position get_offset();
	
	int get_width();
	int get_height();
	
	bool get(Position p);
	bool get(int x, int y);

	bool operator()(Position p);
	bool operator()(int x, int y);
	
	void set(Position p, bool val);
	void set(int x, int y, bool val);
	
	void set_offset(Position o);
	void set_offset(int x, int y);
	
	Bitmask operator-(Bitmask& rhs);
	Bitmask operator+(Bitmask& rhs);
	
	bool overlaps(Bitmask& rhs);
	bool touches(Bitmask& rhs);
	std::vector<Position> shortest_path(Position p0, Position p1);
	std::vector<Position> random_path(Position p0, Position p1, float randomness);
	
	#ifdef DEBUG
		std::string to_string();
	#endif
};