#pragma once
#include "System.hpp"
#include "Handle.hpp"

class UpdateSystem : public System
{
	ECS::Handle current_room;
	int previous_menu;
	int combat_tick;
public:
	UpdateSystem(GameState& gs);
	void do_work(Console& console, GameState& gs) override; //normally I'd pass in a delta, but we're going to lock this to X frames per second (probably 5)
};
