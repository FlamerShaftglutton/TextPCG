#pragma once
#include <string>
#include "Handle.hpp"
#include <vector>
#ifdef DEBUG
	#include "string_utils.hpp"
	#include "Log.hpp"
#endif

class Room
{
	int exits = 0;
	ECS::Handle my_handle;
	ECS::Handle special_exits[9];//size of the Exit enum
	std::string short_description;
	std::string description;
	int x;
	int y;
	std::vector<ECS::Handle> objs;
	std::string minimap_symbol;
	bool visited = false;
	
public:
	enum class Exit
	{
		NORTH = 0,
		EAST,
		SOUTH,
		WEST,
		UP,
		DOWN,
		SECRET0,
		SECRET1,
		SECRET2,
		INVALID
	};
	
	Room(ECS::Handle handle, int x_coordinate, int y_coordinate) { my_handle = handle; x = x_coordinate; y = y_coordinate; for(int i=0;i<9;++i){special_exits[i] = -1;} }
	~Room() { }
	
	inline bool get_exit(Exit e) { if (e == Exit::INVALID) { return false; } return ((exits & (1 << static_cast<std::size_t>(e))) != 0); }
	inline void set_exit(Exit e, bool open) { if (e != Exit::INVALID) { exits |= ((open ? 1 : 0) << static_cast<std::size_t>(e)); } }
	
	inline ECS::Handle get_special_exit(Exit e) { return special_exits[static_cast<std::size_t>(e)]; }
	inline void set_special_exit(Exit e, ECS::Handle h) { special_exits[static_cast<std::size_t>(e)] = h; }
	
	inline ECS::Handle get_handle() { return my_handle; }
	
	inline std::string get_short_description() { return short_description; }
	inline void set_short_description(std::string s) { short_description = s; }
	
	inline std::string get_minimap_symbol() { return minimap_symbol; }
	inline void set_minimap_symbol(std::string s) { minimap_symbol = s; }
	
	inline std::string get_description() { return description; }
	inline void set_description(std::string s) { description = s; }
	
	inline std::vector<ECS::Handle>& objects() { return objs; }
	
	inline void get_xy(int& xx, int& yy) { xx = x; yy =y; }
	
	inline bool get_visited() { return visited; }
	inline void set_visited(bool b) { visited = b; }
};
