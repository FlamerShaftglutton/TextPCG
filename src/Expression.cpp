#include "Expression.hpp"
#include "Handle.hpp"
#include <vector>
#include <string>
#include "ScriptingVariables.hpp"
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

Value* Value::evaluate(ScriptingVariables& pv, std::vector<Value*>* registers)
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

Value* Choose_Expression::evaluate(ScriptingVariables& pv, std::vector<Value*>* registers)
{
	Value* index = args[0]->evaluate(pv,registers);
	int i = index->int_val;
	
	if (index->type != Value::Value_Type::Int || i > ((int)args.size() - 2) || i < 0)
	{
		index->type = Value::Value_Type::Error;
		return index;
	}
	
	delete index;
	return args[i + 1]->evaluate(pv,registers);
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

Value* Random_Expression::evaluate(ScriptingVariables& pv, std::vector<Value*>* registers)
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



Set_Expression::Set_Expression(int rn, Expression* arg)
{
	register_number = rn;
	argument = arg;
}

bool Set_Expression::construct(std::vector<Expression*> arguments)
{
	return true;
}

Set_Expression::~Set_Expression()
{
	delete argument;
}

Value* Set_Expression::evaluate(ScriptingVariables& pv, std::vector<Value*>* registers)
{
	Value* v = argument->evaluate(pv,registers);
	
	if (register_number < 0)
	{
		if (register_number == -1)
		{
			if (v->type == Value::Value_Type::String)
			{
				(*(pv.main_text)) += "\n\n\"" + v->string_val + "\"";
			}
			else
			{
				#ifdef DEBUG
					Log::write("ERROR:Invalid type for setting variable main_text. Value not changed.");
				#endif
				
				v->type = Value::Value_Type::Error;
			}
		}
		else
		{
			#ifdef DEBUG
				Log::write("ERROR:Invalid variable name in Set call. No values changed.");
			#endif
			
			v->type = Value::Value_Type::Error;
		}
	}
	else if (register_number < (int)registers->size())
	{
		delete ((*registers)[register_number]);
		(*registers)[register_number] = copy_value(v);
	}
	else
	{
		#ifdef DEBUG
			Log::write("ERROR:Invalid register number in Set call. Value not changed.");
		#endif
		
		v->type = Value::Value_Type::Error;
	}
	
	return v;
}



Get_Expression::Get_Expression(int rn)
{
	register_number = rn;
}

bool Get_Expression::construct(std::vector<Expression*> arguments)
{
	return true;
}

Value* Get_Expression::evaluate(ScriptingVariables& pv, std::vector<Value*>* registers)
{
	//add in the ability to select game variables as well
	if (register_number < 0)
	{
		if (register_number == -1)
		{
			Value* v = new Value;
			v->type = Value::Value_Type::String;
			v->string_val = (*pv.main_text);
			return v;
		}
		else
		{
			#ifdef DEBUG
				Log::write("ERROR:Invalid variable name in Get call.");
			#endif
			
			Value* v = new Value;
			v->type = Value::Value_Type::Error;
			return v;
		}
	}
	else if (register_number < (int)registers->size())
	{
		return copy_value((*registers)[register_number]);
	}

	#ifdef DEBUG
		Log::write("ERROR:Invalid register number in Get call.");
	#endif
	
	Value* v = new Value;
	v->type = Value::Value_Type::Error;
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

Value* Add_Expression::evaluate(ScriptingVariables& pv, std::vector<Value*>* registers)
{
	Value* l = args[0]->evaluate(pv,registers);
	
	if (l->type == Value::Value_Type::Int)
	{
		for (std::size_t i = 1; i < args.size(); ++i)
		{
			Value* r = args[i]->evaluate(pv,registers);
			
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
				l->type = Value::Value_Type::Error;
				delete r;
				break;
			}
			
			delete r;
		}
		
		return l;
	}
	else if (l->type == Value::Value_Type::Float)
	{
		for (std::size_t i = 1; i < args.size(); ++i)
		{
			Value* r = args[i]->evaluate(pv,registers);
			
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
				l->type = Value::Value_Type::Error;
				delete r;
				break;
			}
			
			delete r;
		}
		
		return l;
	}
	else if (l->type == Value::Value_Type::String)
	{
		for (std::size_t i = 1; i < args.size(); ++i)
		{
			Value* r = args[i]->evaluate(pv,registers);
			
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
			else
			{
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
}

Value* Subtract_Expression::evaluate(ScriptingVariables& pv, std::vector<Value*>* registers)
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
}

Value* Multiply_Expression::evaluate(ScriptingVariables& pv, std::vector<Value*>* registers)
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

Value* Divide_Expression::evaluate(ScriptingVariables& pv, std::vector<Value*>* registers)
{
	Value* l = lhs->evaluate(pv,registers);
	Value* r = rhs->evaluate(pv,registers);
	if ((l->type != Value::Value_Type::Int && l->type != Value::Value_Type::Float) || (r->type != Value::Value_Type::Int && r->type != Value::Value_Type::Float) || (r->type == Value::Value_Type::Int && r->int_val == 0) || (r->type == Value::Value_Type::Float && r->float_val == 0.0f))
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

Value* Power_Expression::evaluate(ScriptingVariables& pv, std::vector<Value*>* registers)
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

Value* Min_Expression::evaluate(ScriptingVariables& pv, std::vector<Value*>* registers)
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

Value* Max_Expression::evaluate(ScriptingVariables& pv, std::vector<Value*>* registers)
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
}

Value* Say_Expression::evaluate(ScriptingVariables& pv, std::vector<Value*>* registers)
{
	Value* v = arg->evaluate(pv,registers);

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
		
		std::string name = *(pv.caller.name);
		std::string c = *(pv.caller.friendly) ? "<fg=green>" : "<fg=red>";
		
		(*(pv.main_text)) += "\n\n<fg=white>\"" + v->string_val + "\" " + s + " " + c + name + "<fg=white>.";
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
