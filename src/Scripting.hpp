#pragma once
#include "Handle.hpp"
#include <vector>
#include <string>
#include "ScriptingVariables.hpp"
#include "Expression.hpp"

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
	void evaluate(ScriptingVariables& pv);
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
	void execute_on_sight(ScriptingVariables& pv);
	void execute_on_use(ScriptingVariables& pv);
	void execute_on_attack_step(ScriptingVariables& pv);
	std::string to_string();
};
