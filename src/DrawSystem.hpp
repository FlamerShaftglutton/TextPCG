#pragma once
#include "System.hpp"
#include "Console.hpp"
#include "GameState.hpp"

class DrawSystem : public System
{
	int text_box_frame, 
		lower_bar_frame,
		minimap_frame,
		NPC_frame,
		inventory_frame;
	
	void outline_frame(Console& console, int frame, bool top, bool bottom, bool left, bool right);
	void draw_minimap(Console& console, GameState& gs);
	void draw_NPCs(Console& console, GameState& gs);
	
public:
	DrawSystem(int text_box_f, int lower_bar_f, int minimap_f, int NPC_f, int inventory_f);
	void do_work(Console& console, GameState& gs) override;
};