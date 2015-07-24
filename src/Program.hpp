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
	int int_val;
	float float_val;
	std::string string_val;

	enum class Value_Type
	{
		Float,
		Int,
		String
	} type;
	
	virtual bool construct(std::vector<Expression*> arguments) override;
	virtual Value* evaluate(ProgramVariables& pv, std::vector<Value*> registers) override;
};

//now some classes that implement that interface
class Choose_Expression: public Expression
{
	std::vector<Expression*> arguments;
public:
	virtual bool construct(std::vector<Expression*> arguments) override;
	virtual Value* evaluate(ProgramVariables& pv, std::vector<Value*> registers) override;
	~Choose_Expression();
};

class Random_Expression: public Expression
{
	Expression* lower_limit;
	Expression* upper_limit;
public:
	virtual bool construct(std::vector<Expression*> arguments) override;
	virtual Value* evaluate(ProgramVariables& pv, std::vector<Value*> registers) override;
	~Random_Expression();
};

class Set_Expression: public Expression
{
	Value* register_number;
	Expression* argument;
public:
	virtual bool construct(std::vector<Expression*> arguments) override;
	virtual Value* evaluate(ProgramVariables& pv, std::vector<Value*> registers) override;
	~Set_Expression();
};

class Get_Expression: public Expression
{
	Value* register_number;
public:
	virtual bool construct(std::vector<Expression*> arguments) override;
	virtual Value* evaluate(ProgramVariables& pv, std::vector<Value*> registers) override;
	~Get_Expression();
};

class Add_Expression: public Expression
{
	Expression* lhs;
	Expression* rhs;
public:
	virtual bool construct(std::vector<Expression*> arguments) override;
	virtual Value* evaluate(ProgramVariables& pv, std::vector<Value*> registers) override;
	~Add_Expression();
};

class Subtract_Expression: public Expression
{
	Expression* lhs;
	Expression* rhs;
public:
	virtual bool construct(std::vector<Expression*> arguments) override;
	virtual Value* evaluate(ProgramVariables& pv, std::vector<Value*> registers) override;
	~Subtract_Expression();
};

class Multiply_Expression: public Expression
{
	Expression* lhs;
	Expression* rhs;
public:
	virtual bool construct(std::vector<Expression*> arguments) override;
	virtual Value* evaluate(ProgramVariables& pv, std::vector<Value*> registers) override;
	~Multiply_Expression();
};

class Divide_Expression: public Expression
{
	Expression* lhs;
	Expression* rhs;
public:
	virtual bool construct(std::vector<Expression*> arguments) override;
	virtual Value* evaluate(ProgramVariables& pv, std::vector<Value*> registers) override;
	~Divide_Expression();
};

class Power_Expression: public Expression
{
	Expression* lhs;
	Expression* rhs;
public:
	virtual bool construct(std::vector<Expression*> arguments) override;
	virtual Value* evaluate(ProgramVariables& pv, std::vector<Value*> registers) override;
	~Power_Expression();
};

class Program
{
	std::vector<Value*> registers;
	std::vector<Expression*> expressions;
	ECS::Handle handle;
public:
	Program();
	bool construct(std::string s);
	std::string to_string();
};