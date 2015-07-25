#include "Program.hpp"
#include "Handle.hpp"
#include <vector>
#include <string>
#include "ProgramVariables.hpp"
#include "mymath.hpp"
#include <cmath>

Value* copy_value(Value* v)
{
	Value* retval = new Value;

	retval->string_val = string_val;
	
	retval->type = type;
	
	switch (type)
	{
		case Value::Value_Type::Int: retval->int_val = int_val; break;
		case Value::Value_Type::Bool: retval->bool_val = bool_val; break;
		case Value::Value_Type::Float: retval->float_val = float_val; break;
	}
	
	return retval;
}

bool Value::construct(std::vector<Expression*> arguments)
{
	return true;
}

Value* Value::evaluate(ProgramVariables& pv, std::vector<Value*>* registers)
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

Value* Choose_Expression::evaluate(ProgramVariables& pv, std::vector<Value*>* registers)
{
	Value* index = args[0]->evaluate(pv,registers);
	int i = index->int_val;
	
	if (index->type != Value::Value_Type::Int || i > args.size()-2 || i < 0)
	{
		index->type = Value::Value_Type::Error;
		return index;
	}
	
	delete index;
	return args[i]->evaluate(pv,registers);
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
}

Value* Random_Expression::evaluate(ProgramVariables& pv, std::vector<Value*>* registers)
{
	Value* ll = lower_limit->evaluate(pv,registers);
	Value* ul = upper_limit->evaluate(pv,registers);
	
	if (ll->type != ul->type || ll->type != Value::Value_Type::Int)
	{
		delete ll;
		ul->type = Value::Value_Type::Error;
		return ul;
	}
	
	ul->int_val = MyMath::random_int(ll->int_val,ul->int_val);
	delete ll;
	return ul;
}



bool Set_Expression::construct(std::vector<Expression*> arguments)
{
	return true;
}

Set_Expression::~Set_Expression()
{
	delete argument;
}

Value* Set_Expression::evaluate(ProgramVariables& pv, std::vector<Value*>* registers)
{
	//add in the ability to set game variables as well
	Value* v = argument->evaluate(pv,registers);
	(*registers)[register_number] = copy_value(v);
	return v;
}



bool Get_Expression::construct(std::vector<Expression*> arguments)
{
	return true;
}

Value* Get_Expression::evaluate(ProgramVariables& pv, std::vector<Value*>* registers)
{
	//add in the ability to select game variables as well
	return (*registers)[register_number];
}



bool Add_Expression::construct(std::vector<Expression*> arguments)
{
	if (arguments.size() != 2)
	{
		return false;
	}
	
	lhs = arguments[0];
	rhs = arguments[1];
	return true;
}

Add_Expression::~Add_Expression()
{
	delete lhs;
	delete rhs;
}

Value* Add_Expression::evaluate(ProgramVariables& pv, std::vector<Value*>* registers)
{
	Value* l = lhs->evaluate(pv,registers);
	Value* r = rhs->evaluate(pv,registers);
	if ((l->type != Value::Value_Type::Int && l->type != Value::Value_Type::Float) || (r->type != Value::Value_Type::Int && r->type != Value::Value_Type::Float))
	{
		delete l;
		r->type = Value::Value_Type::Error;
		return r;
	}
	
	if (l->type == Value::Value_Type::Float && r->type == Value::Value_Type::Float)
	{
		l->float_val += r->float_val;
		delete r;
		return l;
	}
	else if (l->type == Value::Value_Type::Int && r->type == Value::Value_Type::Int)
	{
		l->int_val += r->int_val;
		delete r;
		return l;
	}
	else if (l->type == Value::Value_Type::Float && r->type == Value::Value_Type::Int)
	{
		l->float_val += (float)r->int_val;
		delete r;
		return l;
	}
	else
	{
		r->float_val += (float)l->int_val;
		delete l;
		return r;
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
}

Value* Subtract_Expression::evaluate(ProgramVariables& pv, std::vector<Value*>* registers)
{
	Value* l = lhs->evaluate(pv,registers);
	Value* r = rhs->evaluate(pv,registers);
	if ((l->type != Value::Value_Type::Int && l->type != Value::Value_Type::Float) || (r->type != Value::Value_Type::Int && r->type != Value::Value_Type::Float))
	{
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
		-r->float_val += (float)l->int_val;
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
}

Value* Multiply_Expression::evaluate(ProgramVariables& pv, std::vector<Value*>* registers)
{
	Value* l = lhs->evaluate(pv,registers);
	Value* r = rhs->evaluate(pv,registers);
	if ((l->type != Value::Value_Type::Int && l->type != Value::Value_Type::Float) || (r->type != Value::Value_Type::Int && r->type != Value::Value_Type::Float))
	{
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
}

Value* Divide_Expression::evaluate(ProgramVariables& pv, std::vector<Value*>* registers)
{
	Value* l = lhs->evaluate(pv,registers);
	Value* r = rhs->evaluate(pv,registers);
	if ((l->type != Value::Value_Type::Int && l->type != Value::Value_Type::Float) || (r->type != Value::Value_Type::Int && r->type != Value::Value_Type::Float) || (r->type == Value::Value_Type::Int && r->int_value == 0) || (r->type == Value::Value_Type::Float && r->float_val == 0.0f))
	{
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
}

Value* Power_Expression::evaluate(ProgramVariables& pv, std::vector<Value*>* registers)
{
	Value* l = lhs->evaluate(pv,registers);
	Value* r = rhs->evaluate(pv,registers);
	if ((l->type != Value::Value_Type::Int && l->type != Value::Value_Type::Float) || (r->type != Value::Value_Type::Int && r->type != Value::Value_Type::Float))
	{
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

Value* Min_Expression::evaluate(ProgramVariables& pv, std::vector<Value*>* registers)
{
	Value* retval = args[0]->evaluate(pv,registers);
	
	for (unsigned i = 1; i < args.size(); ++i)
	{
		Value* a = args[i]->evaluate(pv,registers);
		
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

Value* Max_Expression::evaluate(ProgramVariables& pv, std::vector<Value*>* registers)
{
	Value* retval = args[0]->evaluate(pv,registers);
	
	for (unsigned i = 1; i < args.size(); ++i)
	{
		Value* a = args[i]->evaluate(pv,registers);
		
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
	
	return retval;
}



Script::Script(std::string script, Register* regs)
{
	registers = regs;
	raw_script = script;
	
	//interpret the string into a program, manually creating registers and values
	std::vector<std::string> tokens;
	std::string special_characters = "();.-+/*^";
	for (std::size_t i = 0; i < script.length(); ++i)
	{
		//skip whitespace
		if (script[i] == ' ' || script[i] == '\t' || script[i] == '\n')
		{
			continue;
		}
		
		//grab lonely special characters
		if (special_characters.find_first_of(script[i]) != std::string::npos)
		{
			tokens.push_back(script.substr(i,1));
			continue;
		}
		
		//grab strings
		if (script[i] == '"')
		{
			std::size_t e = i;
			for (++e; e < script.length();++e)
			{
				if (script[e] == '"' && script[e-1] != '\\')
				{
					break;
				}
			}
			
			#ifdef DEBUG
			if (e >= script.length())
			{
				Log::write("\tWarning: End of script reached without finding closing quotation mark.");
			}
			#endif
			tokens.push_back(script.substr(i + 1, e - i - 1));
			i = e;
			continue;
		}
		
		//grab numbers
		if (script[i] >= '0' && script[i] script <= '9')
		{
			std::size_t e = i;
			for(++e; e < script.length() && script[e] >= '0' && script[e] <= '9'; ++e);
			
			tokens.push_back(script.substr(i, e - i));
			i = e - 1;
			continue;
		}
	}
	
	//now that the string is tokenized, interpret those tokens
	std::vector<unsigned> unresolved_tokens;
	for (unsigned i = 0; i < tokens.size(); ++i)
	{
		//the first token in every chunk will be a parenthesis
		bool has_opening_brace = token[i] == "(";
		
		#ifdef DEBUG
			if (!has_opening_brace)
				Log::write("\tWarning: Missing opening paren in expression. Continuing without.");
		#endif
		
		++i;
		
		if (i > tokens.size())
	}
}

Script::~Script()
{
	for (unsigned i = 0; i < expressions.size(); ++i)
	{
		delete expressions[i];
		expressions[i] = nullptr;
	}
}

std::string Script::to_string()
{
	return raw_script;
}

void evaluate(ProgramVariables& pv)
{
	for (unsigned i = 0; i < expressions.size(); ++i)
	{
		Value* v = expressions[i]->evaluate(pv,registers);
		delete v;
	}
}



ScriptSet::ScriptSet(std::string on_creation, std::string on_sight, std::string on_use)
{
	registers = new Register();
	on_creation = on_sight = on_use = nullptr;
	
	if (on_creation != "")
	{
		on_creation_script = new Script(on_creation,registers);
	}
	if (on_sight != "")
	{
		on_sight_script = new Script(on_sight,registers);
	}
	if (on_use != "")
	{
		on_use = new Script(on_use,registers);
	}
}

ScriptSet::~ScriptSet()
{
	for (unsigned i = 0; i < registers.size(); ++i)
	{
		delete registers[i];
		registers[i] = nullptr;
	}
}

void ScriptSet::execute_on_creation(ProgramVariables& pv)
{
	if (on_creation_script != nullptr)
	{
		on_creation_script->evaluate(pv);
	}
}

void ScriptSet::execute_on_sight(ProgramVariables& pv)
{
	if (on_sight_script != nullptr)
	{
		on_sight_script->evaluate(pv);
	}
}

void ScriptSet::execute_on_use(ProgramVariables& pv)
{
	if (on_use_script != nulltpr)
	{
		on_use_script->evaluate(pv);
	}
}

std::string ScriptSet::to_String()
{
	std::string retval;
	
	//first off, build the creation string from the variables in the register
	for (unsigned i = 0; i < registers.size(); ++i)
	{
		retval += "(set " + StringUtils::to_string(i) + " " + registers[i]->to_string() + ");";
	}
	retval += char(4);
	
	//then add in the other two Scripts
	if (on_sight_script != nullptr)
	{
		retval += on_sight_script->to_string();
	}
	retval += char(4);
	if (on_use_script != nullptr)
	{
		retval += on_use_script->to_string();
	}
	
	return retval;
}
