#pragma once
#include "Handle.hpp"
#include <vector>
#include <string>
#include "ProgramVariables.hpp"

class Value;

//an Expression interface
class Expression
{
public:
	virtual ~Expression() = default;
	virtual bool construct(std::vector<Expression*> arguments) = 0;
	virtual Value* evaluate(ProgramVariables& pv) = 0;
};

//a value class
class Value : public Expression
{
public:
	union
	{
		int int_val;
		float float_val;
		bool bool_val;
	};
	
	std::string string_val;

	enum class Value_Type
	{
		Bool,
		Float,
		Int,
		String,
		Error
	} type;
	
	virtual bool construct(std::vector<Expression*> arguments) override;
	virtual Value* evaluate(ProgramVariables& pv, std::vector<Value*>* registers) override;
	std::string to_string();
};

//now some classes that implement that interface
class Choose_Expression: public Expression
{
	std::vector<Expression*> args;
public:
	virtual bool construct(std::vector<Expression*> arguments) override;
	virtual Value* evaluate(ProgramVariables& pv, std::vector<Value*>* registers) override;
	~Choose_Expression();
};

class Random_Expression: public Expression
{
	Expression* lower_limit;
	Expression* upper_limit;
public:
	virtual bool construct(std::vector<Expression*> arguments) override;
	virtual Value* evaluate(ProgramVariables& pv, std::vector<Value*>* registers) override;
	~Random_Expression();
};

class Set_Expression: public Expression
{
	int register_number;
	Expression* argument;
public:
	virtual bool construct(std::vector<Expression*> arguments) override;
	virtual Value* evaluate(ProgramVariables& pv, std::vector<Value*>* registers) override;
	~Set_Expression();
};

class Get_Expression: public Expression
{
	int register_number;
public:
	virtual bool construct(std::vector<Expression*> arguments) override;
	virtual Value* evaluate(ProgramVariables& pv, std::vector<Value*>* registers) override;
};

class Add_Expression: public Expression
{
	Expression* lhs;
	Expression* rhs;
public:
	virtual bool construct(std::vector<Expression*> arguments) override;
	virtual Value* evaluate(ProgramVariables& pv, std::vector<Value*>* registers) override;
	~Add_Expression();
};

class Subtract_Expression: public Expression
{
	Expression* lhs;
	Expression* rhs;
public:
	virtual bool construct(std::vector<Expression*> arguments) override;
	virtual Value* evaluate(ProgramVariables& pv, std::vector<Value*>* registers) override;
	~Subtract_Expression();
};

class Multiply_Expression: public Expression
{
	Expression* lhs;
	Expression* rhs;
public:
	virtual bool construct(std::vector<Expression*> arguments) override;
	virtual Value* evaluate(ProgramVariables& pv, std::vector<Value*>* registers) override;
	~Multiply_Expression();
};

class Divide_Expression: public Expression
{
	Expression* lhs;
	Expression* rhs;
public:
	virtual bool construct(std::vector<Expression*> arguments) override;
	virtual Value* evaluate(ProgramVariables& pv, std::vector<Value*>* registers) override;
	~Divide_Expression();
};

class Power_Expression: public Expression
{
	Expression* lhs;
	Expression* rhs;
public:
	virtual bool construct(std::vector<Expression*> arguments) override;
	virtual Value* evaluate(ProgramVariables& pv, std::vector<Value*>* registers) override;
	~Power_Expression();
};

class Min_Expression: public Expression
{
	std::vector<Expression*> args;
public:
	virtual bool construct(std::vector<Expression*> arguments) override;
	virtual Value* evaluate(ProgramVariables& pv, std::vector<Value*>* registers) override;
	~Min_Expression();
};

class Max_Expression: public Expression
{
	std::vector<Expression*> args;
public:
	virtual bool construct(std::vector<Expression*> arguments) override;
	virtual Value* evaluate(ProgramVariables& pv, std::vector<Value*>* registers) override;
	~Max_Expression();
};

//Script stuff
typedef std::vector<Value*> Register;

class Script
{
	Register* registers;
	std::vector<Expression*> expressions;
	std::string raw_script;
public:
	Script(std::string script, Register* regs);
	~Script();
	std::string to_string();
	void evaluate(ProgramVariables& pv);
};

class ScriptSet
{
	Register* registers;
	
	Script* on_creation_script;
	Script* on_sight_script;
	Script* on_use_script;
	
public:
	ScriptSet(std::string on_creation, std::string on_sight, std::string on_use);
	~ScriptSet();
	void execute_on_creation(ProgramVariables& pv);
	void execute_on_sight(ProgramVariables& pv);
	void execute_on_use(ProgramVariables& pv);
	std::string to_String();
};