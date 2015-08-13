#include "PCG.hpp"
#include "GameState.hpp"
#include "Level.hpp"
#include "Room.hpp"
#include "Object.hpp"
#include "CombatData.hpp"
#include "Handle.hpp"
#include "mymath.hpp"

#include <vector>

struct node
{
	ECS::Handle handle;
	
	node* children[4];
	node* parent;
	
	bool locked;
	
	int count_children()
	{
		int count = 1;//count yourself
		
		for (std::size_t i = 0; i < 4; ++i)
		{
			if (children[i] != nullptr && !children[i]->locked)
			{
				count += children[i]->count_children();
			}
		}
		
		return count;
	}
	
	bool is_leaf()
	{
		return children[0] == children[1] && children[0] == children[2] && children[0] == children[3] && children[0] == nullptr;
	}
}

void PCG::create_world(GameState& gs)
{
	//set up the basic stuff
	gs.level = new Level(50,50);
	gs.main_text = "";
	gs.main_text_dirty_flag = true;
	gs.frames_elapsed = 0;
	
	//start filling it in
	node* starting_node = new node;
	starting_node->parent = nullptr;
	starting_node->children[0] = starting_node->children[1] = starting_node->children[2] = starting_node->children[3] = nullptr;
	starting_node->locked = false;
	starting_node->handle = gs.level->create_room(gs.level->get_width() / 2, gs.level->get_height() / 2);
	Room* sr = gs.level->get_room(starting_node->handle);
	//sr->set_description("A small room with unremarkable features. Seems like a good place to start.");
	//sr->set_short_description("Starting Room");
	sr->set_minimap_symbol("<bg=green> ");
	
	std::vector<node*> stack;
	stack.push_back(starting_node);
	
	int num_rooms = 1;
	
	while(!stack.empty())
	{
		//grab a random spot
		unsigned spot = (unsigned)MyMath::random_int(0,(int)stack.size() - 1);
		if (spot != stack.size() - 1)
		{
			node* t = stack[spot];
			stack[spot] = stack.back();
			stack.back() = t;
		}
		node* n = stack.back();
		stack.pop_back();
		Room* r = gs.level->get_room(n.handle);
		int rx,ry;
		r->get_xy(rx,ry);
		
		//now that we have a spot, decide if we're going to add any children
		if (num_rooms < 20)
		{
			//first off, find out how many children we can have
			int max_children = 0;
			if (ry > 1 && gs.level->get_room(ry-1,rx) == nullptr)
			{
				++max_children;
			}
			if (rx > 1 && gs.level->get_room(ry,rx-1) == nullptr)
			{
				++max_children;
			}
			if (rx < (gs.level->get_width() - 2) && gs.level->get_room(ry, rx + 1) == nullptr)
			{
				++max_children;
			}
			if (ry < (gs.level->get_height() - 2) && gs.level->get_room(ry + 1, rx) == nullptr)
			{
				++max_children;
			}
			
			//if we can't have kids, just bail
			if (max_children > 0)
			{
				//update the description
				r->set_short_description("A Hallway");
				r->set_description("A corridor leading to other rooms.");
			
				//if we can have kids, have a random amount!
				int num_kids = MyMath::random_int(1,max_children);
				
				//shuffle the list of directions
				int dirs[] = {1,2,4,8};
				for (int j = 0; j < 4; ++j)
				{
					int tt = dirs.back();
					int ss = MyMath::random_int(0,3);
					dirs.back() = dirs[ss];
					dirs[ss] = tt;
				}
				
				//for each kid...
				for (int i = 0; i < num_kids; ++i)
				{
					int x_off = (dirs[i] & 3) == 2 ? -1 : dirs[i] & 3;
					int y_off = ((dirs[i] >> 2) & 3) == 2 ? -1 : ((dirs[i] >> 2) & 3);
					
					if (ry + y_off > 1 && ry + y_off < (gs.level->get_height() - 2) &&
						rx + x_off > 1 && rx + x_off < (gs.level->get_width()  - 2) &&
						gs.level->get_room(ry + y_off, rx + x_off) == nullptr)
					{
						//update the number of rooms
						++num_rooms;
						
						//set stuff in the node
						node* child = new node;
						child->handle = gs.level->create_room(rx + x_off, ry + y_off);
						child->children[0] = child->children[1] = child->children[2] = child->children[3] = nullptr;
						child->locked = false;
						child->parent = n;
						
						//set stuff in the room
						Room* rr = gs.level->get_room(child->handle);
						rr->set_short_description("A Small Room");
						rr->set_description("A <fg=green>leaf<fg=white> node, seemingly.");
						
						//set exits in the rooms
						rr->set_exit(dirs[i] == 1 ? Room::Exit::WEST : dirs[i] == 2 ? Room::Exit::EAST : dirs[i] == 4 ? Room::Exit::NORTH : Room::Exit::SOUTH,true);
						r->set_exit(dirs[i] == 1 ? Room::Exit::EAST : dirs[i] == 2 ? Room::Exit::WEST : dirs[i] == 4 ? Room::Exit::SOUTH : Room::Exit::NORTH,true);
						
						//set stuff in the parent node
						n->children[i] = child;
						
						//add this child to the stack
						stack.push_back(child);
					}
				}
			}
		}
	}
	
	//At this point we should have a map full of rooms and a tree full of nodes. Time to start placing locks and keys.
	//create the last key
	ECS::Handle kh = gs.level->create_object();
	Object* ko = gs.level->get_object(kh);
	ko->visible = false;
	ko->visible_in_short_description = false;
	ko->friendly = true;
	ko->mobile = false;
	ko->playable = false;
	ko->open = false;
	ko->holdable = true;
	ko->object_container = -1;
	ko->hitpoints = -1;
	ko->attack = 0;
	ko->hit_chance = 0.0f;
	ko->name = "The Final Key";
	ko->description = "You won the game!";
	ko->scripts.construct("","(say \"You have won the game! Nicely done!\");","(say \"You have won the game! Nicely done!\");","");

	//actually put the key in a random leaf-node room
	node* random_node = starting_node;
	
	while (!random_node->is_leaf())
	{
		for (int i = 0; i < 4; ++i)
		{
			if (random_node->children[i] != nullptr)
			{
				random_node = random_node->children[i];
				break;
			}
		}
	}
	ko->room_container = random_node->handle;
	
	for (int i = 0; i < 3; ++i)
	{
		//figure out which room to lock
		int steps_up = MyMath::random_int(0,2);
		for (unsigned i = 0; i < steps_up && random_node->parent != starting_node && random_node->count_children() < 5; ++i)
		{
			random_node = random_node->parent;
		}
		
		//lock the door between this room and the parent room
		random_room->locked = true;
		int px,py,cx,cy;
		Room* pr = gs.level->get_room(random_room->parent->handle);
		pr->get_xy(px,py);
		gs.level->get_room(random_room->handle)->get_xy(cx,cy);
		int x_off = cx - px,
			y_off = cy - py;
		Room::Exit e = x_off == 0 && y_off == -1 ? Room::Exit::NORTH : x_off == 0 && y_off == 1 ? Room::Exit::SOUTH : x_off == -1 ? Room::Exit::West : Room::Exit::East;
		pr->set_exit(e,false);
		
		//update the room's description
		std::string direction_names[] = {"North","East","South","West"};
		pr->set_description(pr->get_description() + "\nA <fg=green>door<fg=white> leads to the " + direction_names[(int)e] + ", but it's locked.");
		
		//now place another key!
		random_node = starting_node;
		
		while (!random_node->is_leaf())
		{
			//shuffle the directions
			int dirs[] = {0,1,2,3};
			for (int j = 0; j < 4; ++j)
			{
				int tt = dirs.back();
				int ss = MyMath::random_int(0,3);
				dirs.back() = dirs[ss];
				dirs[ss] = tt;
			}
			
			//go a random direction
			for (int j = 0; j < 4; ++i)
			{
				if (random_node->children[dirs[j]] != nullptr)
				{
					random_node = random_node->children[dirs[j]];
					break;
				}
			}
		}
		
		//put in a key (color matching???)
		kh = gs.level->create_object();
		ko = gs.level->get_object(kh);
		ko->visible = false;
		ko->visible_in_short_description = false;
		ko->friendly = true;
		ko->mobile = false;
		ko->playable = false;
		ko->open = false;
		ko->holdable = true;
		ko->object_container = -1;
		ko->room_container = random_node->handle;
		ko->hitpoints = -1;
		ko->attack = 0;
		ko->hit_chance = 0.0f;
		ko->name = "A Key";
		ko->description = "A skeleton key. Find the door it belongs to!";
		ko->scripts.construct("","","","");//fill in the use script to open the last lock we put in
	}
}