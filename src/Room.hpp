#pragma once
#include <string>
#include "Handle.hpp"
#include <vector>

class Room
{
	int exits = 0;
	ECS::Handle my_handle;
	ECS::Handle special_exits[9];//size of the Exit enum
	std::string description;
	int x;
	int y;
	std::vector<ECS::Handle> objs;
	
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
		SECRET2
	};
	
	Room(ECS::Handle handle, int x_coordinate, int y_coordinate) { my_handle = handle; x = x_coordinate; y = y_coordinate; for(int i=0;i<9;++i){special_exits[i] = -1;} }
	~Room() { }
	
	inline bool get_exit(Exit e) { return ((exits & (1 << static_cast<std::size_t>(e))) != 0); }
	inline void set_exit(Exit e, bool open) { exits &= ((open ? 1 : 0) << static_cast<std::size_t>(e)); }
	
	inline ECS::Handle get_special_exit(Exit e) { return special_exits[static_cast<std::size_t>(e)]; }
	inline void set_special_exit(Exit e, ECS::Handle h) { special_exits[static_cast<std::size_t>(e)] = h; }
	
	inline ECS::Handle get_handle() { return my_handle; }
	
	inline std::string get_description() { return description; }
	inline void set_description(std::string s) { description = s; }
	
	inline std::vector<ECS::Handle>& objects() { return objs; }
};
