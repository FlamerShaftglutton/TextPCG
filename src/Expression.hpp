#pragma once
#include <vector>
#include <string>
#include "ScriptingVariables.hpp"

class Value;

//an Expression interface
class Expression
{
public:
	virtual ~Expression() = default;
	virtual bool construct(std::vector<Expression*> arguments) = 0;
	virtual Value* evaluate(ScriptingVariables& pv, std::vector<Value*>* registers) = 0;
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
	
	bool construct(std::vector<Expression*> arguments) override;
	Value* evaluate(ScriptingVariables& pv, std::vector<Value*>* registers) override;
	std::string to_string();
};

//now some classes that implement that interface
class Choose_Expression: public Expression
{
	std::vector<Expression*> args;
public:
	bool construct(std::vector<Expression*> arguments) override;
	Value* evaluate(ScriptingVariables& pv, std::vector<Value*>* registers) override;
	~Choose_Expression();
};

class Random_Expression: public Expression
{
	Expression* lower_limit;
	Expression* upper_limit;
public:
	bool construct(std::vector<Expression*> arguments) override;
	Value* evaluate(ScriptingVariables& pv, std::vector<Value*>* registers) override;
	~Random_Expression();
};

class Set_Expression: public Expression
{
	int register_number;
	Expression* argument;
public:
	Set_Expression(int rn, Expression* arg);
	bool construct(std::vector<Expression*> arguments) override;
	Value* evaluate(ScriptingVariables& pv, std::vector<Value*>* registers) override;
	~Set_Expression();
};

class Get_Expression: public Expression
{
	int register_number;
public:
	Get_Expression(int rn);
	bool construct(std::vector<Expression*> arguments) override;
	Value* evaluate(ScriptingVariables& pv, std::vector<Value*>* registers) override;
};

class Add_Expression: public Expression
{
	std::vector<Expression*> args;
public:
	bool construct(std::vector<Expression*> arguments) override;
	Value* evaluate(ScriptingVariables& pv, std::vector<Value*>* registers) override;
	~Add_Expression();
};

class Subtract_Expression: public Expression
{
	Expression* lhs;
	Expression* rhs;
public:
	bool construct(std::vector<Expression*> arguments) override;
	Value* evaluate(ScriptingVariables& pv, std::vector<Value*>* registers) override;
	~Subtract_Expression();
};

class Multiply_Expression: public Expression
{
	Expression* lhs;
	Expression* rhs;
public:
	bool construct(std::vector<Expression*> arguments) override;
	Value* evaluate(ScriptingVariables& pv, std::vector<Value*>* registers) override;
	~Multiply_Expression();
};

class Divide_Expression: public Expression
{
	Expression* lhs;
	Expression* rhs;
public:
	bool construct(std::vector<Expression*> arguments) override;
	Value* evaluate(ScriptingVariables& pv, std::vector<Value*>* registers) override;
	~Divide_Expression();
};

class Power_Expression: public Expression
{
	Expression* lhs;
	Expression* rhs;
public:
	bool construct(std::vector<Expression*> arguments) override;
	Value* evaluate(ScriptingVariables& pv, std::vector<Value*>* registers) override;
	~Power_Expression();
};

class Min_Expression: public Expression
{
	std::vector<Expression*> args;
public:
	bool construct(std::vector<Expression*> arguments) override;
	Value* evaluate(ScriptingVariables& pv, std::vector<Value*>* registers) override;
	~Min_Expression();
};

class Max_Expression: public Expression
{
	std::vector<Expression*> args;
public:
	bool construct(std::vector<Expression*> arguments) override;
	Value* evaluate(ScriptingVariables& pv, std::vector<Value*>* registers) override;
	~Max_Expression();
};

class Say_Expression : public Expression
{
	Expression* arg;
public:
	bool construct(std::vector<Expression*> arguments) override;
	Value* evaluate(ScriptingVariables& pv, std::vector<Value*>* registers) override;
	~Say_Expression();
};