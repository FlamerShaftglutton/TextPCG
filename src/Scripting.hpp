#pragma once
#include "Handle.hpp"
#include <vector>
#include <string>

//forward declarations
class Expression;
struct GameState;
struct Value;

//Script stuff
typedef std::vector<Value*> Register;

class Script
{
	Register* registers;
	std::vector<Expression*> expressions;
	std::string raw_script;
	Expression* recursively_resolve(std::vector<std::string>& tokens, std::vector<int>& token_types);
public:
	Script(std::string script, Register* regs);
	~Script();
	std::string to_string();
	void evaluate(GameState& pv, ECS::Handle caller);
};

class ScriptSet
{
	Register* registers;
	
	Script* on_creation_script;
	Script* on_sight_script;
	Script* on_use_script;
	Script* on_attack_step_script;
	
public:
	ScriptSet();
	~ScriptSet();
	void construct(std::string on_creation, std::string on_sight, std::string on_use, std::string on_attack_step);
	
	void execute_on_creation();
	void execute_on_sight(GameState& pv, ECS::Handle caller);
	void execute_on_use(GameState& pv, ECS::Handle caller);
	void execute_on_attack_step(GameState& pv, ECS::Handle caller);
	std::string to_string();
};
