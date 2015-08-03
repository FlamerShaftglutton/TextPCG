#pragma once
#include "System.hpp"
#include "Console.hpp"
#include "GameState.hpp"
#include <string>

class InputSystem : public System
{
	void command_look_room(Console& console, GameState& gs, ECS::Handle room, bool short_description);
	void command_look_object(Console& console, GameState& gs, std::string desc, std::string type);
	void command_use_object(Console& console, GameState& gs, std::string desc);
	void command_get_object(Console& console, GameState& gs, std::string desc);
	Object* find_object(GameState& gs, std::string desc);
public:
	void do_work(Console& console, GameState& gs) override;
};
