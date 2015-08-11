#pragma once
#include "System.hpp"
#include "Console.hpp"
#include "GameState.hpp"

class DrawSystem : public System
{
	void outline_frame(Console& console, int frame, bool top, bool bottom, bool left, bool right);
	void draw_minimap(Console& console, GameState& gs, int minimap_frame);
	void draw_NPCs(Console& console, GameState& gs, int NPC_frame);
	void draw_inventory(Console& console, GameState& gs, int inventory_frame);
	
public:
	void do_work(Console& console, GameState& gs) override;
};
