#include "PCG.hpp"
#include "GameState.hpp"
#include "Level.hpp"
#include "Room.hpp"
#include "Object.hpp"
#include "CombatData.hpp"
#include "Handle.hpp"
#include "mymath.hpp"
#include "string_utils.hpp"

#include <vector>
#include <functional>

#ifdef DEBUG
	#include "Log.hpp"
#endif

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
};

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
		Room* r = gs.level->get_room(n->handle);
		int rx,ry;
		r->get_xy(rx,ry);
		
		//now that we have a spot, decide if we're going to add any children
		if (num_rooms < 20)
		{
			//first off, find out how many children we can have
			int max_children = 0;
			if (ry > 1 && gs.level->get_room(rx, ry - 1) == nullptr)
			{
				++max_children;
			}
			if (rx > 1 && gs.level->get_room(rx - 1, ry) == nullptr)
			{
				++max_children;
			}
			if (rx < (gs.level->get_width() - 2) && gs.level->get_room(rx + 1, ry) == nullptr)
			{
				++max_children;
			}
			if (ry < (gs.level->get_height() - 2) && gs.level->get_room(rx, ry + 1) == nullptr)
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
					int tt = dirs[3];
					int ss = MyMath::random_int(0,2);
					dirs[3] = dirs[ss];
					dirs[ss] = tt;
				}
				
				//for each kid...
				for (int i = 0; i < num_kids; ++i)
				{
					int x_off = (dirs[i] & 3) == 2 ? -1 : dirs[i] & 3;
					int y_off = ((dirs[i] >> 2) & 3) == 2 ? -1 : ((dirs[i] >> 2) & 3);
					
					if (ry + y_off > 1 && ry + y_off < (gs.level->get_height() - 2) &&
						rx + x_off > 1 && rx + x_off < (gs.level->get_width()  - 2) &&
						gs.level->get_room(rx + x_off, ry + y_off) == nullptr)
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
						rr->set_minimap_symbol("<bg=yellow> ");
						
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
	
	while (!random_node->is_leaf() || random_node == starting_node)
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
	gs.level->get_room(random_node->handle)->objects().push_back(ko->get_handle());
	
	//set up some colors for the doors
	std::vector<std::string> color_names = { "red", "green", "yellow", "blue", "magenta", "cyan"};
	
	//make locks and keys!
	for (int i = 0; i < 3; ++i)
	{
		//figure out which room to lock
		int steps_up = 1;//MyMath::random_int(0,2);
		for (int i = 0; i < steps_up && random_node->parent != starting_node && random_node->count_children() < 5; ++i)
		{
			random_node = random_node->parent;
		}
		
		//lock the door between this room and the parent room
		random_node->locked = true;
		int px,py,cx,cy;
		Room* pr = gs.level->get_room(random_node->parent->handle);
		pr->get_xy(px,py);
		gs.level->get_room(random_node->handle)->get_xy(cx,cy);
		int x_off = cx - px,
			y_off = cy - py;
		Room::Exit e = x_off == 0 && y_off == -1 ? Room::Exit::NORTH : x_off == 0 && y_off == 1 ? Room::Exit::SOUTH : x_off == -1 ? Room::Exit::WEST : Room::Exit::EAST;
		pr->set_exit(e,false);
		
		//pick a color for the door
		std::size_t c_pos = MyMath::random_int(0,color_names.size()-1);
		std::string door_color = color_names[c_pos];
		color_names[c_pos] = color_names.back();
		color_names.pop_back();
		
		//update the room's description
		std::string direction_names[] = {"North","East","South","West"};
		pr->set_description(pr->get_description() + "\nA large door leads to the " + direction_names[(int)e] + ", but it's locked. It glows <fg=" + door_color + ">" + door_color + "<fg=white>.");
		
		//now place another key!
		//random_node = starting_node;
		std::vector<node*> available_leaves;
		std::function<void(node*)> add_leaves = [&](node* mn) { if (mn->is_leaf()) { available_leaves.push_back(mn); } else { for (int z = 0; z < 4; ++z) { if (mn->children[z] != nullptr && !mn->children[z]->locked) { add_leaves(mn->children[z]); } } } };
		
		add_leaves(starting_node);
		
		#ifdef DEBUG
		if (available_leaves.size() == 0)
		{
			Log::write("Derp, no available nodes!");
		}
		#endif
		random_node = available_leaves[MyMath::random_int(0,available_leaves.size() - 1)];
		
		//make a key
		kh = gs.level->create_object();
		ko = gs.level->get_object(kh);
		ko->visible = true;
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
		ko->name = "A <fg=" + door_color + ">Key";
		ko->description = "A skeleton key. It glows <fg=" + door_color + ">" + door_color + "<fg=white>. Find the door it belongs to!";
		ko->scripts.construct("",
							  "",
							  "(if (= (get current_room.handle) " + StringUtils::to_string((int)pr->get_handle()) + ") (+ (set current_room.description \"<fg=white><bg=black>A small, cramped room. To the " + direction_names[(int)e] + " is an unlocked door.\") (set global.main_text (+ (get global.main_text) \"<fg=white><bg=black>\n\nThe key opens a door to the " + direction_names[(int)e] + ".\")) (set current_room.open_" + StringUtils::to_lowercase(direction_names[(int)e].substr(0,1)) + " true) (destroy (get caller.handle))) (set global.main_text (+ (get global.main_text) \"<fg=white><bg=black>\n\nIt does nothing\")));",
							  "");
		
		//put the key in the room
		gs.level->get_room(ko->room_container)->objects().push_back(ko->get_handle());
	}
	
	//finally, make the player in the first room!
	gs.playable_character = gs.level->create_object();
	Object* o = gs.level->get_object(gs.playable_character);
	o->visible = true;
	o->visible_in_short_description = true;
	o->friendly = true;
	o->mobile = true;
	o->playable = true;
	o->open = true;
	o->holdable = false;
	o->room_container = sr->get_handle();
	o->object_container = -1;
	o->hitpoints = 100;
	o->attack = 10;
	o->hit_chance = 0.75f;
	o->name = "Myself";
	o->description = "A <fg=red>hideous<fg=white> looking human. Possibly beaten, or possibly just always ugly. Hard to tell.";
	o->scripts.construct("","","","");
	sr->objects().push_back(o->get_handle());
}
