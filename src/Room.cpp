#include "Room.hpp"
#include <string>
#include "Handle.hpp"
#include <vector>
#ifdef DEBUG
	#include "string_utils.hpp"
	#include "Log.hpp"
#endif

Room::Room(ECS::Handle handle, int x_coordinate, int y_coordinate)
{
	my_handle = handle;
	x = x_coordinate;
	y = y_coordinate;

	for (int i = 0; i < (int)Exit::INVALID; ++i)
	{
		special_exits[i] = -1;
		exits[i] = Exit_Status::Wall;
		door_descriptions[i] = "";
	}
}

Room::~Room()
{

}

Room::Exit_Status Room::get_exit(Room::Exit e)
{
	if (e == Exit::INVALID)
	{
		return Exit_Status::Wall;
	}

	return exits[static_cast<std::size_t>(e)];
}

void Room::set_exit(Room::Exit e, Room::Exit_Status status)
{
	if (e != Exit::INVALID)
	{
		exits[static_cast<std::size_t>(e)] = status;
	}
}

ECS::Handle Room::get_special_exit(Room::Exit e)
{
	return special_exits[static_cast<std::size_t>(e)];
}

void Room::set_special_exit(Room::Exit e, ECS::Handle h)
{
	if (e != Exit::INVALID)
	{
		special_exits[static_cast<std::size_t>(e)] = h;
	}
}


ECS::Handle Room::get_handle()
{
	return my_handle;
}

std::string Room::get_short_description()
{
	return short_description;
}
void Room::set_short_description(std::string s)
{
	short_description = s;
}

std::string Room::get_minimap_symbol()
{
	return minimap_symbol;
}
void Room::set_minimap_symbol(std::string s)
{
	minimap_symbol = s;
}

std::string Room::get_description()
{
	return description;
}
void Room::set_description(std::string s)
{
	description = s;
}

std::string Room::get_door_description(Room::Exit e)
{
	return door_descriptions[static_cast<std::size_t>(e)];
}

void Room::set_door_description(std::string s, Room::Exit e)
{
	if (e != Exit::INVALID)
	{
		door_descriptions[static_cast<std::size_t>(e)] = s;
	}
}

std::string Room::get_description_with_doors()
{
	std::string direction_names[] = {"North", "East", "South", "West", "Up", "Down", "Secret0", "Secret1", "Secret2", "Invalid"};
	std::string retval = description;
	for (std::size_t i = 0; i < static_cast<std::size_t>(Exit::INVALID); ++i)
	{
		//if we have a door description, add it in
		if (door_descriptions[i] != "")
		{
			std::string c = "green>open";
			if (get_exit(static_cast<Exit>(i)) == Exit_Status::Locked)
			{
				c = "red>locked";
			}
			#ifdef DEBUG
			else if (get_exit(static_cast<Exit>(i)) == Exit_Status::Wall)
			{
				Log::write("Warning: The room '" + StringUtils::to_string(static_cast<int>(my_handle)) + "' has a door but the door is neither locked nor open. Just counting it as 'open'.");
			}
			#endif
			
			retval += "<fg=white><bg=black>\nTo the " + direction_names[i] + " is " + door_descriptions[i] + "<fg=white><bg=black>. It is <fg=" + c + "<fg=white>.";
		}
		//if there is no door description but the way is still locked, just say there's a generic door
		else if (exits[i] == Exit_Status::Locked)
		{
			retval += "<fg=white><bg=black>\nTo the " + direction_names[i] + " is an unremarkable door. It is <fg=red>locked<fg=white>.";
		}
	}
	
	return retval;
}

std::vector<ECS::Handle>& Room::objects()
{
	return objs;
}

void Room::get_xy(int& xx, int& yy)
{
	xx = x;
	yy = y;
}

bool Room::get_visited()
{
	return visited;
}

void Room::set_visited(bool b)
{
	visited = b;
}