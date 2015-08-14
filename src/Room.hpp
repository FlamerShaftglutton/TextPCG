#pragma once
#include <string>
#include "Handle.hpp"
#include <vector>

class Room
{
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
	
	enum class Exit_Status
	{
		Open,
		Locked,
		Wall
	};
	
	Room(ECS::Handle handle, int x_coordinate, int y_coordinate);
	~Room();
	
	Exit_Status get_exit(Exit e);
	void set_exit(Exit e, Exit_Status status);
	
	ECS::Handle get_special_exit(Exit e);
	void set_special_exit(Exit e, ECS::Handle h);
	
	ECS::Handle get_handle();
	
	std::string get_short_description();
	void set_short_description(std::string s);
	
	std::string get_minimap_symbol();
	void set_minimap_symbol(std::string s);
	
	std::string get_description();
	void set_description(std::string s);
	std::string get_description_with_doors();
	
	std::string get_door_description(Exit e);
	void set_door_description(std::string s, Exit e);
	
	std::vector<ECS::Handle>& objects();
	
	void get_xy(int& xx, int& yy);
	
	bool get_visited();
	void set_visited(bool b);

private:
	ECS::Handle my_handle;
	Exit_Status exits[(std::size_t)Exit::INVALID];//size of the Exit enum
	ECS::Handle special_exits[(std::size_t)Exit::INVALID];//size of the Exit enum
	std::string short_description;
	std::string description;
	std::string door_descriptions[(std::size_t)Exit::INVALID];//size of the Exit enum
	int x;
	int y;
	std::vector<ECS::Handle> objs;
	std::string minimap_symbol;
	bool visited = false;
	
};
