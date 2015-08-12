#pragma once

#include "Room.hpp"
#include "Handle.hpp"
//#include "Scripting.hpp"
#include <vector>

//forward declarations
struct Object;
//unfortunately we can't forward declare Room because of the Exit enum and C++ being dumb

class Level
{
	std::vector<Room*> rooms;
	std::vector<Object*> objects;
	std::vector<ECS::Handle> objects_to_destroy;
	int width;
	int height;
	
public:
	Level(int level_width, int level_height);
	~Level();
	
	Room* get_room(ECS::Handle h);
	Room* get_room(int x, int y);
	
	int get_width();
	int get_height();
	
	unsigned get_num_rooms();
	unsigned get_num_objects();
	
	ECS::Handle create_room(int x, int y);
	ECS::Handle create_object();
	
	Object* get_object(ECS::Handle h);
	void destroy_object(ECS::Handle h, bool immediately = true);
	void cleanup_objects();
	
	ECS::Handle get_open_neighbor(ECS::Handle room, Room::Exit direction);
};
