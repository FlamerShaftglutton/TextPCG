#include "PCG.hpp"
#include "GameState.hpp"
#include "Level.hpp"
#include "Room.hpp"
#include "Object.hpp"
#include "CombatData.hpp"
#include "Handle.hpp"
#include "mymath.hpp"
#include "string_utils.hpp"
#include "Bitmask.hpp"

#include <cmath>
#include <vector>
#include <functional>
#include <map>
#include <algorithm>

#ifdef DEBUG
	#include "Log.hpp"
	#include <fstream>
#endif

struct Dungeon
{
	Bitmask bitmask;
	
	Position entrance;
	
	ECS::Handle macguffin;
};

struct Zone
{
	Bitmask bitmask;

	Dungeon d;
	ECS::Handle macguffin;
	Zone* macguffin_opens = nullptr;
	
	std::vector<Zone*> children;
	std::map<Zone*,Position> exits;
	Zone* parent;
	int theme;
	
	bool locked;
	
	int count_children()
	{
		int count = 1;
		
		for (std::size_t i = 0; i < children.size(); ++i)
		{
			if (!children[i]->locked)
			{
				count += children[i]->count_children();
			}
		}
		
		return count;
	}
	
	bool is_leaf()
	{
		return children.empty();
	}
};

struct node
{
	ECS::Handle handle;
	
	node* children[4];
	node* parent;
	Position pos;
	
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
	//create a helper lambda
	std::function<void(node*, std::vector<node*>&)> add_leaves = [&](node* mn, std::vector<node*>& al) { if (mn->is_leaf()) { al.push_back(mn); } else { for (int z = 0; z < 4; ++z) { if (mn->children[z] != nullptr && !mn->children[z]->locked) { add_leaves(mn->children[z],al); } } } };
	
	//create a starting point for the dungeon
	node* starting_node = new node;
	starting_node->pos.x = dungeon_stats.entrance.x;
	starting_node->pos.y = dungeon_stats.entrance.y;
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
		if (gs.level->get_room(n->pos.x, n->pos.y) != nullptr)
		{
			continue;
		}
		
		//now check if it's an invalid spot for a room
		if (!dungeon_stats.bitmask(n->pos))
		//if (n->pos.x < min_rx || n->pos.x > max_rx || n->pos.y < min_ry || n->pos.y > max_ry)
		{
			continue;
		}
		
		//if it's a valid spot, create a new room!
		Room* nr = gs.level->get_room(gs.level->create_room(n->pos.x, n->pos.y));
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
			Room::Exit pcd = n->parent->pos.y > n->pos.y ? Room::Exit::NORTH : n->parent->pos.y < n->pos.y ? Room::Exit::SOUTH : n->parent->pos.x > n->pos.x ? Room::Exit::WEST : Room::Exit::EAST;
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
		nn->pos.x = n->pos.x - 0;
		nn->pos.y = n->pos.y - 1;
		nn->locked = false;
		nn->parent = n;
		nn->handle = -1;
		nn->children[0] = nn->children[1] = nn->children[2] = nn->children[3] = nullptr;
		stack.push_back(nn);
		
		node* en = new node;
		en->pos.x = n->pos.x + 1;
		en->pos.y = n->pos.y - 0;
		en->locked = false;
		en->parent = n;
		en->handle = -1;
		en->children[0] = en->children[1] = en->children[2] = en->children[3] = nullptr;
		stack.push_back(en);
		
		node* sn = new node;
		sn->pos.x = n->pos.x - 0;
		sn->pos.y = n->pos.y + 1;
		sn->locked = false;
		sn->parent = n;
		sn->handle = -1;
		sn->children[0] = sn->children[1] = sn->children[2] = sn->children[3] = nullptr;
		stack.push_back(sn);
		
		node* wn = new node;
		wn->pos.x = n->pos.x - 1;
		wn->pos.y = n->pos.y - 0;
		wn->locked = false;
		wn->parent = n;
		wn->handle = -1;
		wn->children[0] = wn->children[1] = wn->children[2] = wn->children[3] = nullptr;
		stack.push_back(wn);
	}

	//At this point we should have a map full of rooms and a tree full of nodes. Time to start placing locks and keys.
	//put the MacGuffin in the farthest room from the starting room
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
	ECS::Handle kh = dungeon_stats.macguffin;
	Object* ko = gs.level->get_object(kh);
	ko->room_container = random_node->handle;
	gs.level->get_room(random_node->handle)->objects().push_back(kh);
	
	//set up some colors for the doors
	std::vector<std::string> color_names = { "red", "green", "yellow"/*, "blue"*/, "magenta", "cyan"};//blue is too hard to read
	
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
		nn = nullptr;
	}
}

void create_zone(GameState& gs, ECS::Handle enemy_1, Zone* zone)
{
	//theme stuff
	std::string themes[] = {"Volcano","Forest","Plains","Desert","Mountains","Tundra","Urban"};
	std::string descriptions[] = {"A hot, unforgiving patch of blackened dirt.",
								  "A small clearing surrounded by trees and shrubs.",
								  "A wide open clearing of grass and flowers.",
								  "A dune of sand with a few sharp rocks and even sharper plants.",
								  "A small plateau amidst jagged rocks and cliffs.",
								  "A flat expanse of snow and ice. Wind whips at you mercilessly.",
								  "A patch of dirt road between shacks."};
	std::string minimap_symbols[] = {"<fg=yellow><bg=red>;",
								   "<fg=white><bg=green>T",
								   "<bg=green> ",
								   "<bg=yellow><fg=red>.",
								   "<fg=magenta><bg=yellow>^",
								   "<fg=cyan><bg=white>.",
								   "<fg=yellow><bg=red>H"};

	//set up a dungeon skeleton, although don't actually fill it in yet
	if (zone->theme == 0)
	{
		Position dungeon_offset(zone->bitmask.get_offset().x + (zone->bitmask.get_width() - 9) / 2, zone->bitmask.get_offset().y + (zone->bitmask.get_height() - 5) / 2);
		zone->d.bitmask = Bitmask("111111111\n011111110\n001111100\n000111000\n000010000", dungeon_offset);
		zone->d.entrance.x = dungeon_offset.x + 4;
		zone->d.entrance.y = dungeon_offset.y + 4;
	}
	else if (zone->theme == 1)
	{
		Position dungeon_offset(zone->bitmask.get_offset().x + (zone->bitmask.get_width() - 7) / 2, zone->bitmask.get_offset().y + (zone->bitmask.get_height() - 7) / 2);
		zone->d.bitmask = Bitmask("0011100\n0111110\n1111111\n1111111\n1111111\n0111110\n0011100", dungeon_offset);
		zone->d.entrance.x = dungeon_offset.x + 3;
		zone->d.entrance.y = dungeon_offset.y + 6;
	}
	else if (zone->theme == 2)
	{
		Position dungeon_offset{0,0};
		
		zone->d.bitmask = Bitmask(7,7,dungeon_offset);
		
		Position sp(3,6);
		std::vector<Position> stack;
		stack.push_back(sp);
		
		for (int i = 0; i < 30; ++i)
		{
			//grab something from the stack
			int index = MyMath::random_int(0, stack.size() - 1);
			Position p = stack[index];
			stack[index] = stack.back();
			stack.pop_back();
			
			//fill in that position
			zone->d.bitmask.set(p,true);
			
			//add in the children
			if (p.x > 0)
			{
				stack.push_back(Position(p.x - 1, p.y));
			}
			if (p.x < 6)
			{
				stack.push_back(Position(p.x + 1, p.y));
			}
			if (p.y > 0)
			{
				stack.push_back(Position(p.x, p.y - 1));
			}
			if (p.y < 6)
			{
				stack.push_back(Position(p.x, p.y + 1));
			}
		}
		
		zone->d.bitmask.shrink_to_fit();
		
		//now adjust the offset
		dungeon_offset.x = zone->bitmask.get_offset().x + (zone->bitmask.get_width() - zone->d.bitmask.get_width()) / 2;
		dungeon_offset.y = zone->bitmask.get_offset().y + (zone->bitmask.get_height() - zone->d.bitmask.get_height()) / 2;
		zone->d.bitmask.set_offset(dungeon_offset);
		
		//now adjust the entrance
		zone->d.entrance.x = dungeon_offset.x + zone->d.bitmask.get_width() / 2;
		zone->d.entrance.y = dungeon_offset.y + zone->d.bitmask.get_height() - 1;
	}
	else
	{
		int dw = MyMath::random_int(2,7);
		int dh = MyMath::random_int(1 + 16 / dw, 49 / dw > 10 ? 10 : 49/dw);
		Position dungeon_offset;
		dungeon_offset.x = zone->bitmask.get_offset().x + (zone->bitmask.get_width() - dw) / 2;
		dungeon_offset.y = zone->bitmask.get_offset().y + (zone->bitmask.get_height() - dh) / 2;
		zone->d.bitmask = Bitmask(dw, dh, dungeon_offset);
		zone->d.entrance.x = dungeon_offset.x + dw / 2;
		zone->d.entrance.y = dungeon_offset.y + dh - 1;
		for (int x = 0; x < dw; ++x)
		{
			for (int y = 0; y < dh; ++y)
			{
				zone->d.bitmask.set(x + dungeon_offset.x, y + dungeon_offset.y, true);
			}
		}
	}
	
	//now check if the starting zone is on a valid space
	int ny = zone->d.entrance.y;
	for (;!zone->bitmask(zone->d.entrance.x, ny + 1) && ny >= zone->bitmask.get_offset().y - zone->d.bitmask.get_height() - 2; --ny);
	for (;!zone->bitmask(zone->d.entrance.x, ny + 1) && ny <= zone->bitmask.get_offset().y + zone->bitmask.get_height(); ++ny);
	
	if (ny != zone->d.entrance.y)
	{
		Position d_offset = zone->d.bitmask.get_offset();
		d_offset.y += ny - zone->d.entrance.y;
		zone->d.bitmask.set_offset(d_offset);
		
		zone->d.entrance.y = ny;
	}
	
	//attach a MacGuffin to the room
	zone->d.macguffin = zone->macguffin = gs.level->create_object();
	
	//first fill up every square that isn't part of the dungeon
	Bitmask b = zone->bitmask - zone->d.bitmask;
	Position offset = b.get_offset();
	int mx = offset.x + b.get_width();
	int my = offset.y + b.get_height();
	for (int x = offset.x; x < mx; ++x)
	{
		for (int y = offset.y; y < my; ++y)
		{
			if (b(x,y) && gs.level->get_room(x,y) == nullptr)
			{
				Room* r = gs.level->get_room(gs.level->create_room(x, y));
				r->set_description(descriptions[zone->theme]);
				r->set_short_description("A " + themes[zone->theme] + " Region");
				r->set_minimap_symbol(minimap_symbols[zone->theme]);
				r->set_visited(true);
				
				if (b(x, y - 1))
				{
					r->set_exit(Room::Exit::NORTH, Room::Exit_Status::Open);
				}
				if (b(x + 1, y))
				{
					r->set_exit(Room::Exit::EAST, Room::Exit_Status::Open);
				}
				if (b(x, y + 1))
				{
					r->set_exit(Room::Exit::SOUTH, Room::Exit_Status::Open);
				}
				if (b(x - 1, y))
				{
					r->set_exit(Room::Exit::WEST, Room::Exit_Status::Open);
				}
			}
			#ifdef DEBUG
			else if (gs.level->get_room(x,y) != nullptr)
			{
				Log::write("Warning: Overlapping rooms! Second room not created!");
			}
			#endif
		}
	}
	
	//now make the dungeon!
	create_dungeon(gs, enemy_1, zone->d);
	
	//now connect it to the outside
	gs.level->get_room(zone->d.entrance.x, zone->d.entrance.y)->set_exit(Room::Exit::SOUTH, Room::Exit_Status::Open);
	gs.level->get_room(zone->d.entrance.x, zone->d.entrance.y + 1)->set_exit(Room::Exit::NORTH, Room::Exit_Status::Open);
}

void create_overworld(GameState& gs)
{
	//create a starting zones
	Zone* starting_zone = new Zone;
	std::vector<Zone*> all_zones;
	std::vector<Zone*> stack;
	stack.push_back(starting_zone);
	
	//create a tree of zones
	int num_zones = 6;
	//int zone_radius = 7;
	//auto distance_between_points = [](int x1, int y1, int x2, int y2){float diff_x = float(x1 - x2), diff_y = float(y1 - y2); return (int)(std::sqrt(diff_x * diff_x + diff_y * diff_y)); };
	for (int i = 0; i < num_zones; ++i)
	{
		int index = MyMath::random_int(0, stack.size() - 1);
		Zone* z = stack[index];//TODO: use a modified priority queue to get results that are a bit better
		Zone* c = new Zone;
		z->children.push_back(c);
		c->parent = z;
		stack.push_back(c);
		
		if (z->count_children() > 2)
		{
			stack.erase(stack.begin() + index);
		}
	}
	
	//create a list of all of the zones
	stack.clear();
	stack.push_back(starting_zone);
	while (!stack.empty())
	{
		Zone* z = stack.back();
		stack.pop_back();
		
		all_zones.push_back(z);
		
		for (Zone* zz : z->children)
		{
			stack.push_back(zz);
		}
	}
	
	//fill in some basic stuff for each zone
	for (Zone* z : all_zones)
	{
		//fill up the bitmask
		Position p;
		p.x = 0;
		p.y = 0;
		z->bitmask = Bitmask(15, 15, p);
		int max_rooms = (int)(z->bitmask.get_width() * z->bitmask.get_height() * 0.9f);
		int min_rooms = max_rooms / 2;
		
		p = {7,7};
		
		std::vector<Position> ps;
		ps.push_back(p);
		int num_rooms = MyMath::random_int(min_rooms, max_rooms);
		
		for (int i = 0; i < num_rooms && !ps.empty(); ++i)
		{
			//grab a random entry
			int index = MyMath::random_int(0, ps.size() - 1);
			p = ps[index];
			ps[index] = ps.back();
			ps.pop_back();
			
			//fill it in
			if (z->bitmask(p))
			{
				--i;
			}
			else
			{
				z->bitmask.set(p,true);
			}
			
			//add in all of the children
			if (p.x > 0)
			{
				ps.push_back(Position(p.x - 1, p.y));
			}
			if (p.x < z->bitmask.get_width() - 1)
			{
				ps.push_back(Position(p.x + 1, p.y));
			}
			if (p.y > 0)
			{
				ps.push_back(Position(p.x, p.y - 1));
			}
			if (p.y < z->bitmask.get_height() - 1)
			{
				ps.push_back(Position(p.x, p.y + 1));
			}
		}
		
		//now that it is populated, crop it
		z->bitmask.shrink_to_fit();
	}
	
	//create a helper lambda
	std::function<void(Zone*, std::vector<Zone*>&)> add_leaves = [&](Zone* mn, std::vector<Zone*>& al) { if (mn->count_children() == 1) { al.push_back(mn); } else { for (std::size_t z = 0; z < mn->children.size(); ++z) { if (mn->children[z] != nullptr && !mn->children[z]->locked) { add_leaves(mn->children[z],al); } } } };

	//prime the pump
	Zone* random_zone = starting_zone;
	Zone* previous_zone = nullptr;
	while (!random_zone->is_leaf())
	{
		//travel in a random direction
		random_zone = random_zone->children[MyMath::random_int(0,random_zone->children.size() - 1)];
	}
	
	//loop until every zone has a 'key' and a lock
	while (starting_zone->count_children() > 1)//the starting node counts itself, so we have to use 1 instead of 0
	{
		//slap the macguffin into it
		random_zone->macguffin_opens = previous_zone;
		
		//lock it
		random_zone->locked = true;
		
		//pick a random (unlocked) zone with no unlocked children to put the next macguffin in
		std::vector<Zone*> available_leaves;
		add_leaves(starting_zone, available_leaves);

		previous_zone = random_zone;
		random_zone = available_leaves[MyMath::random_int(0,available_leaves.size() - 1)];
	}
	starting_zone->macguffin_opens = previous_zone;
	
	//position each zone
	const int num_angles = 16;
	float angles[num_angles];
	for (std::size_t i = 0; i < num_angles; ++i)
	{
		angles[i] = 6.28318530718f * (float)i / (float)num_angles;
	}
	std::vector<Zone*> complete_zones;
	starting_zone->bitmask.set_offset(0,0);
	//starting_zone->pos.x = starting_zone->pos.y = 0;
	complete_zones.push_back(starting_zone);
	std::vector<Zone*> incomplete_zones;
	for (std::size_t i = 0; i < starting_zone->children.size(); ++i)
	{
		incomplete_zones.push_back(starting_zone->children[i]);
	}
	int tries = 0;
	while (!incomplete_zones.empty())
	{
		//grab the top thing on the stack
		Zone* z = incomplete_zones.back();
		incomplete_zones.pop_back();
		
		//shuffle the list of directions
		for (int i = num_angles - 1; i > 0; --i)
		{
			int p = MyMath::random_int(0, i);
			float f = angles[i];
			angles[i] = angles[p];
			angles[p] = f;
		}
		
		//figure out where to put this zone
		bool placed = false;
		int minimum_distance = 6 + (std::max(z->parent->bitmask.get_width(), z->parent->bitmask.get_height()) + std::max(z->bitmask.get_width(), z->bitmask.get_height())) / 2;
		for (int i = 0; i < num_angles; ++i)
		{
			//figure out where this spot would be
			float f = angles[i];
			Position op;
			op.x = z->parent->bitmask.get_offset().x + (int)( std::cos(f) * minimum_distance);
			op.y = z->parent->bitmask.get_offset().y + (int)(-std::sin(f) * minimum_distance);
			z->bitmask.set_offset(op.x, op.y);
			
			//figure out if that touches any other zone
			bool touches = false;
			for (Zone* oz : complete_zones)
			{
				if ((z->bitmask + z->d.bitmask).touches(oz->bitmask))
				{
					touches = true;
					break;
				}
			}
			
			//if it doesn't touch, place it!
			if (!touches)
			{
				complete_zones.push_back(z);
				placed = true;
				break;
			}
		}
		
		//if we couldn't place the zone, scrap the whole overworld and try again
		if (!placed)
		{
			//increment the try counter
			++tries;
			
			//if we've tried too many times, blow up
			if (tries > 4)
			{
				#ifdef DEBUG
					Log::write("ERROR: Couldn't generate an overworld after 5 tries. Blowing up now.");
				#endif
				return;
			}
			else
			{
				//reset everything
				complete_zones.clear();
				incomplete_zones.clear();
				complete_zones.push_back(starting_zone);
				for (std::size_t i = 0; i < starting_zone->children.size(); ++i)
				{
					incomplete_zones.push_back(starting_zone->children[i]);
				}
			}
		}
		else
		{
			//add all of this zone's children to the stack
			for (std::size_t i = 0; i < z->children.size(); ++i)
			{
				incomplete_zones.push_back(z->children[i]);
			}
		}
	}
	
	//now go through and reposition everybody
	int min_x =  100000;
	int min_y =  100000;
	int max_x = -100000;
	int max_y = -100000;
	for (Zone* z : all_zones)
	{
		if (z->bitmask.get_offset().x < min_x)
		{
			min_x = z->bitmask.get_offset().x;
		}
		if (z->bitmask.get_offset().y < min_y)
		{
			min_y = z->bitmask.get_offset().y;
		}
		if (z->bitmask.get_offset().x + z->bitmask.get_width() > max_x)
		{
			max_x = z->bitmask.get_offset().x + z->bitmask.get_width();
		}
		if (z->bitmask.get_offset().y + z->bitmask.get_height() > max_y)
		{
			max_y = z->bitmask.get_offset().y + z->bitmask.get_height();
		}
	}
	min_x -= 6;
	min_y -= 6;
	max_x += 7;
	max_y += 7;
	int width = max_x - min_x;
	int height = max_y - min_y;
	
	
	for (Zone* z : all_zones)
	{
		Position zp = z->bitmask.get_offset();
		z->bitmask.set_offset(zp.x - min_x, zp.y - min_y);
	}
	
	//now that the skeleton of the overworld is set up, start adding meat to it
	gs.level = new Level(width, height);
	
	//create a generic enemy
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
	
	
	//shuffle a list of indexes into the theme arrays
	int indexes[] = {0,1,2,3,4,5,6};
	for (int i = 6; i > 0; --i)
	{
		int p = MyMath::random_int(0, i);
		int t = indexes[i];
		indexes[i] = indexes[p];
		indexes[p] = t;
	}
	
	//for each zone (in reverse order)...
	for (int i = (int)complete_zones.size() - 1; i >= 0; --i)
	{
		//fill in the zone definition
		Zone* z  = complete_zones[i];
		z->theme = indexes[i];
		
		//actually create the zone
		create_zone(gs, enemy_1, z);
	}
	
	//now connect each zone with its children
	for (Zone* z : all_zones)
	{
		//for each child of this zone...
		for (Zone* c : z->children)
		{
			//before making roads, add this child to the stack!
			stack.push_back(c);
			
			//create a composite bitmask to use
			Bitmask composite_bitmask(width,height, {0,0});
			composite_bitmask.set_all(true);
			composite_bitmask -= z->d.bitmask;
			composite_bitmask -= c->d.bitmask;
			
			//subtract all of the other zones
			for (Zone* oz : all_zones)
			{
				if (oz != z && oz != c)
				{
					composite_bitmask -= oz->bitmask;
					composite_bitmask -= oz->d.bitmask;
				}
			}
			
			//find the starting and ending points
			Position starting_position = z->d.entrance;
			++starting_position.y;
			Position ending_position = c->d.entrance;
			++ending_position.y;
			
			//now create a path between them
			std::vector<Position> steps = composite_bitmask.random_path(starting_position, ending_position, 1.5f);
			
			//now actually carve out that path
			std::size_t i;
			for (i = 0; i < steps.size(); ++i)
			{
				//get the step
				Position& p = steps[i];
				
				//now add it to the bitmask for later
				composite_bitmask.set(p.x,p.y,true);
			
				//if this room doesn't exist yet then create a bridge!
				if (gs.level->get_room(p.x,p.y) == nullptr)
				{
					#ifdef DEBUG
						if (i == 0 || i == steps.size() - 1)
						{
							Log::write("Error: Could not create bridge as there was no land to connect to!");
						}
					#endif
				
					//first up, create the room
					Room* r = gs.level->get_room(gs.level->create_room(p.x,p.y));
					
					//now fill in the strings
					r->set_description("A bridge leading between realms.");
					r->set_short_description("A Bridge");
					r->set_minimap_symbol("<fg=white><bg=magenta>#");
					r->set_visited(true);
					
					//now fill in the exits
					Position pp = steps[i - 1];
					Position np = steps[i + 1];
					if (pp.x < p.x || np.x < p.x)
					{
						r->set_exit(Room::Exit::WEST, Room::Exit_Status::Open);
					}
					if (pp.x > p.x || np.x > p.x)
					{
						r->set_exit(Room::Exit::EAST, Room::Exit_Status::Open);
					}
					if (pp.y < p.y || np.y < p.y)
					{
						r->set_exit(Room::Exit::NORTH, Room::Exit_Status::Open);
					}
					if (pp.y > p.y || np.y > p.y)
					{
						r->set_exit(Room::Exit::SOUTH, Room::Exit_Status::Open);
					}
				}
				//if this room does already exist then update it to look like a road
				else
				{
					gs.level->get_room(p.x, p.y)->set_minimap_symbol("<fg=white><bg=magenta>#");
					
					//also, fill in the exit if the previous or next step is a bridge
					if (i > 0)
					{
						Position pp = steps[i - 1];
						if (!z->bitmask(pp) && !c->bitmask(pp))//gs.level->get_room(pp.x, pp.y) == nullptr)
						{
							Room::Exit e = p.x < pp.x ? Room::Exit::EAST : p.x > pp.x ? Room::Exit::WEST : p.y < pp.y ? Room::Exit::SOUTH : Room::Exit::NORTH;
							
							gs.level->get_room(p.x, p.y)->set_exit(e, Room::Exit_Status::Open);
						}
					}
					if (i < steps.size() - 1)
					{
						Position np = steps[i + 1];
						if (!z->bitmask(np) && !c->bitmask(np))
						{
							Room::Exit e = p.x < np.x ? Room::Exit::EAST : p.x > np.x ? Room::Exit::WEST : p.y < np.y ? Room::Exit::SOUTH : Room::Exit::NORTH;
							
							gs.level->get_room(p.x, p.y)->set_exit(e, Room::Exit_Status::Open);
						}
					}
				}
			}
			
			//now step through the path and find the exit points for this zone
			for (i = 0; i < steps.size(); ++i)
			{
				Position p = steps[i];
				Position pn = steps[i + 1];
				
				//if this spot is land and the next spot is water
				if (z->bitmask(p) && !z->bitmask(pn))
				{
					//then look ahead to confirm that this is indeed the correct bridge
					bool right_bridge = false;
					for (std::size_t j = i + 1; j < steps.size(); ++j)
					{
						//if we hit the child zone, we're on the right bridge!
						if (c->bitmask(steps[j]))
						{
							right_bridge = true;
							break;
						}
						
						//if we backtracked into our own zone again, then this is not the right bridge
						else if (z->bitmask(steps[j]))
						{
							//skip ahead
							i = j - 1;
							break;
						}
					}
					
					//if it's the right bridge...
					if (right_bridge)
					{
						//store the exit
						z->exits[c] = p;
						
						//create the locked door
						std::string door_descriptions[] = {"A massive door made of obsidian and fire",
														   "A massive door made of tangled trees and vines",
														   "A massive door made of ivory and woven grass",
														   "A massive door made of sand and stone",
														   "A massive door made of stones capped with snow",
														   "A massive door made of snow and ice",
														   "A massive door made of marble"};
						Room::Exit e = p.x < pn.x ? Room::Exit::EAST : p.x > pn.x ? Room::Exit::WEST : p.y < pn.y ? Room::Exit::SOUTH : Room::Exit::NORTH;
						Room* er = gs.level->get_room(p.x,p.y);
						
						#ifdef DEBUG
							if (er == nullptr)
							{
								Log::write("Error: tried to create a gate in a non-existent room!");
							}
						#endif
						
						er->set_exit(e, Room::Exit_Status::Locked);
						er->set_door_description(door_descriptions[c->theme], e);
						
						//bust out
						break;
					}
				}
			}
			
			//now step through the rest of the path to find the entrance point for the child zone
			//TODO: determine if this is still necessary
			for (; i < steps.size(); ++i)
			{
				if (c->bitmask(steps[i]) && !c->bitmask(steps[i - 1]))
				{
					//store the exit
					c->exits[z] = steps[i];
					
					//bust out
					break;
				}
			}
		}
	}
	
	//now create the macguffins (the dungeons/zones will have references to them, but they won't be filled in yet)
	for (Zone* z : complete_zones)
	{
		Object* ko = gs.level->get_object(z->macguffin);
		if (z->macguffin_opens == nullptr)
		{
			//create a final macguffin (the princess)
			ko->visible = true;
			ko->visible_in_short_description = true;
			ko->friendly = true;
			ko->mobile = true;
			ko->playable = false;
			ko->open = true;
			ko->holdable = false;
			ko->object_container = -1;
			ko->hitpoints = -1;
			ko->attack = 0;
			ko->hit_chance = 0.0f;
			ko->name = "Princess xXx_SS5_Seph0r0th69_xXx";
			ko->description = "A beautiful princess. She wears a pink dress, white gloves, and looks strangely familiar...";
			ko->scripts.construct("(set 0 0);","(say (choose (get 0) \"Thank you for saving me brave warrior! You have my eternal thanks (and you won the game)!\" \"Thanks again (you can quit the game any time).\")); (set 0 1);","(say \"How dare you!\");","");
		}
		else
		{
			//get the room to unlock
			Position pos_to_unlock = z->macguffin_opens->parent->exits[z->macguffin_opens];
			Room* room_to_unlock = gs.level->get_room(pos_to_unlock.x, pos_to_unlock.y);
			int dir = room_to_unlock->get_exit(Room::Exit::NORTH) == Room::Exit_Status::Locked ? 0 :
					  room_to_unlock->get_exit(Room::Exit::EAST) == Room::Exit_Status::Locked ? 1 :
					  room_to_unlock->get_exit(Room::Exit::SOUTH) == Room::Exit_Status::Locked ? 2 : 3;
			char dirs[] = "nesw";
			std::string key_descs[] = {"A massive key made of obsidian and fire",
								      "A massive key made of tangled trees and vines",
								      "A massive key made of ivory and woven grass",
								      "A massive key made of sand and stone",
								      "A massive key made of stones capped with snow",
								      "A massive key made of snow and ice",
								      "A massive key made of marble"};
			std::string key_desc = key_descs[z->macguffin_opens->theme];
		
			#ifdef DEBUG
				if (room_to_unlock == nullptr)
				{
					Log::write("Error: couldn't lock the bridge because the exit room is invalid.");
				}
			#endif
		
			//create a key!
			ko->visible = true;
			ko->visible_in_short_description = false;
			ko->friendly = true;
			ko->mobile = false;
			ko->playable = false;
			ko->open = false;
			ko->holdable = true;
			ko->hitpoints = -1;
			ko->attack = 0;
			ko->hit_chance = 0.0f;
			ko->name = "A <fg=magenta>Massive <fg=green>Key";
			ko->description = key_desc + ". Find the bridge it unlocks!";
			ko->scripts.construct("",
								  "",
								  "(if (= (get current_room.handle) " + StringUtils::to_string((int)room_to_unlock->get_handle()) + ") (+ (set global.main_text (+ (get global.main_text) \"<fg=white><bg=black>\n\nThe gates of the bridge swing open, revealing a whole new realm.\")) (set current_room.open_" + dirs[dir] + " true) (destroy (get caller.handle))) (set global.main_text (+ (get global.main_text) \"<fg=white><bg=black>\n\nIt does nothing\")));",
								  "");
		}
	}
	
	//now fill up every spot not already taken up with water
	for (int x = 0; x < width; ++x)
	{
		for (int y = 0; y < height; ++y)
		{
			if (gs.level->get_room(x,y) == nullptr)
			{
				Room* r = gs.level->get_room(gs.level->create_room(x,y));
				r->set_description("So much water!");
				r->set_short_description("An Ocean of Water");
				r->set_minimap_symbol("<fg=white><bg=blue>~");
				r->set_visited(true);
			}
		}
	}
	
	//put the player object in the starting zone
	Room* sr = gs.level->get_room(starting_zone->d.entrance.x, starting_zone->d.entrance.y + 1);
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
	
	//make a map object and put it in the player's spot
	std::string ms = "";
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			if (gs.level->get_room(x,y)->get_visited())
			{
				ms += gs.level->get_room(x,y)->get_minimap_symbol();
			}
			else
			{
				ms += "<fg=black><bg=black> ";
			}
		}
		ms += "<fg=white><bg=black>\n";
	}
	o = gs.level->get_object(gs.level->create_object());
	o->visible = true;
	o->visible_in_short_description = false;
	o->friendly = true;
	o->mobile = false;
	o->playable = false;
	o->open = true;
	o->holdable = true;
	o->room_container = sr->get_handle();
	o->object_container = -1;
	o->hitpoints = -1;
	o->attack = 0;
	o->hit_chance = 0.0f;
	o->name = "A Map";
	o->description = "A map of all " + StringUtils::to_string(num_zones) + " realms.";
	o->scripts.construct("","","(set main_text (+ (get main_text) \"<fg=white><bg=black>\n" + ms + "\"));","");
	sr->objects().push_back(o->get_handle());
	
	//clean up the tree
	for (std::size_t i = 0; i < complete_zones.size(); ++i)
	{
		delete complete_zones[i];
		complete_zones[i] = nullptr;
	}
	complete_zones.clear();
	
	//print out a map of the overworld
	#ifdef DEBUG
		std::ofstream outfile("derp.txt");
		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				outfile << (gs.level->get_room(x,y)->get_minimap_symbol().back());
			}
			outfile << std::endl;
		}
	#endif
}

void PCG::create_world(GameState& gs)
{
	//set up the basic stuff
	gs.main_text = "";
	gs.main_text_dirty_flag = true;
	gs.frames_elapsed = 0;
	
	//create a couple generic enemies
	/*
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
	enemy_object->name = "Colin, the Bear";
	enemy_object->description = "A dancing creature.";

	enemy_object->scripts.construct("(set 0 0);",
									"(say (choose (random 0 3) \"Thanks for nothing!\" \"Thanks for nothing!\" \"Thanks for nothing!\" \"Thanks for nothing!\"));",
									"(set main_text (+ (get main_text) \"\n<fg=white><bg=black>You can't use an enemy!\"));",
									"(if (get combat.vulnerable_to_attack) (+ \"\" (set 0 2) (set main_text (+ (get main_text) \"<fg=green><bg=black>\nThe bear reels back from your attack!\")) (set combat.player_position_far_front true) ) (+ \"\" (if (get combat.player_attacking) (set 0 3)) (choose (get 0) (if (get combat.player_position_front) (+ \"\" (set main_text (+ (get main_text) \"<fg=white><bg=black>\nThe bear starts waving its mighty paws!\")) (set 0 1) (defend false false true true)) (+ \"\" (set main_text (+ (get main_text) \"<fg=white><bg=black>\n\" (if (get combat.player_position_far_front) \"The bear dances towards you!\" \"The goblin wiggles in your direction!\"))) (set combat.player_position_front true) (defend true true true true) (attack false false false false)))  (if (or (get combat.player_position_front) (get combat.player_position_far_front)) (+ \"\" (set 0 0) (attack false false true true) (set main_text (+ (get main_text) \"<fg=white><bg=black>\nThe bear slashes you while gyrating!\")) (defend true true true true)) (+ \"\" (set 0 0) (set main_text (+ (get main_text) \"<fg=white><bg=black>\nThe bear lands where you were standing!\")) (defend false false true true)))  (+ \"\" (set 0 0) (set main_text (+ (get main_text) \"<fg=white><bg=black>\nThe bear recovers and resumes dancing.\")) (defend true true true true)) (+ \"\" (set 0 0) (set main_text (+ (get main_text) \"<fg=white><bg=black>\nThe bear seems unaffected by your attack!\")) (defend true true true true))))); ");
	
	*/
	//create a couple generic enemies
	/*
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
	*/
	/*
	ECS::Handle enemy_1_spawner = gs.level->create_object();
	Object* spawner = gs.level->get_object(enemy_1_spawner);
	spawner->visible = false;
	spawner->visible_in_short_description = false;
	*/
	
	create_overworld(gs);
}
