#pragma once
#include "Console.hpp"
#include "GameState.hpp"
#include "ScriptingVariables.hpp"

class System
{
private:
	void fill_ObjectMap(Object* o, ObjectMap& om);
protected:
	void fill_scripting_variables(GameState& gs, ScriptingVariables& sv, Room* current_room, Object* caller);
	void unfill_scripting_variables(GameState& gs, ScriptingVariables& sv, Room* current_room);
public:
	virtual void do_work(Console& console, GameState& gs) = 0;
};
