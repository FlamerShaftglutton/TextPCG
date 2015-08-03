#pragma once
#include "Handle.hpp"

struct CombatData
{
	ECS::Handle other;

	enum class Position
	{
		left,
		right,
		front,
		far_front
	} player_position;
	
	bool player_attacking;
	
	bool enemy_vulnerable_sides[4],//indexed with a Position variable
		 enemy_attacking_sides[4];
};