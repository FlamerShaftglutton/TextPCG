#include "Bitmask.hpp"
#include <vector>
#include <string>

#ifdef DEBUG
	#include "Log.hpp"
	#include "string_utils.hpp"
#endif

Bitmask::Bitmask()
{
	width = height = offset.x = offset.y = 0;
}

Bitmask::Bitmask(std::string mask, Position p_offset)
{
	//the string will be a series of 0's and 1's that are split by newlines
	std::vector<std::string> lines = StringUtils::split(mask, '\n');
	for (std::size_t i = 0; i < lines.size(); ++i)
	{
		lines[i] = StringUtils::trim(lines[i]);
	}
	
	width = lines[0].length();
	height = lines.size();
	
	offset = p_offset;
	
	values.assign(width * height, false);
	
	for (std::size_t y = 0; y < lines.size(); ++y)
	{
		#ifdef DEBUG
		if ((int)lines[y].length() != width)
		{
			Log::write("Error: Bitmask defined in string does not have uniform dimensions!");
		}
		#endif
		
		for (std::size_t x = 0; x < lines[y].length(); ++x)
		{
			values[x + y * width] = lines[y][x] != '0';
		}
	}
}

Bitmask::Bitmask(int p_width, int p_height, Position p_offset)
{
	offset = p_offset;
	width = p_width;
	height = p_height;
	
	values.assign(width * height, false);
}

Position Bitmask::get_offset()
{
	return offset;
}

int Bitmask::get_width()
{
	return width;
}

int Bitmask::get_height()
{
	return height;
}

bool Bitmask::get(int x, int y)
{
	return x >= offset.x && x < offset.x + width &&
		   y >= offset.y && y < offset.y + height &&
		   values[x - offset.x + (y - offset.y) * width];
}

bool Bitmask::get(Position p)
{
	return get(p.x,p.y);
}

bool Bitmask::operator()(int x, int y)
{
	return get(x,y);
}

bool Bitmask::operator()(Position p)
{
	return get(p);
}

void Bitmask::set(int x, int y, bool val)
{
	if (x >= offset.x && x < offset.x + width && y >= offset.y && y < offset.y + height)
	{
		values[x - offset.x + (y - offset.y) * width] = val;
	}
}

void Bitmask::set(Position p, bool val)
{
	set(p.x, p.y, val);
}

Bitmask Bitmask::operator-(Bitmask& rhs)
{
	//first up, figure out the dimensions for the new one
	Position no;
	no.x = offset.x < rhs.offset.x ? offset.x : rhs.offset.x;
	no.y = offset.y < rhs.offset.y ? offset.y : rhs.offset.y;
	
	int w = -no.x + (offset.x + width >= rhs.offset.x + rhs.width ? offset.x + width : rhs.offset.x + rhs.width);
	int h = -no.y + (offset.y + height >= rhs.offset.y + rhs.height ? offset.y + height : rhs.offset.y + rhs.height);
	
	Bitmask retval(w,h,no);
	
	for (int y = 0; y < h; ++y)
	{
		for (int x = 0; x < w; ++x)
		{
			retval.values[x + y * w] = get(no.x + x, no.y + y) && !rhs(no.x + x, no.y + y);
		}
	}
	
	return retval;
}

Bitmask Bitmask::operator+(Bitmask& rhs)
{
	//first up, figure out the dimensions for the new one
	Position no;
	no.x = offset.x < rhs.offset.x ? offset.x : rhs.offset.x;
	no.y = offset.y < rhs.offset.y ? offset.y : rhs.offset.y;
	
	int w = -no.x + (offset.x + width >= rhs.offset.x + rhs.width ? offset.x + width : rhs.offset.x + rhs.width);
	int h = -no.y + (offset.y + height >= rhs.offset.y + rhs.height ? offset.y + height : rhs.offset.y + rhs.height);
	
	Bitmask retval(w,h,no);
	
	for (int y = 0; y < h; ++y)
	{
		for (int x = 0; x < w; ++x)
		{
			retval.values[x + y * w] = get(no.x + x, no.y + y) || rhs(no.x + x, no.y + y);
		}
	}
	
	return retval;
}

bool Bitmask::overlaps(Bitmask& rhs)
{
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			if (get(x + offset.x, y + offset.y) && rhs(x + offset.x, y + offset.y))
			{
				return true;
			}
		}
	}
	
	return false;
}

bool Bitmask::touches(Bitmask& rhs)
{
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			//if this space is filled in...
			if (get(x + offset.x, y + offset.y))
			{
				//check in all 5 directions (overlapping being the fifth)
				if (rhs(x + offset.x, y + offset.y) || 
					rhs(x + offset.x, y + offset.y - 1) ||
					rhs(x + offset.x + 1, y + offset.y) ||
					rhs(x + offset.x, y + offset.y + 1) ||
					rhs(x + offset.x - 1, y + offset.y))
				{
					return true;
				}
			}
		}
	}
	
	return false;
}

void Bitmask::set_offset(Position o)
{
	set_offset(o.x, o.y);
}

void Bitmask::set_offset(int x, int y)
{
	offset.x = x;
	offset.y = y;
}