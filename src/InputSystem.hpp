#pragma once
#include "System.hpp"
#include "Console.hpp"
#include "GameState.hpp"

class InputSystem : public System
{

public:
	void do_work(Console& console, GameState& gs) override;
};