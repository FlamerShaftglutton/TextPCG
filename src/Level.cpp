#include "Level.hpp"
#include "Room.hpp"
#include "Object.hpp"

#ifdef DEBUG
	#include "Log.hpp"
	#include "string_utils.hpp"
#endif

Level::Level(int level_width, int level_height) :width(level_width),height(level_height)
{
	for (int i = 0; i < (level_width * level_height); ++i)
	{
		rooms.push_back(nullptr);
	}
}

Level::~Level()
{
	cleanup_objects();

	for(Room* r : rooms)
	{
		if (r != nullptr)
		{
			delete r;
		}
	}
	
	for(Object* o : objects)
	{
		if (o != nullptr)
		{
			delete o;
		}
	}
}

Room* Level::get_room(ECS::Handle h)
{
	if (h < 0 || h >= (ECS::Handle)rooms.size())
	{
		return nullptr;
	}
	
	return rooms[h];
}

Room* Level::get_room(int x, int y)
{
	if (x >= width || x < 0 || y < 0 || y >= height)
	{
		return nullptr;
	}
	
	return rooms[y * width + x];
}

int Level::get_width()
{
	return width;
}

int Level::get_height()
{
	return height;
}

unsigned Level::get_num_rooms()
{
	return rooms.size();
}

unsigned Level::get_num_objects()
{
	return objects.size();
}

ECS::Handle Level::create_room(int x, int y)
{
	if (get_room(x,y) != nullptr)
	{
		return -1;
	}
	else
	{
		ECS::Handle h = (ECS::Handle)(y * width + x); 
		rooms[h] = new Room(h,x,y); 
		return h;
	}
}

ECS::Handle Level::create_object()
{
	ECS::Handle h = (ECS::Handle)objects.size(); 
	objects.push_back(new Object(h)); 
	return h;
}

Object* Level::get_object(ECS::Handle h)
{
	if (h < 0 || h >= (ECS::Handle)objects.size())
	{
		return nullptr;
	} 
	
	return objects[(int)h];
}

void Level::destroy_object(ECS::Handle h, bool immediately) 
{
	if (!immediately)
	{
		objects_to_destroy.push_back(h);
	}
	else
	{
		if (h > 0 && h < (ECS::Handle)objects.size() && objects[(int)h] != nullptr)
		{ 
			Object* o = objects[(int)h];
			
			if (o->room_container >= 0)
			{
				Room* r = get_room(o->room_container);
				
				for (auto i = r->objects().begin(); i != r->objects().end(); ++i)
				{
					if (*i == h)
					{
						r->objects().erase(i);
						break;
					}
				}
			}
			
			if (o->object_container >= 0)
			{
				Object* r = get_object(o->object_container);
				
				for (auto i = r->objects.begin(); i != r->objects.end(); ++i)
				{
					if (*i == h)
					{
						r->objects.erase(i);
						break;
					}
				}
			}
			
			delete objects[(int)h]; objects[(int)h] = nullptr;
		}
	}
}

void Level::cleanup_objects()
{
	for (ECS::Handle h : objects_to_destroy)
	{
		destroy_object(h);
	}
	
	objects_to_destroy.clear();
}

ECS::Handle Level::get_open_neighbor(ECS::Handle room, Room::Exit direction)
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