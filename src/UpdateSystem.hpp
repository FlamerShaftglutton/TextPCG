#pragma once
#include "System.hpp"
#include "Console.hpp"
#include "GameState.hpp"

class UpdateSystem : public System
{

public:
	void do_work(Console& console, GameState& gs) override; //normally I'd pass in a delta, but we're going to lock this to X frames per second (probably 5)
};