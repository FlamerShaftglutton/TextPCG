#pragma once
#include "System.hpp"
#include "Console.hpp"
#include "GameState.hpp"
#include <string>

class InputSystem : public System
{
	void command_look_room(Console& console, GameState& gs, ECS::Handle room, bool short_description);
	void command_look_object(Console& console, GameState& gs, std::string desc, std::string type);
public:
	void do_work(Console& console, GameState& gs) override;
};