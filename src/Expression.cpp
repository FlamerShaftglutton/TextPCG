#include "Expression.hpp"
#include <vector>
#include <string>
#include "GameState.hpp"
#include "Level.hpp"
#include "Room.hpp"
#include "Object.hpp"
#include "CombatData.hpp"
#include "mymath.hpp"
#include <cmath>
#include "string_utils.hpp"

#ifdef DEBUG
	#include "Log.hpp"
#endif

Value* copy_value(Value* v)
{
	Value* retval = new Value;
	
	retval->type = v->type;
	
	switch (v->type)
	{
		case Value::Value_Type::Int: retval->int_val = v->int_val; break;
		case Value::Value_Type::Bool: retval->bool_val = v->bool_val; break;
		case Value::Value_Type::Float: retval->float_val = v->float_val; break;
		case Value::Value_Type::String: retval->string_val = v->string_val; break;
		case Value::Value_Type::Error: break;
	}
	
	return retval;
}

bool Value::construct(std::vector<Expression*> arguments)
{
	return true;
}

Value* Value::evaluate(GameState& gs, ECS::Handle caller)
{
	return copy_value(this);
}

std::string Value::to_string()
{
	switch (type)
	{
		case Value::Value_Type::Int: return StringUtils::to_string(int_val);
		case Value::Value_Type::Bool: return StringUtils::to_string((int)bool_val);
		case Value::Value_Type::Float: return StringUtils::to_string(float_val);
		case Value::Value_Type::String: return "\"" + string_val + "\"";
		default: return "\"\"";//should probably do something with this
	}
}



bool Choose_Expression::construct(std::vector<Expression*> arguments)
{
	if (arguments.size() < 3)
	{
		return false;
	}
	
	args = arguments;
	return true;
}

Choose_Expression::~Choose_Expression()
{
	for (unsigned i = 0; i < args.size(); ++i)
	{
		delete args[i];
		args[i] = nullptr;
	}
}

Value* Choose_Expression::evaluate(GameState& gs, ECS::Handle caller)
{
	Value* index = args[0]->evaluate(gs,caller);
	int i = index->int_val;
	
	if (index->type != Value::Value_Type::Int || i > ((int)args.size() - 2) || i < 0)
	{
		#ifdef DEBUG
			Log::write("ERROR: index of choose function evaluated to ERROR.");
		#endif
	
		index->type = Value::Value_Type::Error;
		return index;
	}
	
	delete index;
	return args[i + 1]->evaluate(gs,caller);
}



bool Random_Expression::construct(std::vector<Expression*> arguments)
{
	if (arguments.size() != 2)
	{
		return false;
	}
	
	lower_limit = arguments[0];
	upper_limit = arguments[1];
	return true;
}

Random_Expression::~Random_Expression()
{
	delete lower_limit;
	delete upper_limit;
	lower_limit = nullptr;
	upper_limit = nullptr;
}

Value* Random_Expression::evaluate(GameState& gs, ECS::Handle caller)
{
	Value* ll = lower_limit->evaluate(gs,caller);
	Value* ul = upper_limit->evaluate(gs,caller);
	
	if (ll->type != ul->type || ll->type != Value::Value_Type::Int)
	{
		#ifdef DEBUG
			Log::write("ERROR: One or both arguments for the Random function were not integers.");
		#endif
	
		delete ll;
		ul->type = Value::Value_Type::Error;
		return ul;
	}
	
	ul->int_val = MyMath::random_int(ll->int_val,ul->int_val);
	delete ll;
	return ul;
}



Set_Register_Expression::Set_Register_Expression(Value* register_val, Expression* arg)
{
	reg = register_val;
	argument = arg;
}

bool Set_Register_Expression::construct(std::vector<Expression*> arguments)
{
	return true;
}

Set_Register_Expression::~Set_Register_Expression()
{
	//note: it is not the Set Expression's job to clean up the register it references. The Script object will do that.

	delete argument;
	argument = nullptr;
}

Value* Set_Register_Expression::evaluate(GameState& gs, ECS::Handle caller)
{
	Value* v = argument->evaluate(gs,caller);
	
	if (reg != nullptr)
	{
		reg->bool_val = v->bool_val;
		reg->int_val = v->int_val;
		reg->float_val = v->float_val;
		reg->string_val = v->string_val;
		
		reg->type = v->type;
	}
	
	return v;
}



Get_Register_Expression::Get_Register_Expression(Value* register_val)
{
	reg = register_val;
}

bool Get_Register_Expression::construct(std::vector<Expression*> arguments)
{
	return true;
}

Value* Get_Register_Expression::evaluate(GameState& gs, ECS::Handle caller)
{
	if (reg != nullptr)
	{
		return copy_value(reg);
	}
	
	Value* v = new Value;
	v->type = Value::Value_Type::Error;
	return v;
}



Variable_Expression::Variable_Expression(std::string vname)
{
	//first off, if the scope is global, just strip that off
	std::vector<std::string> chunks = StringUtils::split(StringUtils::to_lowercase(vname),'.');
	if (chunks.size() > 0 && chunks[0] == "global")
	{
		chunks.erase(chunks.begin());
	}
	
	//start looking through the variable names
	well_formed = false;
	if (chunks.size() > 0)
	{
		if (chunks[0] == "main_text")
		{
			well_formed = chunks.size() == 1;
		
			global_variable = Expression_Variable_Global::main_text;
		}
		else if (chunks.size() == 2)
		{
			std::string& c1 = chunks[1];
			
			if (chunks[0] == "current_room")
			{
				std::vector<std::string> names = {"handle","description","short_description","minimap_symbol","visited","open_n","open_e","open_s","open_w"};
				std::vector<Expression_Variable_Room> values = {Expression_Variable_Room::handle,Expression_Variable_Room::description,Expression_Variable_Room::short_description,Expression_Variable_Room::minimap_symbol,Expression_Variable_Room::visited,Expression_Variable_Room::open_n,Expression_Variable_Room::open_e,Expression_Variable_Room::open_s,Expression_Variable_Room::open_w};
				global_variable = Expression_Variable_Global::current_room;
				
				for (unsigned i = 0; i < names.size(); ++i)
				{
					if (c1 == names[i])
					{
						well_formed = true;
						room_variable = values[i];
						break;
					}
				}
			}
			else if (chunks[0] == "player" || chunks[0] == "caller" || chunks[0] == "object_iterator")
			{
				if (chunks[0] == "player")
				{
					global_variable = Expression_Variable_Global::player;
				}
				else if (chunks[0] == "caller")
				{
					global_variable = Expression_Variable_Global::caller;
				}
				else
				{
					global_variable = Expression_Variable_Global::object_iterator;
				}
				
				std::vector<std::string> names = {"handle","visible","visible_in_short_description","friendly","mobile","playable","open","holdable","hitpoints","attack","hit_chance","description","name"};
				std::vector<Expression_Variable_Object> values = {Expression_Variable_Object::handle,Expression_Variable_Object::visible,Expression_Variable_Object::visible_in_short_description,Expression_Variable_Object::friendly,Expression_Variable_Object::mobile,Expression_Variable_Object::playable,Expression_Variable_Object::open,Expression_Variable_Object::holdable,Expression_Variable_Object::hitpoints,Expression_Variable_Object::attack,Expression_Variable_Object::hit_chance,Expression_Variable_Object::description,Expression_Variable_Object::name};
				for (unsigned i = 0; i < names.size(); ++i)
				{
					if (c1 == names[i])
					{
						well_formed = true;
						object_variable = values[i];
						break;
					}
				}
			}
			else if (chunks[0] == "combat")
			{
				global_variable = Expression_Variable_Global::combat_data;
				std::vector<std::string> names = {"player_position_left","player_position_right","player_position_front","player_position_far_front","player_attacking","vulnerable_left","vulnerable_right","vulnerable_front","vulnerable_far_front","attacking_left","attacking_right","attacking_front","attacking_far_front","vulnerable_to_attack"};
				std::vector<Expression_Variable_Combat> values = {Expression_Variable_Combat::player_position_left,Expression_Variable_Combat::player_position_right,Expression_Variable_Combat::player_position_front,Expression_Variable_Combat::player_position_far_front,Expression_Variable_Combat::player_attacking,Expression_Variable_Combat::vulnerable_left,Expression_Variable_Combat::vulnerable_right,Expression_Variable_Combat::vulnerable_front,Expression_Variable_Combat::vulnerable_far_front,Expression_Variable_Combat::attacking_left,Expression_Variable_Combat::attacking_right,Expression_Variable_Combat::attacking_front,Expression_Variable_Combat::attacking_far_front,Expression_Variable_Combat::vulnerable_to_attack};
				
				for (unsigned i = 0; i < names.size(); ++i)
				{
					if (c1 == names[i])
					{
						well_formed = true;
						combat_variable = values[i];
						break;
					}
				}
			}
		}
	}
	
	#ifdef DEBUG
		if (!well_formed)
		{
			Log::write("ERROR: could not resolve variable name '" + vname + "'.");
		}
	#endif
}



Set_Variable_Expression::Set_Variable_Expression(std::string vname, Expression* arg) : Variable_Expression(vname)
{
	argument = arg;
}

bool Set_Variable_Expression::construct(std::vector<Expression*> arguments)
{
	return well_formed;
}
Set_Variable_Expression::~Set_Variable_Expression()
{
	delete argument;
	argument = nullptr;
}

Value* Set_Variable_Expression::evaluate(GameState& gs, ECS::Handle caller)
{
	Value* v = argument->evaluate(gs,caller);
	bool correct_type = false;
	
	//if it's the global text...
	if (global_variable == Expression_Variable_Global::main_text)
	{
		if (v->type == Value::Value_Type::String)
		{
			gs.main_text = v->string_val;
			gs.main_text_dirty_flag = true;
			correct_type = true;
		}
	}
	//if it's a room variable for the current_room
	else if (global_variable == Expression_Variable_Global::current_room)
	{
		Room* r = gs.level->get_room(gs.level->get_object(gs.playable_character)->room_container);
		switch (room_variable)
		{
			case Expression_Variable_Room::handle:
				#ifdef DEBUG
					Log::write("ERROR: Set function tried to set the <room>.handle field, which is read-only. Nothing changed.");
				#endif
				break;
			case Expression_Variable_Room::description: 
				if (v->type == Value::Value_Type::String)
				{
					r->set_description(v->string_val);
					correct_type = true;
				}
				break;
			case Expression_Variable_Room::short_description:
				if (v->type == Value::Value_Type::String)
				{
					r->set_short_description(v->string_val);
					correct_type = true;
				}
				break;
			case Expression_Variable_Room::minimap_symbol:
				if (v->type == Value::Value_Type::String)
				{
					r->set_minimap_symbol(v->string_val);
					correct_type = true;
				}
				break;
			case Expression_Variable_Room::visited:
				break;
				if (v->type == Value::Value_Type::Bool)
				{
					r->set_visited(v->bool_val);
					correct_type = true;
				}
				break;
			case Expression_Variable_Room::open_n:
				if (v->type == Value::Value_Type::Bool)
				{
					r->set_exit(Room::Exit::NORTH, v->bool_val);
					correct_type = true;
				}
				break;
			case Expression_Variable_Room::open_e:
				if (v->type == Value::Value_Type::Bool)
				{
					r->set_exit(Room::Exit::EAST, v->bool_val);
					correct_type = true;
				}
				break;
			case Expression_Variable_Room::open_s:
				if (v->type == Value::Value_Type::Bool)
				{
					r->set_exit(Room::Exit::SOUTH, v->bool_val);
					correct_type = true;
				}
				break;
			case Expression_Variable_Room::open_w:
				if (v->type == Value::Value_Type::Bool)
				{
					r->set_exit(Room::Exit::WEST, v->bool_val);
					correct_type = true;
				}
				break;
		}
	}
	//if it's a combat variable
	else if (global_variable == Expression_Variable_Global::combat_data)
	{
		//first off, blow up if we aren't in combat right now
		if (gs.combat_data == nullptr)
		{
			#ifdef DEBUG
				Log::write("ERROR:Tried to alter a combat variable outside of combat!");
			#endif
			
			v->type = Value::Value_Type::Error;
			correct_type = true;
		}
		else
		{
			switch (combat_variable)
			{
				case Expression_Variable_Combat::player_position_left:
					if (v->type == Value::Value_Type::Bool)
					{
						gs.combat_data->player_position =  CombatData::Position::left;
						correct_type = true;
					}
					break;
				case Expression_Variable_Combat::player_position_right:
					if (v->type == Value::Value_Type::Bool)
					{
						gs.combat_data->player_position =  CombatData::Position::right;
						correct_type = true;
					}
					break;
				case Expression_Variable_Combat::player_position_front:
					if (v->type == Value::Value_Type::Bool)
					{
						gs.combat_data->player_position =  CombatData::Position::front;
						correct_type = true;
					}
					break;
				case Expression_Variable_Combat::player_position_far_front:
					if (v->type == Value::Value_Type::Bool)
					{
						gs.combat_data->player_position =  CombatData::Position::far_front;
						correct_type = true;
					}
					break;
				case Expression_Variable_Combat::player_attacking:
					if (v->type == Value::Value_Type::Bool)
					{
						gs.combat_data->player_attacking = v->bool_val;
						correct_type = true;
					}
					break;
				case Expression_Variable_Combat::vulnerable_left:
					if (v->type == Value::Value_Type::Bool)
					{
						gs.combat_data->enemy_vulnerable_sides[(int)CombatData::Position::left] = v->bool_val;
						correct_type = true;
					}
					break;
				case Expression_Variable_Combat::vulnerable_right:
					if (v->type == Value::Value_Type::Bool)
					{
						gs.combat_data->enemy_vulnerable_sides[(int)CombatData::Position::right] = v->bool_val;
						correct_type = true;
					}
					break;
				case Expression_Variable_Combat::vulnerable_front:
					if (v->type == Value::Value_Type::Bool)
					{
						gs.combat_data->enemy_vulnerable_sides[(int)CombatData::Position::front] = v->bool_val;
						correct_type = true;
					}
					break;
				case Expression_Variable_Combat::vulnerable_far_front:
					if (v->type == Value::Value_Type::Bool)
					{
						gs.combat_data->enemy_vulnerable_sides[(int)CombatData::Position::far_front] = v->bool_val;
						correct_type = true;
					}
					break;
				case Expression_Variable_Combat::attacking_left:
					if (v->type == Value::Value_Type::Bool)
					{
						gs.combat_data->enemy_attacking_sides[(int)CombatData::Position::left] = v->bool_val;
						correct_type = true;
					}
					break;
				case Expression_Variable_Combat::attacking_right:
					if (v->type == Value::Value_Type::Bool)
					{
						gs.combat_data->enemy_attacking_sides[(int)CombatData::Position::right] = v->bool_val;
						correct_type = true;
					}
					break;
				case Expression_Variable_Combat::attacking_front:
					if (v->type == Value::Value_Type::Bool)
					{
						gs.combat_data->enemy_attacking_sides[(int)CombatData::Position::front] = v->bool_val;
						correct_type = true;
					}
					break;
				case Expression_Variable_Combat::attacking_far_front:
					if (v->type == Value::Value_Type::Bool)
					{
						gs.combat_data->enemy_attacking_sides[(int)CombatData::Position::far_front] = v->bool_val;
						correct_type = true;
					}
					break;
				case Expression_Variable_Combat::vulnerable_to_attack:
					#ifdef DEBUG
						Log::write("ERROR: Set function tried to set the combat.vulnerable_to_attack field, which is read-only. Nothing changed.");
					#endif
					break;
			}
		}
	}
	//if it's an object variable
	else
	{
		//which object?
		Object* om = global_variable == Expression_Variable_Global::caller ? gs.level->get_object(caller) : global_variable == Expression_Variable_Global::player ? gs.level->get_object(gs.playable_character) : nullptr;//pv.object_iterator;//TODO: fix iteration!
		
		//which field?
		switch (object_variable)
		{
			case Expression_Variable_Object::handle:
				#ifdef DEBUG
					Log::write("ERROR: Set function tried to set the <object>.handle field, which is read-only. Nothing changed.");
				#endif
				break;
			case Expression_Variable_Object::visible:
				if (v->type == Value::Value_Type::Bool)
				{
					correct_type = true;
					om->visible = v->bool_val;
				}
				break;
			case Expression_Variable_Object::visible_in_short_description:
				if (v->type == Value::Value_Type::Bool)
				{
					correct_type = true;
					om->visible_in_short_description = v->bool_val;
				}
				break;
			case Expression_Variable_Object::friendly:
				if (v->type == Value::Value_Type::Bool)
				{
					correct_type = true;
					om->friendly = v->bool_val;
				}
				break;
			case Expression_Variable_Object::mobile:
				if (v->type == Value::Value_Type::Bool)
				{
					correct_type = true;
					om->mobile = v->bool_val;
				}
				break;
			case Expression_Variable_Object::playable:
				#ifdef DEBUG
					Log::write("ERROR: Set function tried to set the <object>.playable field, which is read-only. Nothing changed.");
				#endif
				break;
			case Expression_Variable_Object::open:
				if (v->type == Value::Value_Type::Bool)
				{
					correct_type = true;
					om->open = v->bool_val;
				}
				break;
			case Expression_Variable_Object::holdable:
				if (v->type == Value::Value_Type::Bool)
				{
					correct_type = true;
					om->holdable = v->bool_val;
				}
				break;
			case Expression_Variable_Object::hitpoints:
				if (v->type == Value::Value_Type::Int)
				{
					correct_type = true;
					om->hitpoints = v->int_val;
				}
				break;
			case Expression_Variable_Object::attack:
				if (v->type == Value::Value_Type::Int)
				{
					correct_type = true;
					om->attack = v->int_val;
				}
				break;
			case Expression_Variable_Object::hit_chance:
				if (v->type == Value::Value_Type::Float)
				{
					correct_type = true;
					om->hit_chance = v->float_val;
				}
				break;
			case Expression_Variable_Object::description:
				if (v->type == Value::Value_Type::String)
				{
					correct_type = true;
					om->description = v->string_val;
				}
				break;
			case Expression_Variable_Object::name:
				if (v->type == Value::Value_Type::String)
				{
					correct_type = true;
					om->name = v->string_val;
				}
				break;
		}
	}
	
	if (!correct_type)
	{
		#ifdef DEBUG
			Log::write("ERROR:Invalid type for variable in Set call. Value not changed.");
		#endif
		
		v->type = Value::Value_Type::Error;
	}
	
	return v;
}



Get_Variable_Expression::Get_Variable_Expression(std::string vname) : Variable_Expression(vname)
{
	//the base class takes care of everything!
}

bool Get_Variable_Expression::construct(std::vector<Expression*> arguments)
{
	return well_formed;
}

Value* Get_Variable_Expression::evaluate(GameState& gs, ECS::Handle caller)
{
	Value* v = new Value;
	v->type = Value::Value_Type::Error;//this function should never reach the end before setting the value, but just in case...
	
	//if it's the global text...
	if (global_variable == Expression_Variable_Global::main_text)
	{
		v->type = Value::Value_Type::String;
		v->string_val = gs.main_text;
	}
	//if it's a room variable for the current_room
	else if (global_variable == Expression_Variable_Global::current_room)
	{
		Room* r = gs.level->get_room(gs.level->get_object(gs.playable_character)->room_container);
		switch (room_variable)
		{
			case Expression_Variable_Room::handle:
				v->type = Value::Value_Type::Int;
				v->int_val = r->get_handle();
				break;
			case Expression_Variable_Room::description:
				v->type = Value::Value_Type::String;
				v->string_val = r->get_description();
				break;
			case Expression_Variable_Room::short_description:
				v->type = Value::Value_Type::String;
				v->string_val = r->get_short_description();
				break;
			case Expression_Variable_Room::minimap_symbol:
				v->type = Value::Value_Type::String;
				v->string_val = r->get_minimap_symbol();
				break;
			case Expression_Variable_Room::visited:
				v->type = Value::Value_Type::Bool;
				v->bool_val = r->get_visited();
				break;
			case Expression_Variable_Room::open_n:
				v->type = Value::Value_Type::Bool;
				v->bool_val = r->get_exit(Room::Exit::NORTH);
				break;
			case Expression_Variable_Room::open_e:
				v->type = Value::Value_Type::Bool;
				v->bool_val = r->get_exit(Room::Exit::EAST);
				break;
			case Expression_Variable_Room::open_s:
				v->type = Value::Value_Type::Bool;
				v->bool_val = r->get_exit(Room::Exit::SOUTH);
				break;
			case Expression_Variable_Room::open_w:
				v->type = Value::Value_Type::Bool;
				v->bool_val = r->get_exit(Room::Exit::WEST);
				break;
		}
	}
	//if it's a combat variable
	else if (global_variable == Expression_Variable_Global::combat_data)
	{
		if (gs.combat_data == nullptr)
		{
			#ifdef DEBUG
				Log::write("ERROR:Tried to access combat variable outside of combat!");
			#endif
			
			v->type = Value::Value_Type::Error;
		}
		else
		{
			switch (combat_variable)
			{
				case Expression_Variable_Combat::player_position_left:
					v->type = Value::Value_Type::Bool;
					v->bool_val = gs.combat_data->player_position == CombatData::Position::left;
					break;
				case Expression_Variable_Combat::player_position_right:
					v->type = Value::Value_Type::Bool;
					v->bool_val = gs.combat_data->player_position == CombatData::Position::right;
					break;
				case Expression_Variable_Combat::player_position_front:
					v->type = Value::Value_Type::Bool;
					v->bool_val = gs.combat_data->player_position == CombatData::Position::front;
					break;
				case Expression_Variable_Combat::player_position_far_front:
					v->type = Value::Value_Type::Bool;
					v->bool_val = gs.combat_data->player_position == CombatData::Position::far_front;
					break;
				case Expression_Variable_Combat::player_attacking:
					v->type = Value::Value_Type::Bool;
					v->bool_val = gs.combat_data->player_attacking;
					break;
				case Expression_Variable_Combat::vulnerable_left:
					v->type = Value::Value_Type::Bool;
					v->bool_val = gs.combat_data->enemy_vulnerable_sides[(int)CombatData::Position::left];
					break;
				case Expression_Variable_Combat::vulnerable_right:
					v->type = Value::Value_Type::Bool;
					v->bool_val = gs.combat_data->enemy_vulnerable_sides[(int)CombatData::Position::right];
					break;
				case Expression_Variable_Combat::vulnerable_front:
					v->type = Value::Value_Type::Bool;
					v->bool_val = gs.combat_data->enemy_vulnerable_sides[(int)CombatData::Position::front];
					break;
				case Expression_Variable_Combat::vulnerable_far_front:
					v->type = Value::Value_Type::Bool;
					v->bool_val = gs.combat_data->enemy_vulnerable_sides[(int)CombatData::Position::far_front];
					break;
				case Expression_Variable_Combat::attacking_left:
					v->type = Value::Value_Type::Bool;
					v->bool_val = gs.combat_data->enemy_attacking_sides[(int)CombatData::Position::left];
					break;
				case Expression_Variable_Combat::attacking_right:
					v->type = Value::Value_Type::Bool;
					v->bool_val = gs.combat_data->enemy_attacking_sides[(int)CombatData::Position::right];
					break;
				case Expression_Variable_Combat::attacking_front:
					v->type = Value::Value_Type::Bool;
					v->bool_val = gs.combat_data->enemy_attacking_sides[(int)CombatData::Position::front];
					break;
				case Expression_Variable_Combat::attacking_far_front:
					v->type = Value::Value_Type::Bool;
					v->bool_val = gs.combat_data->enemy_attacking_sides[(int)CombatData::Position::far_front];
					break;
				case Expression_Variable_Combat::vulnerable_to_attack:
					v->type = Value::Value_Type::Bool;
					v->bool_val = gs.combat_data->vulnerable_to_attack;
					break;
			}
		}
	}
	//if it's an object variable
	else
	{
		//which object?
		Object* om = global_variable == Expression_Variable_Global::caller ? gs.level->get_object(caller) : global_variable == Expression_Variable_Global::player ? gs.level->get_object(gs.playable_character) : nullptr;//pv.object_iterator;//TODO: fix iteration!
		
		//which field?
		switch (object_variable)
		{
			case Expression_Variable_Object::handle:
				v->type = Value::Value_Type::Int;
				v->int_val = om->get_handle();
				break;
			case Expression_Variable_Object::visible:
				v->type = Value::Value_Type::Bool;
				v->bool_val = om->visible;
				break;
			case Expression_Variable_Object::visible_in_short_description:
				v->type = Value::Value_Type::Bool;
				v->bool_val = om->visible_in_short_description;
				break;
			case Expression_Variable_Object::friendly:
				v->type = Value::Value_Type::Bool;
				v->bool_val = om->friendly;
				break;
			case Expression_Variable_Object::mobile:
				v->type = Value::Value_Type::Bool;
				v->bool_val = om->mobile;
				break;
			case Expression_Variable_Object::playable:
				v->type = Value::Value_Type::Bool;
				v->bool_val = om->playable;
				break;
			case Expression_Variable_Object::open:
				v->type = Value::Value_Type::Bool;
				v->bool_val = om->open;
				break;
			case Expression_Variable_Object::holdable:
				v->type = Value::Value_Type::Bool;
				v->bool_val = om->holdable;
				break;
			case Expression_Variable_Object::hitpoints:
				v->type = Value::Value_Type::Int;
				v->int_val = om->hitpoints;
				break;
			case Expression_Variable_Object::attack:
				v->type = Value::Value_Type::Int;
				v->int_val = om->attack;
				break;
			case Expression_Variable_Object::hit_chance:
				v->type = Value::Value_Type::Float;
				v->float_val = om->hit_chance;
				break;
			case Expression_Variable_Object::description:
				v->type = Value::Value_Type::String;
				v->string_val = om->description;
				break;
			case Expression_Variable_Object::name:
				v->type = Value::Value_Type::String;
				v->string_val = om->name;
				break;
		}
	}
	
	return v;
}



bool Add_Expression::construct(std::vector<Expression*> arguments)
{
	if (arguments.size() < 2)
	{
		return false;
	}
	
	args = arguments;
	return true;
}

Add_Expression::~Add_Expression()
{
	for (std::size_t i = 0; i < args.size(); ++i)
	{
		delete args[i];
		args[i] = nullptr;
	}
}

Value* Add_Expression::evaluate(GameState& gs, ECS::Handle caller)
{
	Value* l = args[0]->evaluate(gs,caller);
	
	if (l->type == Value::Value_Type::Int || l->type == Value::Value_Type::Float)
	{
		for (std::size_t i = 1; i < args.size(); ++i)
		{
			Value* r = args[i]->evaluate(gs,caller);
			
			if (l->type == Value::Value_Type::Int)
			{
				if (r->type == Value::Value_Type::Int)
				{
					l->int_val += r->int_val;
				}
				else if (r->type == Value::Value_Type::Float)
				{
					l->type = Value::Value_Type::Float;
					l->float_val = (float)l->int_val + r->float_val;
				}
				else
				{
					#ifdef DEBUG
						Log::write("ERROR: In + function the first type was an integer but a later argument was not a number.");
					#endif
				
					l->type = Value::Value_Type::Error;
					delete r;
					break;
				}
			}
			else if (l->type == Value::Value_Type::Float)
			{
				if (r->type == Value::Value_Type::Int)
				{
					l->float_val += (float)r->int_val;
				}
				else if (r->type == Value::Value_Type::Float)
				{
					l->float_val += r->float_val;
				}
				else
				{
					#ifdef DEBUG
						Log::write("ERROR: In + function the first type was a floating point but a later argument was not a number.");
					#endif
				
					l->type = Value::Value_Type::Error;
					delete r;
					break;
				}
			}
			
			delete r;
		}
		
		return l;
	}
	else if (l->type == Value::Value_Type::String)
	{
		for (std::size_t i = 1; i < args.size(); ++i)
		{
			Value* r = args[i]->evaluate(gs,caller);
			
			if (r->type == Value::Value_Type::Int)
			{
				l->string_val += StringUtils::to_string(r->int_val);
			}
			else if (r->type == Value::Value_Type::Float)
			{
				l->string_val += StringUtils::to_string(r->float_val);
			}
			else if (r->type == Value::Value_Type::String)
			{
				l->string_val += r->string_val;
			}
			else if (r->type == Value::Value_Type::Bool)
			{
				l->string_val += r->bool_val ? "true" : "false";
			}
			#ifdef DEBUG
			else if (r->type == Value::Value_Type::Error)
			{
				l->string_val += "ERROR EVALUATING ARGUMENT";
			}
			#endif
			else
			{
				#ifdef DEBUG
					Log::write("ERROR: Could not concatenate arguments in + expression.");
				#endif
			
				l->type = Value::Value_Type::Error;
				delete r;
				break;
			}
			
			delete r;
		}
		
		return l;
	}
	else
	{
		#ifdef DEBUG
			Log::write("ERROR: + function must start with an expression that evaluates to an integer, floating point, or string.");
		#endif
	
		l->type = Value::Value_Type::Error;
		return l;
	}
}

bool Subtract_Expression::construct(std::vector<Expression*> arguments)
{
	if (arguments.size() != 2)
	{
		return false;
	}
	
	lhs = arguments[0];
	rhs = arguments[1];
	return true;
}

Subtract_Expression::~Subtract_Expression()
{
	delete lhs;
	delete rhs;
	lhs = nullptr;
	rhs = nullptr;
}

Value* Subtract_Expression::evaluate(GameState& gs, ECS::Handle caller)
{
	Value* l = lhs->evaluate(gs,caller);
	Value* r = rhs->evaluate(gs,caller);
	if ((l->type != Value::Value_Type::Int && l->type != Value::Value_Type::Float) || (r->type != Value::Value_Type::Int && r->type != Value::Value_Type::Float))
	{
		#ifdef DEBUG
			Log::write("ERROR: One or both arguments in - function evaluate to non-numbers.");
		#endif
	
		delete l;
		r->type = Value::Value_Type::Error;
		return r;
	}
	
	if (l->type == Value::Value_Type::Float && r->type == Value::Value_Type::Float)
	{
		l->float_val -= r->float_val;
		delete r;
		return l;
	}
	else if (l->type == Value::Value_Type::Int && r->type == Value::Value_Type::Int)
	{
		l->int_val -= r->int_val;
		delete r;
		return l;
	}
	else if (l->type == Value::Value_Type::Float && r->type == Value::Value_Type::Int)
	{
		l->float_val -= (float)r->int_val;
		delete r;
		return l;
	}
	else
	{
		r->float_val = (float)l->int_val - r->float_val;
		delete l;
		return r;
	}
}

bool Multiply_Expression::construct(std::vector<Expression*> arguments)
{
	if (arguments.size() != 2)
	{
		return false;
	}
	
	lhs = arguments[0];
	rhs = arguments[1];
	return true;
}

Multiply_Expression::~Multiply_Expression()
{
	delete lhs;
	delete rhs;
	lhs = nullptr;
	rhs = nullptr;
}

Value* Multiply_Expression::evaluate(GameState& gs, ECS::Handle caller)
{
	Value* l = lhs->evaluate(gs,caller);
	Value* r = rhs->evaluate(gs,caller);
	if ((l->type != Value::Value_Type::Int && l->type != Value::Value_Type::Float) || (r->type != Value::Value_Type::Int && r->type != Value::Value_Type::Float))
	{
		#ifdef DEBUG
			Log::write("Error: One or both arguments in * function evaluate to non-numbers.");
		#endif
	
		delete l;
		r->type = Value::Value_Type::Error;
		return r;
	}
	
	if (l->type == Value::Value_Type::Float && r->type == Value::Value_Type::Float)
	{
		l->float_val *= r->float_val;
		delete r;
		return l;
	}
	else if (l->type == Value::Value_Type::Int && r->type == Value::Value_Type::Int)
	{
		l->int_val *= r->int_val;
		delete r;
		return l;
	}
	else if (l->type == Value::Value_Type::Float && r->type == Value::Value_Type::Int)
	{
		l->float_val *= (float)r->int_val;
		delete r;
		return l;
	}
	else
	{
		r->float_val *= (float)l->int_val;
		delete l;
		return r;
	}
}

bool Divide_Expression::construct(std::vector<Expression*> arguments)
{
	if (arguments.size() != 2)
	{
		return false;
	}
	
	lhs = arguments[0];
	rhs = arguments[1];
	return true;
}

Divide_Expression::~Divide_Expression()
{
	delete lhs;
	delete rhs;
	lhs = nullptr;
	rhs = nullptr;
}

Value* Divide_Expression::evaluate(GameState& gs, ECS::Handle caller)
{
	Value* l = lhs->evaluate(gs,caller);
	Value* r = rhs->evaluate(gs,caller);
	if ((l->type != Value::Value_Type::Int && l->type != Value::Value_Type::Float) || (r->type != Value::Value_Type::Int && r->type != Value::Value_Type::Float) || (r->type == Value::Value_Type::Int && r->int_val == 0) || (r->type == Value::Value_Type::Float && r->float_val == 0.0f))
	{
		#ifdef DEBUG
			Log::write("ERROR: One or both arguments in / function evaluate to non-numbers.");
		#endif
	
		delete l;
		r->type = Value::Value_Type::Error;
		return r;
	}
	
	if (l->type == Value::Value_Type::Float && r->type == Value::Value_Type::Float)
	{
		l->float_val /= r->float_val;
		delete r;
		return l;
	}
	else if (l->type == Value::Value_Type::Int && r->type == Value::Value_Type::Int)
	{
		l->int_val /= r->int_val;
		delete r;
		return l;
	}
	else if (l->type == Value::Value_Type::Float && r->type == Value::Value_Type::Int)
	{
		l->float_val /= (float)r->int_val;
		delete r;
		return l;
	}
	else
	{
		r->float_val = (float)l->int_val / r->float_val;
		delete l;
		return r;
	}
}

bool Power_Expression::construct(std::vector<Expression*> arguments)
{
	if (arguments.size() != 2)
	{
		return false;
	}
	
	lhs = arguments[0];
	rhs = arguments[1];
	return true;
}

Power_Expression::~Power_Expression()
{
	delete lhs;
	delete rhs;
	lhs = nullptr;
	rhs = nullptr;
}

Value* Power_Expression::evaluate(GameState& gs, ECS::Handle caller)
{
	Value* l = lhs->evaluate(gs,caller);
	Value* r = rhs->evaluate(gs,caller);
	if ((l->type != Value::Value_Type::Int && l->type != Value::Value_Type::Float) || (r->type != Value::Value_Type::Int && r->type != Value::Value_Type::Float))
	{
		#ifdef DEBUG
			Log::write("Error: One or both arguments in ^ function evaluate to non-numbers.");
		#endif
	
		delete l;
		r->type = Value::Value_Type::Error;
		return r;
	}
	
	if (l->type == Value::Value_Type::Float && r->type == Value::Value_Type::Float)
	{
		l->float_val = pow(l->float_val,r->float_val);
		delete r;
		return l;
	}
	else if (l->type == Value::Value_Type::Int && r->type == Value::Value_Type::Int)
	{
		l->int_val = (int)pow((double)l->int_val,(double)r->int_val);
		delete r;
		return l;
	}
	else if (l->type == Value::Value_Type::Float && r->type == Value::Value_Type::Int)
	{
		l->float_val = pow(l->float_val,(float)r->int_val);
		delete r;
		return l;
	}
	else
	{
		r->float_val = pow((float)l->int_val,r->float_val);
		delete l;
		return r;
	}
}



bool Min_Expression::construct(std::vector<Expression*> arguments)
{
	if (arguments.size() < 2)
	{
		return false;
	}
	
	args = arguments;
	return true;
}

Min_Expression::~Min_Expression()
{
	for (unsigned i = 0; i < args.size(); ++i)
	{
		delete args[i];
		args[i] = nullptr;
	}
}

Value* Min_Expression::evaluate(GameState& gs, ECS::Handle caller)
{
	Value* retval = args[0]->evaluate(gs,caller);
	
	if (retval->type == Value::Value_Type::Int || retval->type == Value::Value_Type::Float)
	{
		for (unsigned i = 1; i < args.size(); ++i)
		{
			Value* a = args[i]->evaluate(gs,caller);
			
			if (a->type != Value::Value_Type::Int && a->type != Value::Value_Type::Float)
			{
				#ifdef DEBUG
					Log::write("ERROR: An argument for the min function evaluated to a non-number.");
				#endif
			
				delete a;
				
				retval->type = Value::Value_Type::Error;
				break;
			}
			
			if ((retval->type == Value::Value_Type::Int && a->type == Value::Value_Type::Int && a->int_val < retval->int_val) ||
				(retval->type == Value::Value_Type::Int && a->type == Value::Value_Type::Float && a->float_val < (float)retval->int_val) ||
				(retval->type == Value::Value_Type::Float && a->type == Value::Value_Type::Int && (float)a->int_val < retval->int_val) ||
				(retval->type == Value::Value_Type::Float && a->type == Value::Value_Type::Float && a->float_val < retval->float_val))
			{
				delete retval;
				retval = a;
			}
			else
			{
				delete a;
			}
		}
	}
	
	return retval;
}



bool Max_Expression::construct(std::vector<Expression*> arguments)
{
	if (arguments.size() < 2)
	{
		return false;
	}
	
	args = arguments;
	return true;
}

Max_Expression::~Max_Expression()
{
	for (unsigned i = 0; i < args.size(); ++i)
	{
		delete args[i];
		args[i] = nullptr;
	}
}

Value* Max_Expression::evaluate(GameState& gs, ECS::Handle caller)
{
	Value* retval = args[0]->evaluate(gs,caller);
	
	if (retval->type == Value::Value_Type::Int || retval->type == Value::Value_Type::Float)
	{
		for (unsigned i = 1; i < args.size(); ++i)
		{
			Value* a = args[i]->evaluate(gs,caller);
			
			if (a->type != Value::Value_Type::Int && a->type != Value::Value_Type::Float)
			{
				#ifdef DEBUG
					Log::write("ERROR: An argument for the max function evaluated to a non-number.");
				#endif
			
				delete a;
				
				retval->type = Value::Value_Type::Error;
				break;
			}
			
			if ((retval->type == Value::Value_Type::Int && a->type == Value::Value_Type::Int && a->int_val > retval->int_val) ||
				(retval->type == Value::Value_Type::Int && a->type == Value::Value_Type::Float && a->float_val > (float)retval->int_val) ||
				(retval->type == Value::Value_Type::Float && a->type == Value::Value_Type::Int && (float)a->int_val > retval->int_val) ||
				(retval->type == Value::Value_Type::Float && a->type == Value::Value_Type::Float && a->float_val > retval->float_val))
			{
				delete retval;
				retval = a;
			}
			else
			{
				delete a;
			}
		}
	}
	else
	{
		retval->type = Value::Value_Type::Error;
	}

	return retval;
}



bool Say_Expression::construct(std::vector<Expression*> arguments)
{
	if (arguments.size() > 1)
	{
		return false;
	}
	
	arg = arguments[0];
	return true;
}

Say_Expression::~Say_Expression()
{
	delete arg;
	arg = nullptr;
}

Value* Say_Expression::evaluate(GameState& gs, ECS::Handle caller)
{
	Value* v = arg->evaluate(gs,caller);

	if (v->type == Value::Value_Type::String)
	{
		std::string s = "says";
		if (v->string_val.back() == '!')
		{
			s = "exclaims";
		}
		else if (v->string_val.back() == '?')
		{
			s = "asks";
		}
		
		Object* o = gs.level->get_object(caller);
		std::string name = o->name;
		std::string c = o->friendly ? "<fg=green>" : "<fg=red>";
		
		gs.main_text += "\n\n<fg=white>\"" + v->string_val + "\" " + s + " " + c + name + "<fg=white>.";
		gs.main_text_dirty_flag = true;
	}
	else
	{
		#ifdef DEBUG
			Log::write("ERROR:Invalid type for Say expression. Nothing changed.");
		#endif
		
		v->type = Value::Value_Type::Error;
	}

	return v;
}



bool If_Expression::construct(std::vector<Expression*> arguments)
{
	if (arguments.size() != 3 && arguments.size() != 2)
	{
		return false;
	}

	condition = arguments[0];
	if_true = arguments[1];
	
	if (arguments.size() == 2)
		if_false = nullptr;
	else
		if_false = arguments[2];
	
	return true;
}

If_Expression::~If_Expression()
{
	delete condition;
	delete if_true;
	if (if_false != nullptr)
	{
		delete if_false;
		if_false = nullptr;
	}
	
	condition = nullptr;
	if_true = nullptr;
}

Value* If_Expression::evaluate(GameState& gs, ECS::Handle caller)
{
	Value* c = condition->evaluate(gs,caller);
	
	if (c->type != Value::Value_Type::Bool)
	{
		#ifdef DEBUG
			Log::write("ERROR: The condition expression in if function evaluated to a non-bool type.");
		#endif
	
		c->type = Value::Value_Type::Error;
		return c;
	}
	
	if (c->bool_val)
	{
		delete c;
		return if_true->evaluate(gs,caller);
	}
	else if (if_false != nullptr)
	{
		delete c;
		return if_false->evaluate(gs,caller);
	}
	
	c->type = Value::Value_Type::Bool;
	c->bool_val = true;
	return c;
}



bool And_Expression::construct(std::vector<Expression*> arguments)
{
	if (arguments.size() < 2)
	{
		return false;
	}

	args = arguments;
	return true;
}

And_Expression::~And_Expression()
{
	for (unsigned i = 0; i < args.size(); ++i)
	{
		delete args[i];
		args[i] = nullptr;
	}
}

Value* And_Expression::evaluate(GameState& gs, ECS::Handle caller)
{
	for (unsigned i = 0; i < args.size(); ++i)
	{
		Value* v = args[i]->evaluate(gs,caller);
		
		if (v->type != Value::Value_Type::Bool)
		{
			#ifdef DEBUG
				Log::write("ERROR: an argument for the And function evaluated to a non-bool type.");
			#endif
		
			v->type = Value::Value_Type::Error;
			return v;
		}
		
		if (v->bool_val == false)
		{
			return v;
		}
		
		delete v;
	}
	
	Value* v = new Value;
	v->type = Value::Value_Type::Bool;
	v->bool_val = true;
	return v;
}



bool Not_Expression::construct(std::vector<Expression*> arguments)
{
	if (arguments.size() != 1)
	{
		return false;
	}

	arg = arguments[0];
	return true;
}

Not_Expression::~Not_Expression()
{
	delete arg;
	arg = nullptr;
}

Value* Not_Expression::evaluate(GameState& gs, ECS::Handle caller)
{
	Value* v = arg->evaluate(gs,caller);
	
	if (v->type != Value::Value_Type::Bool)
	{
		#ifdef DEBUG
			Log::write("ERROR: the argument for the Not function evaluated to a non-bool type.");
		#endif
	
		v->type = Value::Value_Type::Error;
	}
	else
	{
		v->bool_val = !(v->bool_val);
	}
	
	return v;
}



bool Or_Expression::construct(std::vector<Expression*> arguments)
{
	if (arguments.size() < 2)
	{
		return false;
	}

	args = arguments;
	return true;
}

Or_Expression::~Or_Expression()
{
	for (unsigned i = 0; i < args.size(); ++i)
	{
		delete args[i];
		args[i] = nullptr;
	}
}

Value* Or_Expression::evaluate(GameState& gs, ECS::Handle caller)
{
	for (unsigned i = 0; i < args.size(); ++i)
	{
		Value* v = args[i]->evaluate(gs,caller);
		
		if (v->type != Value::Value_Type::Bool)
		{
			#ifdef DEBUG
				Log::write("ERROR: an argument for the Or function evaluated to a non-bool type.");
			#endif
		
			v->type = Value::Value_Type::Error;
			return v;
		}
		
		if (v->bool_val == true)
		{
			return v;
		}
		
		delete v;
	}
	
	Value* v = new Value;
	v->type = Value::Value_Type::Bool;
	v->bool_val = false;
	return v;
}



bool Xor_Expression::construct(std::vector<Expression*> arguments)
{
	if (arguments.size() != 2)
	{
		return false;
	}

	lhs = arguments[0];
	rhs = arguments[1];
	
	return true;
}

Xor_Expression::~Xor_Expression()
{
	delete lhs;
	delete rhs;
	lhs = nullptr;
	rhs = nullptr;
}

Value* Xor_Expression::evaluate(GameState& gs, ECS::Handle caller)
{
	Value* l = lhs->evaluate(gs,caller);
	Value* r = rhs->evaluate(gs,caller);
	
	if (l->type != r->type || l->type != Value::Value_Type::Bool)
	{
		#ifdef DEBUG
			Log::write("ERROR: an argument for the Xor function evaluated to a non-bool type.");
		#endif
	
		delete r;
		l->type = Value::Value_Type::Error;
		return l;
	}
	
	bool retval = l->bool_val != r->bool_val;
	delete r;
	l->bool_val = retval;
	return l;
}




bool LessThan_Expression::construct(std::vector<Expression*> arguments)
{
	if (arguments.size() != 2)
	{
		return false;
	}

	lhs = arguments[0];
	rhs = arguments[1];
	return true;
}

LessThan_Expression::~LessThan_Expression()
{
	delete lhs;
	delete rhs;
	lhs = nullptr;
	rhs = nullptr;
}

Value* LessThan_Expression::evaluate(GameState& gs, ECS::Handle caller)
{
	Value* l = lhs->evaluate(gs,caller);
	Value* r = rhs->evaluate(gs,caller);
	
	if (l->type == Value::Value_Type::Int && r->type == Value::Value_Type::Int)
	{
		bool retval = l->int_val < r->int_val;
		
		delete r;
		l->type = Value::Value_Type::Bool;
		l->bool_val = retval;
		return l;
	}
	else if (l->type == Value::Value_Type::Int && r->type == Value::Value_Type::Float)
	{
		bool retval = (float)l->int_val < r->float_val;
		
		delete r;
		l->type = Value::Value_Type::Bool;
		l->bool_val = retval;
		return l;
	}
	else if (l->type == Value::Value_Type::Float && r->type == Value::Value_Type::Int)
	{
		bool retval = l->float_val < (float)r->int_val;
		
		delete r;
		l->type = Value::Value_Type::Bool;
		l->bool_val = retval;
		return l;
	}
	else if (l->type == Value::Value_Type::Float && r->type == Value::Value_Type::Float)
	{
		bool retval = l->float_val < r->float_val;
		
		delete r;
		l->type = Value::Value_Type::Bool;
		l->bool_val = retval;
		return l;
	}
	
	#ifdef DEBUG
		Log::write("ERROR: One or both arguments for < expression evaluated to non-numbers.");
	#endif
	
	delete r;
	l->type = Value::Value_Type::Error;
	return l;
}




bool GreaterThan_Expression::construct(std::vector<Expression*> arguments)
{
	if (arguments.size() != 2)
	{
		return false;
	}

	lhs = arguments[0];
	rhs = arguments[1];
	return true;
}

GreaterThan_Expression::~GreaterThan_Expression()
{
	delete lhs;
	delete rhs;
	lhs = nullptr;
	rhs = nullptr;
}

Value* GreaterThan_Expression::evaluate(GameState& gs, ECS::Handle caller)
{
	Value* l = lhs->evaluate(gs,caller);
	Value* r = rhs->evaluate(gs,caller);
	
	if (l->type == Value::Value_Type::Int && r->type == Value::Value_Type::Int)
	{
		bool retval = l->int_val > r->int_val;
		
		delete r;
		l->type = Value::Value_Type::Bool;
		l->bool_val = retval;
		return l;
	}
	else if (l->type == Value::Value_Type::Int && r->type == Value::Value_Type::Float)
	{
		bool retval = (float)l->int_val > r->float_val;
		
		delete r;
		l->type = Value::Value_Type::Bool;
		l->bool_val = retval;
		return l;
	}
	else if (l->type == Value::Value_Type::Float && r->type == Value::Value_Type::Int)
	{
		bool retval = l->float_val > (float)r->int_val;
		
		delete r;
		l->type = Value::Value_Type::Bool;
		l->bool_val = retval;
		return l;
	}
	else if (l->type == Value::Value_Type::Float && r->type == Value::Value_Type::Float)
	{
		bool retval = l->float_val > r->float_val;
		
		delete r;
		l->type = Value::Value_Type::Bool;
		l->bool_val = retval;
		return l;
	}
	
	#ifdef DEBUG
		Log::write("ERROR: One or both arguments for > expression evaluated to non-numbers.");
	#endif
	
	delete r;
	l->type = Value::Value_Type::Error;
	return l;
}




bool LessThanEqual_Expression::construct(std::vector<Expression*> arguments)
{
	if (arguments.size() != 2)
	{
		return false;
	}

	lhs = arguments[0];
	rhs = arguments[1];
	return true;
}

LessThanEqual_Expression::~LessThanEqual_Expression()
{
	delete lhs;
	delete rhs;
	lhs = nullptr;
	rhs = nullptr;
}

Value* LessThanEqual_Expression::evaluate(GameState& gs, ECS::Handle caller)
{
	Value* l = lhs->evaluate(gs,caller);
	Value* r = rhs->evaluate(gs,caller);
	
	if (l->type == Value::Value_Type::Int && r->type == Value::Value_Type::Int)
	{
		bool retval = l->int_val <= r->int_val;
		
		delete r;
		l->type = Value::Value_Type::Bool;
		l->bool_val = retval;
		return l;
	}
	else if (l->type == Value::Value_Type::Int && r->type == Value::Value_Type::Float)
	{
		bool retval = (float)l->int_val <= r->float_val;
		
		delete r;
		l->type = Value::Value_Type::Bool;
		l->bool_val = retval;
		return l;
	}
	else if (l->type == Value::Value_Type::Float && r->type == Value::Value_Type::Int)
	{
		bool retval = l->float_val <= (float)r->int_val;
		
		delete r;
		l->type = Value::Value_Type::Bool;
		l->bool_val = retval;
		return l;
	}
	else if (l->type == Value::Value_Type::Float && r->type == Value::Value_Type::Float)
	{
		bool retval = l->float_val <= r->float_val;
		
		delete r;
		l->type = Value::Value_Type::Bool;
		l->bool_val = retval;
		return l;
	}
	
	#ifdef DEBUG
		Log::write("ERROR: One or both arguments for <= expression evaluated to non-numbers.");
	#endif
	
	delete r;
	l->type = Value::Value_Type::Error;
	return l;
}




bool GreaterThanEqual_Expression::construct(std::vector<Expression*> arguments)
{
	if (arguments.size() != 2)
	{
		return false;
	}

	lhs = arguments[0];
	rhs = arguments[1];
	return true;
}

GreaterThanEqual_Expression::~GreaterThanEqual_Expression()
{
	delete lhs;
	delete rhs;
	lhs = nullptr;
	rhs = nullptr;
}

Value* GreaterThanEqual_Expression::evaluate(GameState& gs, ECS::Handle caller)
{
	Value* l = lhs->evaluate(gs,caller);
	Value* r = rhs->evaluate(gs,caller);
	
	if (l->type == Value::Value_Type::Int && r->type == Value::Value_Type::Int)
	{
		bool retval = l->int_val >= r->int_val;
		
		delete r;
		l->type = Value::Value_Type::Bool;
		l->bool_val = retval;
		return l;
	}
	else if (l->type == Value::Value_Type::Int && r->type == Value::Value_Type::Float)
	{
		bool retval = (float)l->int_val >= r->float_val;
		
		delete r;
		l->type = Value::Value_Type::Bool;
		l->bool_val = retval;
		return l;
	}
	else if (l->type == Value::Value_Type::Float && r->type == Value::Value_Type::Int)
	{
		bool retval = l->float_val >= (float)r->int_val;
		
		delete r;
		l->type = Value::Value_Type::Bool;
		l->bool_val = retval;
		return l;
	}
	else if (l->type == Value::Value_Type::Float && r->type == Value::Value_Type::Float)
	{
		bool retval = l->float_val >= r->float_val;
		
		delete r;
		l->type = Value::Value_Type::Bool;
		l->bool_val = retval;
		return l;
	}
	
	#ifdef DEBUG
		Log::write("ERROR: One or both arguments for >= expression evaluated to non-numbers.");
	#endif
	
	delete r;
	l->type = Value::Value_Type::Error;
	return l;
}




bool Equal_Expression::construct(std::vector<Expression*> arguments)
{
	if (arguments.size() != 2)
	{
		return false;
	}

	lhs = arguments[0];
	rhs = arguments[1];
	return true;
}

Equal_Expression::~Equal_Expression()
{
	delete lhs;
	delete rhs;
	lhs = nullptr;
	rhs = nullptr;
}

Value* Equal_Expression::evaluate(GameState& gs, ECS::Handle caller)
{
	Value* l = lhs->evaluate(gs,caller);
	Value* r = rhs->evaluate(gs,caller);
	
	if (l->type == Value::Value_Type::Int && r->type == Value::Value_Type::Int)
	{
		bool retval = l->int_val == r->int_val;
		
		delete r;
		l->type = Value::Value_Type::Bool;
		l->bool_val = retval;
		return l;
	}
	else if (l->type == Value::Value_Type::Int && r->type == Value::Value_Type::Float)
	{
		bool retval = (float)l->int_val == r->float_val;
		
		delete r;
		l->type = Value::Value_Type::Bool;
		l->bool_val = retval;
		return l;
	}
	else if (l->type == Value::Value_Type::Float && r->type == Value::Value_Type::Int)
	{
		bool retval = l->float_val == (float)r->int_val;
		
		delete r;
		l->type = Value::Value_Type::Bool;
		l->bool_val = retval;
		return l;
	}
	else if (l->type == Value::Value_Type::Float && r->type == Value::Value_Type::Float)
	{
		bool retval = l->float_val == r->float_val;
		
		delete r;
		l->type = Value::Value_Type::Bool;
		l->bool_val = retval;
		return l;
	}
	else if (l->type == r->type && l->type == Value::Value_Type::String)
	{
		bool retval = l->string_val == r->string_val;
		
		delete r;
		l->type = Value::Value_Type::Bool;
		l->bool_val = retval;
		return l;
	}
	else if (l->type == r->type && l->type == Value::Value_Type::Bool)
	{
		bool retval = l->bool_val == r->bool_val;
		
		delete r;
		l->type = Value::Value_Type::Bool;
		l->bool_val = retval;
		return l;
	}
	
	#ifdef DEBUG
		Log::write("ERROR: Type mismatch between evaluated arguments of = function.");
	#endif
	
	delete r;
	l->type = Value::Value_Type::Error;
	return l;
}




bool NotEqual_Expression::construct(std::vector<Expression*> arguments)
{
	if (arguments.size() != 2)
	{
		return false;
	}

	lhs = arguments[0];
	rhs = arguments[1];
	return true;
}

NotEqual_Expression::~NotEqual_Expression()
{
	delete lhs;
	delete rhs;
	lhs = nullptr;
	rhs = nullptr;
}

Value* NotEqual_Expression::evaluate(GameState& gs, ECS::Handle caller)
{
	Value* l = lhs->evaluate(gs,caller);
	Value* r = rhs->evaluate(gs,caller);
	
	if (l->type == Value::Value_Type::Int && r->type == Value::Value_Type::Int)
	{
		bool retval = l->int_val != r->int_val;
		
		delete r;
		l->type = Value::Value_Type::Bool;
		l->bool_val = retval;
		return l;
	}
	else if (l->type == Value::Value_Type::Int && r->type == Value::Value_Type::Float)
	{
		bool retval = (float)l->int_val != r->float_val;
		
		delete r;
		l->type = Value::Value_Type::Bool;
		l->bool_val = retval;
		return l;
	}
	else if (l->type == Value::Value_Type::Float && r->type == Value::Value_Type::Int)
	{
		bool retval = l->float_val != (float)r->int_val;
		
		delete r;
		l->type = Value::Value_Type::Bool;
		l->bool_val = retval;
		return l;
	}
	else if (l->type == Value::Value_Type::Float && r->type == Value::Value_Type::Float)
	{
		bool retval = l->float_val != r->float_val;
		
		delete r;
		l->type = Value::Value_Type::Bool;
		l->bool_val = retval;
		return l;
	}
	else if (l->type == r->type && l->type == Value::Value_Type::String)
	{
		bool retval = l->string_val != r->string_val;
		
		delete r;
		l->type = Value::Value_Type::Bool;
		l->bool_val = retval;
		return l;
	}
	else if (l->type == r->type && l->type == Value::Value_Type::Bool)
	{
		bool retval = l->bool_val != r->bool_val;
		
		delete r;
		l->type = Value::Value_Type::Bool;
		l->bool_val = retval;
		return l;
	}
	
	#ifdef DEBUG
		Log::write("ERROR: Type mismatch between evaluated arguments of != function.");
	#endif
	
	delete r;
	l->type = Value::Value_Type::Error;
	return l;
}




bool Between_Expression::construct(std::vector<Expression*> arguments)
{
	if (arguments.size() != 3)
	{
		return false;
	}

	val = arguments[0];
	lower_limit = arguments[1];
	upper_limit = arguments[2];
	
	return true;
}

Between_Expression::~Between_Expression()
{
	delete val;
	delete lower_limit;
	delete upper_limit;
	
	val = nullptr;
	lower_limit = nullptr;
	upper_limit = nullptr;
}

Value* Between_Expression::evaluate(GameState& gs, ECS::Handle caller)
{
	Value* v = val->evaluate(gs,caller);
	Value* l = lower_limit->evaluate(gs,caller);
	Value* r = upper_limit->evaluate(gs,caller);
	
	if (l->type == Value::Value_Type::Int && r->type == Value::Value_Type::Int && v->type == Value::Value_Type::Int)
	{
		bool retval = v->int_val >= l->int_val && v->int_val <= r->int_val;
		
		delete r;
		delete v;
		l->type = Value::Value_Type::Bool;
		l->bool_val = retval;
		return l;
	}
	
	float vf, lf, rf;
	
	if (v->type == Value::Value_Type::Int)
	{
		vf = (float)v->int_val;
	}
	else if (v->type == Value::Value_Type::Float)
	{
		vf = v->float_val;
	}
	else
	{
		#ifdef DEBUG
			Log::write("ERROR: first argument in between function evaluated to non-number.");
		#endif
	
		delete l;
		delete r;
		v->type = Value::Value_Type::Error;
		
		return v;
	}
	
	if (l->type == Value::Value_Type::Int)
	{
		lf = (float)l->int_val;
	}
	else if (l->type == Value::Value_Type::Float)
	{
		lf = l->float_val;
	}
	else
	{
		#ifdef DEBUG
			Log::write("ERROR: second argument in between function evaluated to non-number.");
		#endif
	
		delete l;
		delete r;
		v->type = Value::Value_Type::Error;
		
		return v;
	}
	
	if (r->type == Value::Value_Type::Int)
	{
		rf = (float)r->int_val;
	}
	else if (r->type == Value::Value_Type::Float)
	{
		rf = r->float_val;
	}
	else
	{
		#ifdef DEBUG
			Log::write("ERROR: third argument in between function evaluated to non-number.");
		#endif
	
		delete l;
		delete r;
		v->type = Value::Value_Type::Error;
		
		return v;
	}
	
	delete l;
	delete r;
	v->type = Value::Value_Type::Bool;
	v->bool_val = vf >= lf && vf <= rf;
	return v;
}




FEOIR_Expression::FEOIR_Expression(Value* iter_reg)
{
	reg = iter_reg;
}

bool FEOIR_Expression::construct(std::vector<Expression*> arguments)
{
	if (arguments.size() < 1)
	{
		return false;
	}

	args = arguments;
	return true;
}

FEOIR_Expression::~FEOIR_Expression()
{
	delete reg;
	reg = nullptr;

	for (unsigned i = 0; i < args.size(); ++i)
	{
		delete args[i];
		args[i] = nullptr;
	}
}

Value* FEOIR_Expression::evaluate(GameState& gs, ECS::Handle caller)
{
	Value* v;
	Room* r = gs.level->get_room(gs.level->get_object(gs.playable_character)->room_container);
	
	for (std::size_t i = 0; i < r->objects().size(); ++i)
	{
		//fill in the iterator
		reg->int_val = (int)r->objects()[i];
		
		//for each expression in the list...
		for (std::size_t j = 0; j < args.size(); ++j)
		{
			v = args[j]->evaluate(gs,caller);
			
			#ifdef DEBUG
				if (v->type == Value::Value_Type::Error)
				{
					Log::write("Warning: Expression [" + StringUtils::to_string((int)j) + "] in FEOIR function returned an error for object " + StringUtils::to_string((int)r->objects()[i]) + ".");
				}
			#endif
		}
	}
	
	v->type = Value::Value_Type::Bool;
	v->bool_val = true;
	return v;
}



bool Attack_Expression::construct(std::vector<Expression*> arguments)
{
	if (arguments.size() != 4)
	{
		#ifdef DEBUG
			Log::write("ERROR: incorrect number of arguments for Attack function.");
		#endif
	
		return false;
	}
	
	left = arguments[0];
	right = arguments[1];
	front = arguments[2];
	far_front = arguments[3];
	
	return true;
}

Attack_Expression::~Attack_Expression()
{
	delete left;
	delete right;
	delete front;
	delete far_front;
	left = nullptr;
	right = nullptr;
	front = nullptr;
	far_front = nullptr;
}

Value* Attack_Expression::evaluate(GameState& gs, ECS::Handle caller)
{
	Value* l = left->evaluate(gs,caller);
	Value* r = right->evaluate(gs,caller);
	Value* f = front->evaluate(gs,caller);
	Value* ff = far_front->evaluate(gs,caller);
	
	if (l->type != r->type || l->type != f->type || l->type != ff->type || l->type != Value::Value_Type::Bool || gs.combat_data == nullptr)
	{
		#ifdef DEBUG
			if (gs.combat_data == nullptr)
			{
				Log::write("ERROR: Attack function called outside of combat.");
			}
			else
			{
				Log::write("ERROR: one or more arguments for the Attack function evaluated to non-bool type.");
			}
		#endif
		
		delete r;
		delete f;
		delete ff;
		
		l->type = Value::Value_Type::Error;
		
		return l;
	}
	
	gs.combat_data->enemy_attacking_sides[(int)CombatData::Position::left] = l->bool_val;
	gs.combat_data->enemy_attacking_sides[(int)CombatData::Position::right] = r->bool_val;
	gs.combat_data->enemy_attacking_sides[(int)CombatData::Position::front] = f->bool_val;
	gs.combat_data->enemy_attacking_sides[(int)CombatData::Position::far_front] = ff->bool_val;

	delete r;
	delete f;
	delete ff;
	
	l->bool_val = true;
	
	return l;
}



bool Defend_Expression::construct(std::vector<Expression*> arguments)
{
	if (arguments.size() != 4)
	{
		#ifdef DEBUG
			Log::write("ERROR: incorrect number of arguments for Defend function");
		#endif
	
		return false;
	}
	
	left = arguments[0];
	right = arguments[1];
	front = arguments[2];
	far_front = arguments[3];
	
	return true;
}

Defend_Expression::~Defend_Expression()
{
	delete left;
	delete right;
	delete front;
	delete far_front;
	left = nullptr;
	right = nullptr;
	front = nullptr;
	far_front = nullptr;
}

Value* Defend_Expression::evaluate(GameState& gs, ECS::Handle caller)
{
	Value* l = left->evaluate(gs,caller);
	Value* r = right->evaluate(gs,caller);
	Value* f = front->evaluate(gs,caller);
	Value* ff = far_front->evaluate(gs,caller);
	
	if (l->type != r->type || l->type != f->type || l->type != ff->type || l->type != Value::Value_Type::Bool || gs.combat_data == nullptr)
	{
		#ifdef DEBUG
			if (gs.combat_data == nullptr)
			{
				Log::write("ERROR: Defend function called outside of combat.");
			}
			else
			{
				Log::write("ERROR: one or more arguments for the Defend function evaluated to non-bool type.");
			}
		#endif
		
		delete r;
		delete f;
		delete ff;
		
		l->type = Value::Value_Type::Error;
		
		return l;
	}
	
	gs.combat_data->enemy_vulnerable_sides[(int)CombatData::Position::left] = !l->bool_val;
	gs.combat_data->enemy_vulnerable_sides[(int)CombatData::Position::right] = !r->bool_val;
	gs.combat_data->enemy_vulnerable_sides[(int)CombatData::Position::front] = !f->bool_val;
	gs.combat_data->enemy_vulnerable_sides[(int)CombatData::Position::far_front] = !ff->bool_val;

	delete r;
	delete f;
	delete ff;
	
	l->bool_val = true;
	
	return l;
}



bool Destroy_Expression::construct(std::vector<Expression*> arguments)
{
	if (arguments.size() != 1)
	{
		#ifdef DEBUG
			Log::write("ERROR: incorrect number of arguments for Destroy function");
		#endif
	
		return false;
	}
	
	target = arguments[0];
	
	return true;
}

Destroy_Expression::~Destroy_Expression()
{
	delete target;
	target = nullptr;
}

Value* Destroy_Expression::evaluate(GameState& gs, ECS::Handle caller)
{
	Value* t = target->evaluate(gs,caller);
	
	if (t->type != Value::Value_Type::Int)
	{
		#ifdef DEBUG
			Log::write("ERROR: The argument for the Destroy function did not evaluate to an integer (handle) type.");
		#endif
		
		t->type = Value::Value_Type::Error;
		
		return t;
	}
	
	gs.level->destroy_object((ECS::Handle)t->int_val, false);//schedule it for cleanup, but don't delete it immediately (since the script of that object may be running right now!
	
	return t;
}



bool Copy_Expression::construct(std::vector<Expression*> arguments)
{
	if (arguments.size() != 1)
	{
		#ifdef DEBUG
			Log::write("ERROR: incorrect number of arguments for Copy function");
		#endif
	
		return false;
	}
	
	target = arguments[0];
	
	return true;
}

Copy_Expression::~Copy_Expression()
{
	delete target;
	target = nullptr;
}

Value* Copy_Expression::evaluate(GameState& gs, ECS::Handle caller)
{
	Value* t = target->evaluate(gs,caller);
	
	if (t->type != Value::Value_Type::Int)
	{
		#ifdef DEBUG
			Log::write("ERROR: The argument for the Copy function did not evaluate to an integer (handle) type.");
		#endif
		
		t->type = Value::Value_Type::Error;
		
		return t;
	}

	Object* oo = gs.level->get_object(t->int_val);
	
	if (oo == nullptr)
	{
		#ifdef DEBUG
			Log::write("ERROR: The Copy function couldn't evaluate because the handle '" + StringUtils::to_string(t->int_val) + "' is not a valid object handle.");
		#endif
		
		t->type = Value::Value_Type::Error;
		return t;
	}
	
		
	ECS::Handle new_obj = gs.level->create_object();
	Object* on = gs.level->get_object(new_obj);

	on->copy(*oo);

	t->int_val = (int)new_obj;
	
	return t;
}
