#include "Program.hpp"
#include "Handle.hpp"
#include <vector>
#include <string>
#include "ProgramVariables.hpp"

bool Value::construct(std::vector<Expression*> arguments)
{

}

Value* Value::evaluate(ProgramVariables& pv, std::vector<Value*> registers)
{

}

bool Choose_Expression::construct(std::vector<Expression*> arguments)
{

}

Choose_Expression::~Choose_Expression()
{

}

Value* Choose_Expression::evaluate(ProgramVariables& pv, std::vector<Value*> registers)
{

}

Random_Expression::~Random_Expression()
{

}

bool Random_Expression::construct(std::vector<Expression*> arguments)
{

}

Value* Random_Expression::evaluate(ProgramVariables& pv, std::vector<Value*> registers)
{

}

bool Set_Expression::construct(std::vector<Expression*> arguments)
{

}

Set_Expression::~Set_Expression()
{

}

Value* Set_Expression::evaluate(ProgramVariables& pv, std::vector<Value*> registers)
{

}

bool Get_Expression::construct(std::vector<Expression*> arguments)
{

}

Get_Expression::~Get_Expression()
{

}

Value* Get_Expression::evaluate(ProgramVariables& pv, std::vector<Value*> registers)
{

}

bool Add_Expression::construct(std::vector<Expression*> arguments)
{

}

Add_Expression::~Add_Expression()
{

}

Value* Add_Expression::evaluate(ProgramVariables& pv, std::vector<Value*> registers)
{

}

bool Subtract_Expression::construct(std::vector<Expression*> arguments)
{

}

Subtract_Expression::~Subtract_Expression()
{

}

Value* Subtract_Expression::evaluate(ProgramVariables& pv, std::vector<Value*> registers)
{

}

bool Multiply_Expression::construct(std::vector<Expression*> arguments)
{

}

Multiply_Expression::~Multiply_Expression()
{

}

Value* Multiply_Expression::evaluate(ProgramVariables& pv, std::vector<Value*> registers)
{

}

bool Divide_Expression::construct(std::vector<Expression*> arguments)
{

}

Divide_Expression::~Divide_Expression()
{

}

Value* Divide_Expression::evaluate(ProgramVariables& pv, std::vector<Value*> registers)
{

}

bool Power_Expression::construct(std::vector<Expression*> arguments)
{

}

Power_Expression::~Power_Expression()
{

}

Value* Power_Expression::evaluate(ProgramVariables& pv, std::vector<Value*> registers)
{

}

Program::Program()
{

}

Program::~Program()
{

}

bool Program::construct(std::string s)
{

}

std::string Program::to_string()
{

}
