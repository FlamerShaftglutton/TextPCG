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
	gs.level = new Level(1000,1000);
	gs.main_text = "";
	gs.main_text_dirty_flag = true;
	gs.frames_elapsed = 0;
	
	//create a couple generic enemies
	ECS::Handle enemy_1 = gs.level->create_object();
	Object* enemy_object = gs.level->get_object(enemy_1);
	enemy_object->visible = true;
	enemy_object->visible_in_short_description = true;
	enemy_object->friendly = false;
	enemy_object->mobile = true;
	enemy_object->playable = false;
	enemy_object->open = false;
	enemy_object->holdable = false;
	enemy_object->room_container = -1;
	enemy_object->object_container = -1;
	enemy_object->hitpoints = 12;
	enemy_object->attack = 1;
	enemy_object->hit_chance = 0.0f;
	enemy_object->name = "An Evil Goblin";
	enemy_object->description = "An ugly creature with beady bloodthirsty eyes.";

	enemy_object->scripts.construct("(set 0 0);",
						 "(say (choose (random 0 3) \"SCREEE!!!\" \"GEEYAH!!!\" \"Derp?\" \"RAAAH!!!\"));",
						 "(set main_text (+ (get main_text) \"\n<fg=white><bg=black>You can't use an enemy!\"));",
						 "(if (get combat.vulnerable_to_attack) (+ \"\" (set 0 2) (set main_text (+ (get main_text) \"<fg=green><bg=black>\nThe goblin reels back from your attack!\")) (set combat.player_position_far_front true) ) (+ \"\" (if (get combat.player_attacking) (set 0 3)) (choose (get 0) (if (get combat.player_position_front) (+ \"\" (set main_text (+ (get main_text) \"<fg=white><bg=black>\nThe goblin raises his sword over his head!\")) (set 0 1) (defend false false true true)) (+ \"\" (set main_text (+ (get main_text) \"<fg=white><bg=black>\n\" (if (get combat.player_position_far_front) \"The goblin stalks towards you!\" \"The goblin turns towards you!\"))) (set combat.player_position_front true) (defend true true true true) (attack false false false false)))  (if (or (get combat.player_position_front) (get combat.player_position_far_front)) (+ \"\" (set 0 0) (attack false false true true) (set main_text (+ (get main_text) \"<fg=white><bg=black>\nThe goblin slashes you with its sword!\")) (defend true true true true)) (+ \"\" (set 0 0) (set main_text (+ (get main_text) \"<fg=white><bg=black>\nThe goblin's sword clinks to the ground where you used to be standing!\")) (defend false false true true)))  (+ \"\" (set 0 0) (set main_text (+ (get main_text) \"<fg=white><bg=black>\nThe goblin recovers and holds his sword in a defensive position.\")) (defend true true true true)) (+ \"\" (set 0 0) (set main_text (+ (get main_text) \"<fg=white><bg=black>\nThe goblin blocks your attack!\")) (defend true true true true))))); ");
	/*
	ECS::Handle enemy_1_spawner = gs.level->create_object();
	Object* spawner = gs.level->get_object(enemy_1_spawner);
	spawner->visible = false;
	spawner->visible_in_short_description = false;
	*/
	
	//create a helper lambda
	std::function<void(node*, std::vector<node*>&)> add_leaves = [&](node* mn, std::vector<node*>& al) { if (mn->is_leaf()) { al.push_back(mn); } else { for (int z = 0; z < 4; ++z) { if (mn->children[z] != nullptr && !mn->children[z]->locked) { add_leaves(mn->children[z],al); } } } };
	
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
	
	//int num_rooms = 1;
	int offsetter = 3;
	int min_rx, min_ry;
	sr->get_xy(min_rx, min_ry);
	int max_rx = min_rx + offsetter,
		max_ry = min_ry + offsetter;
	min_rx -= offsetter;
	min_ry -= offsetter;
	
	if (min_rx < 1)
	{
		min_rx = 1;
	}
	if (min_ry < 1)
	{
		min_ry = 1;
	}
	if (max_rx > (gs.level->get_width() - 2))
	{
		max_rx = (gs.level->get_width() - 2);
	}
	if (max_ry > (gs.level->get_height() - 2))
	{
		max_ry = (gs.level->get_height() - 2);
	}
	
	
	//loop until we've made enough rooms
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
		
		//first off, find out how many children we can have
		int max_children = 0;
		if (ry > min_ry && gs.level->get_room(rx, ry - 1) == nullptr)
		{
			++max_children;
		}
		if (rx > min_rx && gs.level->get_room(rx - 1, ry) == nullptr)
		{
			++max_children;
		}
		if (rx < max_rx && gs.level->get_room(rx + 1, ry) == nullptr)
		{
			++max_children;
		}
		if (ry < max_ry && gs.level->get_room(rx, ry + 1) == nullptr)
		{
			++max_children;
		}
		
		//if we can have kids...
		if (max_children > 0)
		{
			//if we can have kids, have a random amount!
			int num_kids = max_children;//MyMath::random_int(1,max_children);
			
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
			for (int i = 0; i < 4; ++i)
			{
				int x_off = (dirs[i] & 3) == 2 ? -1 : dirs[i] & 3;
				int y_off = ((dirs[i] >> 2) & 3) == 2 ? -1 : ((dirs[i] >> 2) & 3);
				int n_x = rx + x_off;
				int n_y = ry + y_off;
				
				if (n_y >= min_ry && n_y <= max_ry &&
					n_x >= min_rx && n_x <= max_rx &&
					gs.level->get_room(n_x, n_y) == nullptr)
				{	
					//update the number of rooms
					//++num_rooms;
					++num_kids;
					
					//set stuff in the node
					node* child = new node;
					child->handle = gs.level->create_room(n_x, n_y);
					child->children[0] = child->children[1] = child->children[2] = child->children[3] = nullptr;
					child->locked = false;
					child->parent = n;
					
					//set stuff in the room
					Room* rr = gs.level->get_room(child->handle);
					rr->set_short_description("A Small Room");
					rr->set_description("A <fg=green>leaf<fg=white> node, seemingly.");
					rr->set_minimap_symbol("<bg=yellow> ");
					
					//set exits in the rooms
					rr->set_exit(dirs[i] == 1 ? Room::Exit::WEST : dirs[i] == 2 ? Room::Exit::EAST : dirs[i] == 4 ? Room::Exit::NORTH : Room::Exit::SOUTH,Room::Exit_Status::Open);
					r->set_exit(dirs[i] == 1 ? Room::Exit::EAST : dirs[i] == 2 ? Room::Exit::WEST : dirs[i] == 4 ? Room::Exit::SOUTH : Room::Exit::NORTH,Room::Exit_Status::Open);
					
					//set stuff in the parent node
					n->children[i] = child;
					
					//add this child to the stack
					stack.push_back(child);
				}
			}
			
			//if we ended up having any kids, then update the description
			if (num_kids > 0)
			{
				r->set_short_description("A Hallway");
				r->set_description("A corridor leading to other rooms.");
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
	ko->name = "God";
	ko->description = "You won the game!";
	ko->scripts.construct("","(say \"You have won the game! Nicely done!\");","(say \"You have won the game! Nicely done!\");","");

	//put the key in the farthest room from the starting room
	node* random_node = starting_node;
	
	while (!random_node->is_leaf() || random_node == starting_node)
	{
		int most_kids = 0;
		node* best_kid = nullptr;
		for (int i = 0; i < 4; ++i)
		{
			if (random_node->children[i] != nullptr)
			{
				int nck = random_node->children[i]->count_children();
				if (nck > most_kids)
				{
					most_kids = nck;
					best_kid = random_node->children[i];
				}
			}
		}
		
		if (best_kid != nullptr)
		{
			random_node = best_kid;
		}
	}
	ko->room_container = random_node->handle;
	gs.level->get_room(random_node->handle)->objects().push_back(ko->get_handle());
	
	//set up some colors for the doors
	std::vector<std::string> color_names = { "red", "green", "yellow"/*, "blue"*/, "magenta", "cyan"};
	
	//make locks and keys!
	for (int k = 0; k < 3; ++k)
	{
		//figure out which room to lock
		int steps_up = 1;//MyMath::random_int(1,3);
		for (int i = 0; i < steps_up && random_node->parent != starting_node && random_node->count_children() < 5; ++i)
		{
			random_node = random_node->parent;
		}
		
		//pick a color for the door
		std::size_t c_pos = MyMath::random_int(0,color_names.size()-1);
		std::string door_color = color_names[c_pos];
		color_names[c_pos] = color_names.back();
		color_names.pop_back();
		
		//lock the door between this room and the parent room
		random_node->locked = true;
		int px,py,cx,cy;
		Room* pr = gs.level->get_room(random_node->parent->handle);
		pr->get_xy(px,py);
		gs.level->get_room(random_node->handle)->get_xy(cx,cy);
		int x_off = cx - px,
			y_off = cy - py;
		Room::Exit e = x_off == 0 && y_off == -1 ? Room::Exit::NORTH : x_off == 0 && y_off == 1 ? Room::Exit::SOUTH : x_off == -1 ? Room::Exit::WEST : Room::Exit::EAST;
		pr->set_exit(e,Room::Exit_Status::Locked);
		pr->set_door_description("a large door that glows a soft <fg=" + door_color + ">" + door_color,e);
		
		//now place another key!
		std::vector<node*> available_leaves;
		add_leaves(starting_node,available_leaves);
		
		
		if (available_leaves.size() == 0)
		{
			#ifdef DEBUG
				Log::write("Derp, no available nodes!");
			#endif
			
			random_node = starting_node;
			k = 10000;
		}
		else
		{
			random_node = available_leaves[MyMath::random_int(0,available_leaves.size() - 1)];
		}
		
		//make an enemy in this room
		ECS::Handle new_enemy = gs.level->create_object();
		Object* neo = gs.level->get_object(new_enemy);
		neo->copy(*gs.level->get_object(enemy_1));
		neo->room_container = random_node->handle;
		gs.level->get_room(random_node->handle)->objects().push_back(new_enemy);
		
		//make a key
		std::string direction_names[] = {"North","East","South","West"};
		kh = gs.level->create_object();
		ko = gs.level->get_object(kh);
		ko->visible = true;
		ko->visible_in_short_description = false;
		ko->friendly = true;
		ko->mobile = false;
		ko->playable = false;
		ko->open = false;
		ko->holdable = true;
		ko->object_container = new_enemy;
		ko->room_container = -1;
		ko->hitpoints = -1;
		ko->attack = 0;
		ko->hit_chance = 0.0f;
		ko->name = "A <fg=" + door_color + ">Key";
		ko->description = "A skeleton key. It glows <fg=" + door_color + ">" + door_color + "<fg=white>. Find the door it belongs to!";
		ko->scripts.construct("",
							  "",
							  "(if (= (get current_room.handle) " + StringUtils::to_string((int)pr->get_handle()) + ") (+ (set current_room.description \"<fg=white><bg=black>A small, cramped room. To the " + direction_names[(int)e] + " is an unlocked door.\") (set global.main_text (+ (get global.main_text) \"<fg=white><bg=black>\n\nThe key opens a door to the " + direction_names[(int)e] + ".\")) (set current_room.open_" + StringUtils::to_lowercase(direction_names[(int)e].substr(0,1)) + " true) (destroy (get caller.handle))) (set global.main_text (+ (get global.main_text) \"<fg=white><bg=black>\n\nIt does nothing\")));",
							  "");
		neo->objects.push_back(kh);
	}
	
	//now that the puzzle is made, let's add some enemies!
	std::vector<node*> available_leaves;
	add_leaves(starting_node,available_leaves);
	
	for (node* leaf : available_leaves)
	{
		int max_enemies = 2;
		Room* leaf_room = gs.level->get_room(leaf->handle);
		
		if (!leaf_room->objects().empty())
		{
			max_enemies = 1;
		}
		
		for (int i = MyMath::random_int(0, max_enemies); i > 0; --i)
		{
			ECS::Handle last_enemy = gs.level->create_object();
			Object* leo = gs.level->get_object(last_enemy);
			leo->copy(*gs.level->get_object(enemy_1));
			leo->room_container = leaf->handle;
			leaf_room->objects().push_back(last_enemy);
		}
	}
	
	//make the player in the first room!
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
	
	//also, put a weird painting in a random room
	o = gs.level->get_object(gs.level->create_object());
	o->visible = true;
	o->visible_in_short_description = false;
	o->friendly = true;
	o->mobile = false;
	o->playable = false;
	o->open = true;
	o->holdable = false;
	o->room_container = random_node->handle;
	o->object_container = -1;
	o->hitpoints = -1;
	o->attack = 0;
	o->hit_chance = 0.0f;
	o->name = "An Odd Painting";
	o->description = "<fg=white><bg=black>A strange painting of a bunny.\n<fg=black><bg=yellow> (\\_/) <bg=black>\n<bg=yellow>(='.'=)<bg=black>\n<bg=yellow>(\")_(\")<bg=black>";
	o->scripts.construct("","(set main_text (+ (get main_text) \"<fg=white><bg=black>\nYou feel as if you're being watched...\"));","(set main_text (+ (get main_text) \"<fg=white><bg=black>\nYou attempt to use the painting, but nothing happens.\"));", "");
	gs.level->get_room(random_node->handle)->objects().push_back(o->get_handle());
	
	//finally, free the memory for the tree
	std::vector<node*> to_delete;
	to_delete.push_back(starting_node);
	while (!to_delete.empty())
	{
		//grab and pop off the last element
		node* nn = to_delete.back();
		to_delete.pop_back();
		
		//add all of its children to the stack
		for (int i = 0; i < 4; ++i)
		{
			if (nn->children[i] != nullptr)
			{
				to_delete.push_back(nn->children[i]);
			}
		}
		
		//delete it
		delete nn;
	}
}
