#include "Bitmask.hpp"
#include <vector>
#include <string>
#include <forward_list>
#include <cstdlib>
#include "string_utils.hpp"
#include "mymath.hpp"

#ifdef DEBUG
	#include "Log.hpp"
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

struct pf_node
{
	pf_node* parent;
	Position p;
	int distance_from_start;
	int estimated_distance_from_end;
};

std::vector<Position> Bitmask::shortest_path(Position p0, Position p1)
{
	std::vector<Position> retval;
	
	//prime the pump
	auto insert_into_sorted_list = [] (std::forward_list<pf_node*>& l, pf_node* p)
	{
		auto pi = l.before_begin();
		for (auto i = l.begin(); i != l.end(); pi = i, ++i)
		{
			if (p->distance_from_start + p->estimated_distance_from_end <= (*i)->distance_from_start + (*i)->estimated_distance_from_end)
			{
				return l.insert_after(pi,p);
			}
		}
		return l.insert_after(pi, p);
	};
	
	auto is_in_list = [] (std::forward_list<pf_node*>& l, Position p)
	{
		auto pi = l.before_begin();
		for (auto i = l.begin(); i != l.end(); pi = i, ++i)
		{
			if ((*i)->p.x == p.x && (*i)->p.y == p.y)
			{
				return pi;
			}
		}
		return l.end();
	};
	
	//auto comparator = [](pf_node* lhs, pf_node* rhs){ return lhs->distance_from_start + lhs->estimated_distance_from_end > rhs->distance_from_start + rhs->estimated_distance_from_end; };
	auto g = [](Position start, Position end) { return std::abs(start.x - end.x) + std::abs(start.y - end.y); };
	//std::priority_queue<pf_node*, std::vector<pf_node*>, decltype(comparator)> open(comparator);
	std::forward_list<pf_node*> open;
	std::vector<pf_node*> closed(width * height, nullptr);
	pf_node* starting_node = new pf_node;
	starting_node->parent = nullptr;
	starting_node->p = p0;
	starting_node->distance_from_start = 0;
	starting_node->estimated_distance_from_end = g(p0,p1);
	
	open.push_front(starting_node);
	
	//start the loop
	while (!open.empty())
	{
		//grab the top thing off the queue
		pf_node* node = open.front();
		open.pop_front();
		closed[node->p.x - offset.x + (node->p.y - offset.y) * width] = node;
		
		//now check if we've reached the end
		if (node->p.x == p1.x && node->p.y == p1.y)
		{
			//create a list of directions (in reverse order)
			while (node->parent != nullptr)
			{
				retval.push_back(node->p);
				node = node->parent;
			}
			
			//add in the starting node, since that won't be in there yet
			retval.push_back(starting_node->p);
		}
		//if we haven't reached the end, add all of our children to the open list
		else
		{
			Position dirps[] = {{0,-1},{1,0},{0,1},{-1,0}};
			for (int d = 0; d < 4; ++d)
			{
				if (get(node->p.x + dirps[d].x, node->p.y + dirps[d].y) && closed[node->p.x + dirps[d].x - offset.x + (node->p.y + dirps[d].y - offset.y) * width] == nullptr)
				{
					Position p = node->p;
					p.x += dirps[d].x;
					p.y += dirps[d].y;
					
					//if this node already exists, update it
					auto ip = is_in_list(open, p);
					if (ip != open.end())
					{
						//check if we need to update it!
						auto tip = ip;
						++tip;
						
						if ((*tip)->distance_from_start > node->distance_from_start + 1)
						{
							pf_node* to_move = *tip;
							
							open.erase_after(ip);
							
							to_move->distance_from_start = node->distance_from_start + 1;
							
							insert_into_sorted_list(open,to_move);
						}
					}
					//if we haven't added this node to the open list yet, then add it!
					else
					{
						pf_node* to_create = new pf_node;
						to_create->parent = node;
						to_create->p = p;
						to_create->distance_from_start = node->distance_from_start + 1;
						to_create->estimated_distance_from_end = g(p,p1);
						
						insert_into_sorted_list(open,to_create);
					}
				}
			}
		}
	}
	
	//clean up
	for (pf_node* p : closed)
	{
		delete p;
	}
	closed.clear();
	for (pf_node* p : open)
	{
		delete p;
	}
	open.clear();
	
	//reverse and return the list of positions
	return std::vector<Position>(retval.rbegin(),retval.rend());
}

std::vector<Position> Bitmask::random_path(Position p0, Position p1, float randomness)
{
	std::vector<Position> retval;
	
	//prime the pump
	auto insert_into_sorted_list = [] (std::forward_list<pf_node*>& l, pf_node* p)
	{
		auto pi = l.before_begin();
		for (auto i = l.begin(); i != l.end(); pi = i, ++i)
		{
			if (p->distance_from_start + p->estimated_distance_from_end <= (*i)->distance_from_start + (*i)->estimated_distance_from_end)
			{
				return l.insert_after(pi,p);
			}
		}
		return l.insert_after(pi, p);
	};
	
	auto is_in_list = [] (std::forward_list<pf_node*>& l, Position p)
	{
		auto pi = l.before_begin();
		for (auto i = l.begin(); i != l.end(); pi = i, ++i)
		{
			if ((*i)->p.x == p.x && (*i)->p.y == p.y)
			{
				return pi;
			}
		}
		return l.end();
	};
	
	auto g = [](Position start, Position end) { return std::abs(start.x - end.x) + std::abs(start.y - end.y); };
	std::forward_list<pf_node*> open;
	std::vector<pf_node*> closed(width * height, nullptr);
	pf_node* starting_node = new pf_node;
	starting_node->parent = nullptr;
	starting_node->p = p0;
	starting_node->distance_from_start = 0;
	starting_node->estimated_distance_from_end = g(p0,p1);

	float min_dis = starting_node->estimated_distance_from_end * randomness;
	std::vector<int> weights;
	for (int i = 0; i < 256; ++i)
	{
		weights.push_back(i);
	}
	for (int i = 255; i > 0; --i)
	{
		int pos = MyMath::random_int(0,i);
		int ti = weights.back();
		weights.back() = weights[pos];
		weights[pos] = ti;
	}
	
	open.push_front(starting_node);
	
	//start the loop
	while (!open.empty())
	{
		//grab the top thing off the queue
		pf_node* node = open.front();
		open.pop_front();
		closed[node->p.x - offset.x + (node->p.y - offset.y) * width] = node;
		
		//now check if we've reached the end
		if (node->p.x == p1.x && node->p.y == p1.y)
		{
			//create a list of directions (in reverse order)
			while (node->parent != nullptr)
			{
				retval.push_back(node->p);
				node = node->parent;
			}
			
			//add in the starting node, since that won't be in there yet
			retval.push_back(starting_node->p);
		}
		//if we haven't reached the end, add all of our children to the open list
		else
		{
			Position dirps[] = {{0,-1},{1,0},{0,1},{-1,0}};
			
			for (int d = 0; d < 4; ++d)
			{
				if (get(node->p.x + dirps[d].x, node->p.y + dirps[d].y) && closed[node->p.x + dirps[d].x - offset.x + (node->p.y + dirps[d].y - offset.y) * width] == nullptr)
				{
					Position p = node->p;
					p.x += dirps[d].x;
					p.y += dirps[d].y;
					
					//if this node doesn't already exists, add it
					if (is_in_list(open, p) == open.end())
					{
						pf_node* to_create = new pf_node;
						to_create->parent = node;
						to_create->p = p;
						to_create->distance_from_start = node->distance_from_start + 1; 
						to_create->estimated_distance_from_end = g(p,p1);
						to_create->estimated_distance_from_end += (int)((float)(weights[(to_create->p.x + weights[(to_create->p.y + weights[(p.x + weights[p.y & 255]) & 255]) & 255]) & 255] * std::abs(to_create->distance_from_start + to_create->estimated_distance_from_end - (int)min_dis)) / min_dis);
						
						insert_into_sorted_list(open,to_create);
					}
				}
			}
		}
	}
	
	//clean up
	for (pf_node* p : closed)
	{
		delete p;
	}
	closed.clear();
	for (pf_node* p : open)
	{
		delete p;
	}
	open.clear();
	
	//reverse and return the list of positions
	return std::vector<Position>(retval.rbegin(),retval.rend());
}

#ifdef DEBUG
std::string Bitmask::to_string()
{
	std::string retval = "";
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			retval += values[x + y * width] ? '1' : '0';
		}
		retval += '\n';
	}
	return retval;
}
#endif
