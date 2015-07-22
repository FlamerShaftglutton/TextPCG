#pragma once
#include "Console.hpp"
#include "GameState.hpp"

class System
{
public:
	virtual void do_work(Console& console, GameState& gs) = 0;
};