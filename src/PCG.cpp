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

struct Dungeon
{
	int width;
	int height;
	int x;
	int y;
	
	int entrance_x;
	int entrance_y;
	
	ECS::Handle mcguffin;
};

struct node
{
	ECS::Handle handle;
	
	node* children[4];
	node* parent;
	int x;
	int y;
	
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

void create_dungeon(GameState& gs, ECS::Handle enemy_1, Dungeon dungeon_stats)
{
	//get the upper and lower bounds
	int min_rx = dungeon_stats.x;
	int min_ry = dungeon_stats.y;
	int max_rx = min_rx + dungeon_stats.width - 1;
	int max_ry = min_ry + dungeon_stats.height - 1;
	
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
	
	#ifdef DEBUG
		if (max_rx <= min_rx || max_ry <= min_ry)
		{
			Log::write("Error: dungeon dimensions are invalid.");
		}
		
		if (dungeon_stats.entrance_x < min_rx || dungeon_stats.entrance_x > max_rx || dungeon_stats.entrance_y < min_ry || dungeon_stats.entrance_y > min_rx)
		{
			Log::write("Error: dungeon entrance is outside of the dungeon.");
		}
	#endif

	//create a helper lambda
	std::function<void(node*, std::vector<node*>&)> add_leaves = [&](node* mn, std::vector<node*>& al) { if (mn->is_leaf()) { al.push_back(mn); } else { for (int z = 0; z < 4; ++z) { if (mn->children[z] != nullptr && !mn->children[z]->locked) { add_leaves(mn->children[z],al); } } } };
	
	//create a starting point for the dungeon
	node* starting_node = new node;
	starting_node->x = dungeon_stats.entrance_x;//min_rx + width / 2;
	starting_node->y = dungeon_stats.entrance_y;//min_ry + height - 1;
	starting_node->parent = starting_node->children[0] = starting_node->children[1] = starting_node->children[2] = starting_node->children[3] = nullptr;
	starting_node->locked = false;
	starting_node->handle = -1;
	
	std::vector<node*> stack;
	stack.push_back(starting_node);
	
	//loop until we've filled up the dungeon
	while (!stack.empty())
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
		
		//now check if it's already a room
		if (gs.level->get_room(n->x, n->y) != nullptr)
		{
			continue;
		}
		
		//now check if it's an invalid spot for a room
		if (n->x < min_rx || n->x > max_rx || n->y < min_ry || n->y > max_ry)
		{
			continue;
		}
		
		//if it's a valid spot, create a new room!
		Room* nr = gs.level->get_room(gs.level->create_room(n->x, n->y));
		n->handle = nr->get_handle();
		nr->set_short_description("A Small Room");
		nr->set_description("A <fg=green>leaf<fg=white> node, seemingly.");
		nr->set_minimap_symbol("<bg=yellow> ");
		nr->set_visited(false);
		
		//if this room has a parent, open a doorway between the two and update the parent
		if (n->parent != nullptr)
		{
			//get the parent object
			Room* pr = gs.level->get_room(n->parent->handle);
			
			//make the doors
			Room::Exit pcd = n->parent->y > n->y ? Room::Exit::NORTH : n->parent->y < n->y ? Room::Exit::SOUTH : n->parent->x > n->x ? Room::Exit::WEST : Room::Exit::EAST;
			Room::Exit cpd = pcd == Room::Exit::NORTH ? Room::Exit::SOUTH : pcd == Room::Exit::SOUTH ? Room::Exit::NORTH : pcd == Room::Exit::WEST ? Room::Exit::EAST : Room::Exit::WEST;
			pr->set_exit(pcd, Room::Exit_Status::Open);
			nr->set_exit(cpd, Room::Exit_Status::Open);
		
			//update the tree
			n->parent->children[static_cast<std::size_t>(pcd)] = n;
			
			//update the room object
			pr->set_short_description("A Hallway");
			pr->set_description("A corridor leading to other rooms.");
		}
		
		//create some children for this room!
		node* nn = new node;
		nn->x = n->x - 0;
		nn->y = n->y - 1;
		nn->locked = false;
		nn->parent = n;
		nn->handle = -1;
		nn->children[0] = nn->children[1] = nn->children[2] = nn->children[3] = nullptr;
		stack.push_back(nn);
		
		node* en = new node;
		en->x = n->x + 1;
		en->y = n->y - 0;
		en->locked = false;
		en->parent = n;
		en->handle = -1;
		en->children[0] = en->children[1] = en->children[2] = en->children[3] = nullptr;
		stack.push_back(en);
		
		node* sn = new node;
		sn->x = n->x - 0;
		sn->y = n->y + 1;
		sn->locked = false;
		sn->parent = n;
		sn->handle = -1;
		sn->children[0] = sn->children[1] = sn->children[2] = sn->children[3] = nullptr;
		stack.push_back(sn);
		
		node* wn = new node;
		wn->x = n->x - 1;
		wn->y = n->y - 0;
		wn->locked = false;
		wn->parent = n;
		wn->handle = -1;
		wn->children[0] = wn->children[1] = wn->children[2] = wn->children[3] = nullptr;
		stack.push_back(wn);
	}

	//At this point we should have a map full of rooms and a tree full of nodes. Time to start placing locks and keys.
	//put the McGuffin in the farthest room from the starting room
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
	ECS::Handle kh = dungeon_stats.mcguffin;
	Object* ko = gs.level->get_object(kh);
	ko->room_container = random_node->handle;
	gs.level->get_room(random_node->handle)->objects().push_back(kh);
	
	//set up some colors for the doors
	std::vector<std::string> color_names = { "red", "green", "yellow"/*, "blue"*/, "magenta", "cyan"};
	
	//make locks and keys!
	for (int k = 3; k > 0; --k)
	{
		//figure out which room to lock
		int max_rooms_to_lock = MyMath::random_int(1,(starting_node->count_children() - 1) / k);
		while (random_node->parent != starting_node && random_node->parent->count_children() <= max_rooms_to_lock)
		//for (int i = 0; i < steps_up && random_node->parent != starting_node && random_node->count_children() < 5; ++i)
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
			k = -10000;
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
							  "(if (= (get current_room.handle) " + StringUtils::to_string((int)pr->get_handle()) + ") (+ (set global.main_text (+ (get global.main_text) \"<fg=white><bg=black>\n\nThe key opens a door to the " + direction_names[(int)e] + ".\")) (set current_room.open_" + StringUtils::to_lowercase(direction_names[(int)e].substr(0,1)) + " true) (destroy (get caller.handle))) (set global.main_text (+ (get global.main_text) \"<fg=white><bg=black>\n\nIt does nothing\")));",
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
	
	//also, put a weird painting in a random room
	Object* o = gs.level->get_object(gs.level->create_object());
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
	
	gs.level->get_room(dungeon_stats.entrance_x, dungeon_stats.entrance_y)->set_exit(Room::Exit::SOUTH, Room::Exit_Status::Open);
}

void create_overworld(GameState& gs, ECS::Handle enemy_1)
{
	//create a starting space and 3 dungeons
	node* starting_node = new node;
	starting_node->x = 100;
	starting_node->y = 150;
	
	node* d1 = new node;
	d1->x = 80;
	d1->y = 150;
	d1->locked = false;
	d1->parent = starting_node;
	d1->children[0] = d1->children[1] = d1->children[2] = d1->children[3] = nullptr;
	
	node* d2 = new node;
	d2->x = 110;
	d2->y = 150;
	d2->locked = false;
	d2->parent = starting_node;
	d2->children[0] = d2->children[1] = d2->children[2] = d2->children[3] = nullptr;
	
	node* d3 = new node;
	d3->x = 100;
	d3->y = 130;
	d3->locked = false;
	d3->parent = starting_node;
	d3->children[0] = d3->children[1] = d3->children[2] = d3->children[3] = nullptr;
	
	starting_node->children[0] = d1;
	starting_node->children[1] = d2;
	starting_node->children[2] = d3;
	starting_node->children[3] = nullptr;
	
	//create the last mcguffin
	ECS::Handle mcguffin = -1;
	mcguffin = gs.level->create_object();
	Object* ko = gs.level->get_object(mcguffin);
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
	
	while (starting_node->count_children() > 0)
	{
		//pick a random (unlocked) node to put the mcguffin in
		node* random_node = nullptr;
		if (!starting_node->children[0]->locked && MyMath::random_int(0,2) == 0)
		{
			random_node = starting_node->children[0];
		}
		else if (!starting_node->children[1]->locked && MyMath::random_int(0,1) == 0)
		{
			random_node = starting_node->children[1];
		}
		else
		{
			random_node = starting_node->children[2];
		}
		
		//jam the mcguffin in there and create the dungeon!
		Dungeon d;
		d.mcguffin = mcguffin;
		d.width = MyMath::random_int(2,7);
		d.height = MyMath::random_int(1 + 16 / d.width, 49 / d.width);
		d.x = random_node->x;
		d.y = random_node->y;
		d.entrance_x = d.x + d.width / 2;
		d.entrance_y = d.y + d.height - 1;
		
		create_dungeon(gs, enemy_1, d);
		
		//update the exit
		gs.level->get_room(d.entrance_x, d.entrance_y)->set_exit(Room::Exit::NORTH, starting_node->count_children() > 0 ? Room::Exit_Status::Open : Room::Exit_Status::Locked);
		
		//now lock it and make a new key
		random_node->locked = true;
		if (starting_node->count_children() > 0)
		{
			mcguffin = gs.level->create_object();
			Object* ko = gs.level->get_object(mcguffin);
			ko->visible = true;
			ko->visible_in_short_description = false;
			ko->friendly = true;
			ko->mobile = false;
			ko->playable = false;
			ko->open = false;
			ko->holdable = true;
			ko->object_container = -1;
			ko->room_container = -1;
			ko->hitpoints = -1;
			ko->attack = 0;
			ko->hit_chance = 0.0f;
			ko->name = "A Massive Key";
			ko->description = "A huge key. Find the dungeon it belongs to!";
			ko->scripts.construct("",
								  "",//well poop, how do we reference a room that hasn't been created yet? Now we need to initiate each room before starting, which'll throw everything off!
								  "(if (= (get current_room.handle) " + StringUtils::to_string((int)rrr->get_handle()) + ") (+ (set global.main_text (+ (get global.main_text) \"<fg=white><bg=black>\n\nThe huge doors of the dungeon swing open.\")) (set current_room.open_n true) (destroy (get caller.handle))) (set global.main_text (+ (get global.main_text) \"<fg=white><bg=black>\n\nIt does nothing\")));",
								  "");
		}
	}
	
	//fill in everything but the dungeons
	for (int x = 1; x < gs.level->get_width() - 1; ++x)
	{
		for (int y = 1; y < gs.level->get_height() - 1; ++y)
		{
			if (gs.level->get_room(x,y) == nullptr)
			{
				Room* ro = gs.level->get_room(gs.level->create_room(x,y));
				
				ro->set_short_description("Outside");
				ro->set_description("A sunny, grassey field.");
				ro->set_minimap_symbol("<fg=white><bg=green>#");
				ro->set_visited(true);
				
				if (y > 1 && (gs.level->get_room(x, y - 1) == nullptr || gs.level->get_room(x, y - 1)->get_exit(Room::Exit::SOUTH) == Room::Exit_Status::Open))
				//if (y > 1 && ((y - d.y - d.height) != 0 || !MyMath::between(x, d.x, d.x + d.width - 1)))
				{
					ro->set_exit(Room::Exit::NORTH, Room::Exit_Status::Open);
				}
				if (x < (gs.level->get_width() - 2) && (gs.level->get_room(x + 1, y) == nullptr || gs.level->get_room(x + 1, y)->get_exit(Room::Exit::WEST) == Room::Exit_Status::Open))
				//if (x < (gs.level->get_width() - 2) && ((d.x - x) != 1 || !MyMath::between(y, d.y, d.y + d.height -1)))
				{
					ro->set_exit(Room::Exit::EAST, Room::Exit_Status::Open);
				}
				if (y < (gs.level->get_height() - 2) && (gs.level->get_room(x, y + 1) == nullptr || gs.level->get_room(x, y + 1)->get_exit(Room::Exit::NORTH) == Room::Exit_Status::Open))
				//if (y < (gs.level->get_height() - 2) && ((d.y - y) != 1 || !MyMath::between(x, d.x, d.x + d.width - 1)))
				{
					ro->set_exit(Room::Exit::SOUTH, Room::Exit_Status::Open);
				}
				if (x > 1 && (gs.level->get_room(x - 1, y) == nullptr || gs.level->get_room(x - 1, y)->get_exit(Room::Exit::EAST) == Room::Exit_Status::Open))
				//if (x > 1 && ((x - d.x - d.width) != 0 || !MyMath::between(y, d.y, d.y + d.height -1)))
				{
					ro->set_exit(Room::Exit::WEST, Room::Exit_Status::Open);
				}
				
				if (gs.level->get_room(x, y - 1) != nullptr && gs.level->get_room(x, y - 1)->get_exit(Room::Exit::SOUTH) == Room::Exit_Status::Locked)
				{
					ro->set_exit(Room::Exit::NORTH, Room::Exit_Status::Locked);
					gs.level->get_room(x, y - 1)->set_exit(Room::Exit::SOUTH, Room::Exit_Status::Open);
				}
			}
		}
	}
	
	//update the starting node slightly
	Room* sr = gs.level->get_room(starting_node->x, starting_node->y);
	sr->set_short_description("Your Home");
	sr->set_description("A patch of dirt. The remains of a campfire smolder here.");
	sr->set_minimap_symbol("<fg=red><bg=yellow>%");

	//put the player object at the starting location
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
	
	//create a map object that displays a crude map of the world on_use
	
}

void PCG::create_world(GameState& gs)
{
	//set up the basic stuff
	gs.level = new Level(200,200);
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
	/*
	//create a dungeon
	Dungeon d;
	d.x = 50;
	d.y = 50;
	d.width = 7;
	d.height = 7;
	d.entrance_x = d.x + d.width / 2;
	d.entrance_y = d.y + d.height - 1;
	create_dungeon(gs, enemy_1, d);
	
	//fill in everything but the dungeon
	for (int x = 1; x < gs.level->get_width() - 1; ++x)
	{
		for (int y = 1; y < gs.level->get_height() - 1; ++y)
		{
			if (gs.level->get_room(x,y) == nullptr)
			{
				Room* ro = gs.level->get_room(gs.level->create_room(x,y));
				
				ro->set_short_description("Outside");
				ro->set_description("A sunny, grassey field.");
				ro->set_minimap_symbol("<fg=white><bg=green>#");
				ro->set_visited(true);
				
				if (y > 1 && ((y - d.y - d.height) != 0 || !MyMath::between(x, d.x, d.x + d.width - 1)))
				{
					ro->set_exit(Room::Exit::NORTH, Room::Exit_Status::Open);
				}
				if (x < (gs.level->get_width() - 2) && ((d.x - x) != 1 || !MyMath::between(y, d.y, d.y + d.height -1)))
				{
					ro->set_exit(Room::Exit::EAST, Room::Exit_Status::Open);
				}
				if (y < (gs.level->get_height() - 2) && ((d.y - y) != 1 || !MyMath::between(x, d.x, d.x + d.width - 1)))
				{
					ro->set_exit(Room::Exit::SOUTH, Room::Exit_Status::Open);
				}
				if (x > 1 && ((x - d.x - d.width) != 0 || !MyMath::between(y, d.y, d.y + d.height -1)))
				{
					ro->set_exit(Room::Exit::WEST, Room::Exit_Status::Open);
				}
			}
		}
	}
	*/
	create_overworld(gs,enemy_1);
	
	//make the player just outside the entrance
	
	//update the entrance to connect to the outside
	//sr->set_exit(Room::Exit::NORTH, Room::Exit_Status::Open);
	//gs.level->get_room(d.entrance_x, d.entrance_y)->set_exit(Room::Exit::SOUTH, Room::Exit_Status::Open);
}
