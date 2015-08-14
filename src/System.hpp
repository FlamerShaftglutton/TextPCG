#pragma once

//forward declarations
class Console;
struct GameState;
struct Object;
class Room;
struct ObjectMap;


class System
{
public:
	virtual void do_work(Console& console, GameState& gs) = 0;
};
