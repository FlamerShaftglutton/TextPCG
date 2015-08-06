#include <string>
#include <chrono>
#include "GameState.hpp"
#include "Console.hpp"
#include "Level.hpp"
#include "DrawSystem.hpp"
#include "InputSystem.hpp"
#include "UpdateSystem.hpp"
#include "Serialize.hpp"
#include "string_utils.hpp"
#include "UIConstants.hpp"

	#include <sstream>
	#include <fstream>

#ifdef DEBUG
	#include "Log.hpp"
#endif

void game_loop(Console& console)
{
	//set up the console frames
	int tw = console.get_width() - 10;
	int text_box_frame = console.add_frame(console.get_height() - 2, tw, 0, 0, false, false, true);
	int minimap_frame = console.add_frame(10, 10, 0, tw, false, false, false);
	int NPC_frame = console.add_frame(6, 10, 10, tw, false, false, false);
	int inventory_frame = console.add_frame(console.get_height() - 18, 10, 16, tw, false, false, false);
	int lower_bar_frame = console.add_frame(1, -1, console.get_height() - 2, 0, true, false, false);
	int echo_frame = console.add_frame(1, -1, console.get_height() - 1, 0, true, false, false);
	
	//set up the console echo frame (where the user input is displayed until ENTER is hit)
	console.set_echo_frame(echo_frame);
	console.set_echo_colors(Console::Color::White, Console::Color::Black);

	//set up the timing stuff
	auto start_time = std::chrono::high_resolution_clock::now();
	unsigned long mcount = 0;
	unsigned long frame_period = 1000/5;//5 FPS (updates)
	unsigned long next_frame = 0;
	unsigned long draw_period = 1000/30;//30 FPS (draws)
	unsigned long next_draw = 0;
	
	
	//set up the game state
	GameState gs;
	
	//Serialize::from_file("savedgame.tsf",gs);
	
	gs.menu_index = UI_State::Main_Menu;
	gs.main_text = "";
	gs.main_text_dirty_flag = true;
	gs.frames_elapsed = 0;
	gs.menu_transition = true;
	//*
	gs.level = new Level(9,9);
	
	//Debugging, create dummy level
	{
		for (int i = 1; i <4; ++i)
		{
			for (int j = 1; j < 4; ++j)
			{
				ECS::Handle h = gs.level->create_room(i,j);
				gs.level->get_room(h)->set_short_description("A Small Room");
				gs.level->get_room(h)->set_minimap_symbol("<fg=yellow><bg=yellow> ");
				gs.level->get_room(h)->set_visited(false);
			}
		}
	
		gs.level->get_room(1,1)->set_exit(Room::Exit::EAST,true);
		gs.level->get_room(1,1)->set_exit(Room::Exit::SOUTH,true);
		gs.level->get_room(1,1)->set_description("<fg=white>Room 1,1.\n<fg=green>Lawful Good!");
		
		gs.level->get_room(2,1)->set_exit(Room::Exit::WEST,true);
		gs.level->get_room(2,1)->set_exit(Room::Exit::SOUTH,true);
		gs.level->get_room(2,1)->set_description("<fg=white>Room 2,1.\n<fg=green>Lawful Neutral!");
		
		gs.level->get_room(3,1)->set_exit(Room::Exit::SOUTH,true);
		gs.level->get_room(3,1)->set_description("<fg=white>Room 3,1.\n<fg=green>Lawful Evil!");
		
		gs.level->get_room(1,2)->set_exit(Room::Exit::NORTH,true);
		gs.level->get_room(1,2)->set_exit(Room::Exit::SOUTH,true);
		gs.level->get_room(1,2)->set_description("<fg=white>Room 1,2.\n<fg=yellow>Neutral Good!");
		
		gs.level->get_room(2,2)->set_exit(Room::Exit::NORTH,true);
		gs.level->get_room(2,2)->set_exit(Room::Exit::SOUTH,true);
		gs.level->get_room(2,2)->set_description("<fg=white>Room 2,2.\n<fg=yellow>True neutral!");
		
		gs.level->get_room(3,2)->set_exit(Room::Exit::NORTH,true);
		gs.level->get_room(3,2)->set_exit(Room::Exit::SOUTH,true);
		gs.level->get_room(3,2)->set_description("<fg=white>Room 3,2.\n<fg=yellow>Evil neutral!");
		
		gs.level->get_room(1,3)->set_exit(Room::Exit::NORTH,true);
		gs.level->get_room(1,3)->set_description("<fg=white>Room 1,3.\n<fg=red>Chaotic Good!");
		
		gs.level->get_room(2,3)->set_exit(Room::Exit::EAST,true);
		gs.level->get_room(2,3)->set_exit(Room::Exit::NORTH,true);
		gs.level->get_room(2,3)->set_description("<fg=white>Room 2,3.\n<fg=red>Chaotic Neutral!");
		
		gs.level->get_room(3,3)->set_exit(Room::Exit::WEST,true);
		gs.level->get_room(3,3)->set_exit(Room::Exit::NORTH,true);
		gs.level->get_room(3,3)->set_exit(Room::Exit::EAST,false);
		gs.level->get_room(3,3)->set_description("<fg=white>Room 3,3.\n<fg=red>A small, cramped room. On the eastern wall is a locked <fg=green>door<fg=red>.");
		
		
		
		ECS::Handle h = gs.level->create_room(4,3);
		Room* r = gs.level->get_room(h);
		r->set_short_description("A circular entryway");
		r->set_minimap_symbol("<fg=yellow><bg=white> ");
		r->set_description("<fg=white>An <fg=purple>odd<fg=white> circular entryway. The walls and floors are cold masonry, and on all sides are stone archways leading to more rooms.");
		r->set_exit(Room::Exit::WEST,true);
		r->set_exit(Room::Exit::NORTH,true);
		r->set_exit(Room::Exit::EAST,true);
		r->set_exit(Room::Exit::SOUTH,true);
		
		h = gs.level->create_room(4,2);
		r = gs.level->get_room(h);
		r->set_short_description("A dead end");
		r->set_minimap_symbol("<fg=yellow><bg=white> ");
		r->set_description("<fg=white>A small rectangular room with no visible exit but from the way you came in. The walls and floors are cold masonry, but the corners are dark and hard to see.");
		r->set_exit(Room::Exit::SOUTH,true);
		
		h = gs.level->create_room(5,3);
		r = gs.level->get_room(h);
		r->set_short_description("A dead end");
		r->set_minimap_symbol("<fg=yellow><bg=white> ");
		r->set_description("<fg=white>A small rectangular room with no visible exit but from the way you came in. The walls and floors are cold masonry, but the corners are dark and hard to see.");
		r->set_exit(Room::Exit::WEST,true);
		
		h = gs.level->create_room(4,4);
		r = gs.level->get_room(h);
		r->set_short_description("A dead end");
		r->set_minimap_symbol("<fg=yellow><bg=white> ");
		r->set_description("<fg=white>A small rectangular room with no visible exit but from the way you came in. The walls and floors are cold masonry, but the corners are dark and hard to see.");
		r->set_exit(Room::Exit::NORTH,true);
		
		gs.playable_character = gs.level->create_object();
		Object* o = gs.level->get_object(gs.playable_character);
		o->visible = true;
		o->visible_in_short_description = true;
		o->friendly = true;
		o->mobile = true;
		o->playable = true;
		o->open = true;
		o->holdable = false;
		o->room_container = gs.level->get_room(2,2)->get_handle();
		o->object_container = -1;
		o->hitpoints = 100;
		o->attack = 10;
		o->hit_chance = 0.75f;
		o->name = "Myself";
		o->description = "A <fg=red>hideous<fg=white> looking human. Possibly beaten, or possibly just always ugly. Hard to tell.";
		o->scripts.construct("","","","");
		gs.level->get_room(2,2)->objects().push_back(o->get_handle());
		
		o = gs.level->get_object(gs.level->create_object());
		o->visible = true;
		o->visible_in_short_description = true;
		o->friendly = true;
		o->mobile = true;
		o->playable = false;
		o->open = false;
		o->holdable = false;
		o->room_container = r->get_handle();
		o->object_container = -1;
		o->hitpoints = 12;
		o->attack = 0;
		o->hit_chance = 0.0f;
		o->name = "Mysterious underdweller";
		o->description = "A somewhat short man in a dark gray cloak. He mutters to himself while eyeing you.";
		o->scripts.construct("(set 0 0);",
							 "(say (choose (get 0) \"Hello stranger.\" \"You again?\"));(set 0 1);",
							 "(say \"Err... Sorry, I'm taken, buddy.\");",
							 "");
		r->objects().push_back(o->get_handle());
		
		r = gs.level->get_room(1,3);
		
		o = gs.level->get_object(gs.level->create_object());
		o->visible = true;
		o->visible_in_short_description = true;
		o->friendly = false;
		o->mobile = true;
		o->playable = false;
		o->open = false;
		o->holdable = false;
		o->room_container = r->get_handle();
		o->object_container = -1;
		o->hitpoints = 12;
		o->attack = 1;
		o->hit_chance = 0.0f;
		o->name = "An Evil Goblin";
		o->description = "An ugly creature with beady bloodthirsty eyes.";
		
		std::string s;
		std::ifstream infile("expressions.txt");
		std::stringstream ss;
		ss << infile.rdbuf();
		s = StringUtils::replace(ss.str(),"\\n","\n");
		
		o->scripts.construct("(set 0 0);",
							 "(say \"SCREEE!!!\");",
							 "(set main_text (+ (get main_text) \"\n<fg=white><bg=black>You can't use an enemy!\"));",
							 s);
		r->objects().push_back(o->get_handle());
		ECS::Handle oh = o->get_handle();
		
		o = gs.level->get_object(gs.level->create_object());
		o->visible = true;
		o->visible_in_short_description = false;
		o->friendly = true;
		o->mobile = false;
		o->playable = false;
		o->open = false;
		o->holdable = true;
		o->room_container = -1;
		o->object_container = oh;
		o->hitpoints = -1;
		o->attack = 0;
		o->hit_chance = 0.0f;
		o->name = "A key";
		o->description = "A small iron skeleton key. It is covered in small bumps, yet the texture feels smooth.";
		o->scripts.construct("",
							 "",
							 "(if (= (get current_room.handle) " + StringUtils::to_string(gs.level->get_room(3,3)->get_handle()) + ") (+ (set global.main_text (+ (get global.main_text) \"\n\nThe key opens a door to the east.\")) (set current_room.open_e true) (set caller.destroyed true)) (set global.main_text (+ (get global.main_text) \"<fg=white><bg=black>\n\nIt does nothing\")));",
							 "");
		
		gs.level->get_object(oh)->objects.push_back(o->get_handle());
		
		gs.combat_data = nullptr;
		
		Serialize::to_file("newgame.tsf",gs);
	}
	//*/
	
	//set up our systems
	InputSystem input_system;
	UpdateSystem update_system(gs);
	DrawSystem draw_system(text_box_frame, lower_bar_frame, minimap_frame, NPC_frame, inventory_frame);
	
	//loop until something calls 'break'
	for(int counter = 0;;++counter)
	{
		//check for input from the user
		input_system.do_work(console,gs);
		
		//if the user quit, then kill the program
		if (gs.menu_index == UI_State::Exit)
			break;
	
		//if we're ready for another frame, fill it in!
		mcount = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count();
		if (mcount > next_frame)
		{
			++gs.frames_elapsed;
			next_frame += frame_period;
			
			update_system.do_work(console, gs);//pass in the number of ms since the last frame
		}
		
		//if we're ready for another draw, draw it!
		if (mcount > next_draw)
		{
			next_draw += draw_period;
			draw_system.do_work(console, gs);
		}
	}
	
	Serialize::to_file("savedgame.tsf",gs);
	
	//clean up some stuff
	delete gs.level;
}

int main()
{
	//set up the console with the settings we want
	Console console;
	
	game_loop(console);
	
	return 0;
}
