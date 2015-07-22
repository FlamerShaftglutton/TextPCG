#pragma once

#include "Room.hpp"
#include "Handle.hpp"
#include "Object.hpp"
#include <vector>

#ifdef DEBUG
	#include "Log.hpp"
	#include "string_utils.hpp"
#endif

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
	
	ECS::Handle get_open_neighbor(ECS::Handle room, Room::Exit direction)
	{
		//first off, just check out if this room has an exit that direction
		if(get_room(room)->get_exit(direction))
		{
			//if there is, get the room
			ECS::Handle next_room = get_room(room)->get_special_exit(direction);
			if (next_room < 0)
			{
				int current_room_x, current_room_y;
				get_room(room)->get_xy(current_room_x, current_room_y);
				
				int x_modifier = direction == Room::Exit::SOUTH || direction == Room::Exit::NORTH ? 0 : direction == Room::Exit::EAST ? 1 : -1;
				int y_modifier = direction == Room::Exit::EAST || direction == Room::Exit::WEST ? 0 : direction == Room::Exit::SOUTH ? 1 : -1;
				
				next_room = get_room(current_room_x + x_modifier, current_room_y + y_modifier)->get_handle();
				
			}
			
			#ifdef DEBUG
				if (next_room < 0)
				{
					//alert the programmer that there is an exit with no valid endpoint
					std::string words[] = {"NORTH","EAST","SOUTH","WEST","UP","DOWN","SECRET0","SECRET1","SECRET2","INVALID"};				
					Log::write("WARNING: the room specified can't be found! Details: Handle=" + StringUtils::to_string(room) + ", Direction: " + words[static_cast<std::size_t>(direction)]);
				}
			#endif
			
			return next_room;
		}
		else
			return -1;
	}
};
