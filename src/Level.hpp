#pragma once

#include "Room.hpp"
#include "Handle.hpp"
#include "Object.hpp"
#include <vector>

class Level
{
	std::vector<Room*> rooms;
	std::vector<Object*> objects;
	int width;
	int height;
	
public:
	Level(int level_width, int level_height) :width(level_width),height(level_height) { for (int i = 0; i < (level_width * level_height); ++i){ rooms.push_back(nullptr); } }
	~Level() { for(Room* r : rooms) { delete r; } for(Object* o : objects) { delete o; } }
	
	inline Room* get_room(ECS::Handle h) { if (h < 0 || h >= (ECS::Handle)rooms.size()) { return nullptr;} else { return rooms[h]; } }
	inline Room* get_room(int x, int y) { if (x >= width || x < 0 || y < 0 || y >= height) {return nullptr;} else { return rooms[y * width + x]; } }
	
	inline ECS::Handle create_room(int x, int y) {if (get_room(x,y) != nullptr){return -1;} else { ECS::Handle h = (ECS::Handle)(y * width + x); rooms[h] = new Room(h,x,y); return h;} }
	inline ECS::Handle create_object() { auto h = (ECS::Handle)objects.size(); objects.push_back(new Object(h)); return h; }
	
	inline Object* get_object(ECS::Handle h) { if (h < 0 || h >= (ECS::Handle)objects.size()) { return nullptr;} else {return objects[h]; } }
	
};
