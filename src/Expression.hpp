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

class If_Expression : public Expression
{
	Expression* condition;
	Expression* if_true;
	Expression* if_false;
public:
	bool construct(std::vector<Expression*> arguments) override;
	Value* evaluate(ScriptingVariables& pv, std::vector<Value*>* registers) override;
	~If_Expression();
};

class And_Expression : public Expression
{
	std::vector<Expression*> args;
public:
	bool construct(std::vector<Expression*> arguments) override;
	Value* evaluate(ScriptingVariables& pv, std::vector<Value*>* registers) override;
	~And_Expression();
};

class Not_Expression : public Expression
{
 	Expression* arg;
public:
 	bool construct(std::vector<Expression*> arguments) override;
 	Value* evaluate(ScriptingVariables& pv, std::vector<Value*>* registers) override;
 	~Not_Expression(); 
};
class Or_Expression : public Expression
{
 	std::vector<Expression*> args;
public:
 	bool construct(std::vector<Expression*> arguments) override;
 	Value* evaluate(ScriptingVariables& pv, std::vector<Value*>* registers) override;
 	~Or_Expression(); 
};
class Xor_Expression : public Expression
{
 	Expression* lhs;
	Expression* rhs;
public:
 	bool construct(std::vector<Expression*> arguments) override;
 	Value* evaluate(ScriptingVariables& pv, std::vector<Value*>* registers) override;
 	~Xor_Expression(); 
};
class LessThan_Expression : public Expression
{
 	Expression* lhs;
	Expression* rhs;
public:
 	bool construct(std::vector<Expression*> arguments) override;
 	Value* evaluate(ScriptingVariables& pv, std::vector<Value*>* registers) override;
 	~LessThan_Expression(); 
};
class GreaterThan_Expression : public Expression
{
 	Expression* lhs;
	Expression* rhs;
public:
 	bool construct(std::vector<Expression*> arguments) override;
 	Value* evaluate(ScriptingVariables& pv, std::vector<Value*>* registers) override;
 	~GreaterThan_Expression(); 
};
class LessThanEqual_Expression : public Expression
{
 	Expression* lhs;
	Expression* rhs;
public:
 	bool construct(std::vector<Expression*> arguments) override;
 	Value* evaluate(ScriptingVariables& pv, std::vector<Value*>* registers) override;
 	~LessThanEqual_Expression(); 
};
class GreaterThanEqual_Expression : public Expression
{
 	Expression* lhs;
	Expression* rhs;
public:
 	bool construct(std::vector<Expression*> arguments) override;
 	Value* evaluate(ScriptingVariables& pv, std::vector<Value*>* registers) override;
 	~GreaterThanEqual_Expression(); 
};
class Equal_Expression : public Expression
{
 	Expression* lhs;
	Expression* rhs;
public:
 	bool construct(std::vector<Expression*> arguments) override;
 	Value* evaluate(ScriptingVariables& pv, std::vector<Value*>* registers) override;
 	~Equal_Expression(); 
};
class NotEqual_Expression : public Expression
{
 	Expression* lhs;
	Expression* rhs;
public:
 	bool construct(std::vector<Expression*> arguments) override;
 	Value* evaluate(ScriptingVariables& pv, std::vector<Value*>* registers) override;
 	~NotEqual_Expression(); 
};
class Between_Expression : public Expression
{
 	Expression* val;
	Expression* lower_limit;
	Expression* upper_limit;
public:
 	bool construct(std::vector<Expression*> arguments) override;
 	Value* evaluate(ScriptingVariables& pv, std::vector<Value*>* registers) override;
 	~Between_Expression(); 
};
class FEOIR_Expression : public Expression
{
 	Expression* expr;
public:
 	bool construct(std::vector<Expression*> arguments) override;
 	Value* evaluate(ScriptingVariables& pv, std::vector<Value*>* registers) override;
 	~FEOIR_Expression(); 
};