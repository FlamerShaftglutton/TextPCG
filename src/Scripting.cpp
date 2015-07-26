#include "Scripting.hpp"
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

Value* Add_Expression::evaluate(ScriptingVariables& pv, std::vector<Value*>* registers)
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



Expression* Script::recursively_resolve(std::vector<std::string>& tokens, std::vector<int>& token_types)
{
	//first, check if it's a value
	if (tokens.size() == 1)
	{
		Value* v = new Value;
		
		//if it's a number
		if (token_types[0] == 3)
		{
			if (tokens[0].find('.') != std::string::npos)
			{
				v->type = Value::Value_Type::Float;
				v->float_val = StringUtils::stof(tokens[0]);
			}
			else
			{
				v->type = Value::Value_Type::Int;
				v->int_val = StringUtils::stoi(tokens[0]);
			}
		}
		//if it's a quoted string
		else if (token_types[0] == 4)
		{
			v->type = Value::Value_Type::String;
			v->string_val = tokens[0];
		}
		//if it's a non-quoted string, then it should be a boolean
		else if (token_types[0] == 5 && (StringUtils::to_lowercase(tokens[0]) == "true" || StringUtils::to_lowercase(tokens[0]) == "false"))
		{
			v->type = Value::Value_Type::Bool;
			v->bool_val = StringUtils::to_lowercase(tokens[0]) == "true";
		}
		//if it's something else, then there's an issue
		else
		{
			#ifdef DEBUG
				Log::write("ERROR: invalid value");
			#endif
			
			v->type = Value::Value_Type::Error;
		}
		
		//return the value			
		return v;
	}
	//if it's not a value, it must be a function
	else if (tokens.size() > 1)
	{
		//the first thing should be a non-quoted string, which will be the function name
		if (token_types[0] == 5)
		{
			std::string s = StringUtils::to_lowercase(tokens[0]);
			//if this is a Get or Set call, then this has to be set up a little different
			if (s == "get" || s == "set")
			{
				//either the next thing is a number or it's a non-quoted string
				int rn;
				if (token_types[1] == 3)
				{
					rn = StringUtils::stoi(tokens[1]);
					
					//set up the registers if necessary
					if (rn >= (int)registers->size())
					{
						Value* v = new Value;
						v->type = Value::Value_Type::Int;
						v->int_val = 0;
						registers->push_back(v);
					}
				}
				else if (token_types[1] == 5)
				{
					//start looking through the variable namespace
					//std::vector<std::string> program_variables = {"current_room","main_text"};
					//std::vector<std::string> room_variables = {"description","short_description","minimap_symbol"};
					//std::vector<std::string> object_variables = {"visible","visible_in_short_description","friendly","mobile","hitpoints", 
					if (tokens[1] == "main_text")
					{
						rn = -1;
					}
					else
					{
						#ifdef DEBUG
							Log::write("ERROR: unable to resolve variable name or number in get or set function");
						#endif
						
						Value* v = new Value;
						v->type = Value::Value_Type::Error;
						return v;
					}
				}
				//if it's neither, then there's a problem
				else
				{
					#ifdef DEBUG
						Log::write("ERROR: unable to resolve variable name or number in get or set function");
					#endif
					
					Value* v = new Value;
					v->type = Value::Value_Type::Error;
					return v;
				}
				
				//if it's a 'get', then we're done
				if (s == "get")
				{
					Get_Expression* e = new Get_Expression(rn);
					return e;
				}
				//if it's a set, then we have to get another argument
				else
				{
					if (tokens.size() < 3)
					{
						#ifdef DEBUG
							Log::write("ERROR: argument list too short for Set function.");
						#endif
						
						Value* v = new Value;
						v->type = Value::Value_Type::Error;
						return v;
					}
					
					Expression* e;
					std::vector<std::string> ts;
					std::vector<int> tts;
					
					//if it's a value then this is easy
					if (tokens.size() == 3)
					{
						ts.push_back(tokens[2]);
						tts.push_back(token_types[2]);
					}
					//if it's enclosed in parentheses, we have to remove those first
					else if (token_types[2] == 0 && token_types.back() == 1)
					{
						ts.insert(ts.begin(),tokens.begin() + 3, tokens.end()-1);
						tts.insert(tts.begin(),token_types.begin() + 3, token_types.end()-1);
					}
					//if it's neither, something's wrong
					else
					{
						#ifdef DEBUG
							Log::write("ERROR: second argument for set function unrecognizable.");
						#endif
						
						Value* v = new Value;
						v->type = Value::Value_Type::Error;
						return v;
					}
					
					//now actually resolve it and create the Set expression node
					e = recursively_resolve(ts,tts);
					Set_Expression* se = new Set_Expression(rn,e);
					return se;
				}
			}
			else
			{
				//first off, recursively create a list of arguments
				std::vector<Expression*> arg_list;
				int left = 0;
				int right = 0;
				std::size_t lpos = 1;
				
				for (unsigned i = 1; i < tokens.size(); ++i)
				{
					if (token_types[i] == 0)
						++left;
					if (token_types[i] == 1)
						++right;
					if (left == right)
					{
						//then add a new argument to the list (recursively)
						if (i == lpos)
						{
							std::vector<std::string> tl;
							std::vector<int> ttl;
							
							tl.push_back(tokens[i]);
							ttl.push_back(token_types[i]);
							
							arg_list.push_back(recursively_resolve(tl,ttl));
							++lpos;
						}
						else
						{
							std::vector<std::string> tl(tokens.begin() + lpos + 1, tokens.begin() + i);
							std::vector<int> ttl(token_types.begin() + lpos + 1, token_types.begin() + i);
							lpos = i + 1;
							arg_list.push_back(recursively_resolve(tl,ttl));
						}
					}
				}
				
				//then, grab the right constructor based on the string
				bool it_worked = false;
				Expression* retval;
				
				if (s == "choose")
				{
					retval = new Choose_Expression;
				}
				else if (s == "random")
				{
					retval = new Random_Expression;
				}
				else if (s == "+")
				{
					retval = new Add_Expression;
				}
				else if (s == "-")
				{
					retval = new Subtract_Expression;
				}
				else if (s == "*")
				{
					retval = new Multiply_Expression;
				}
				else if (s == "/")
				{
					retval = new Divide_Expression;
				}
				else if (s == "^")
				{
					retval = new Power_Expression;
				}
				else if (s == "min")
				{
					retval = new Min_Expression;
				}
				else if (s == "max")
				{
					retval = new Max_Expression;
				}
				#ifdef DEBUG
				else
				{
					Log::write("ERROR: invalid function name");
				}
				#endif
				
				it_worked = retval->construct(arg_list);
				
				if (it_worked)
				{
					return retval;
				}
				else
				{
					Value* v = new Value;
					v->type = Value::Value_Type::Error;
					return v;
				}
			}
		}
		else
		{
			#ifdef DEBUG
				Log::write("ERROR: invalid function call");
			#endif
			
			Value* v = new Value;
			v->type = Value::Value_Type::Error;
			return v;
		}
	}
	//if it's neither, something's wrong
	else
	{
		#ifdef DEBUG
			Log::write("ERROR: Unable to parse expression.");
		#endif
		
		Value* v = new Value;
		v->type = Value::Value_Type::Error;
		return v;
	}
}

Script::Script(std::string script, Register* regs)
{
	registers = regs;
	raw_script = script;
	
	//interpret the string into a program, manually creating registers and values
	std::vector<std::string> tokens;
	std::vector<int> token_types; //0: opening paren, 1: closing paren, 2: semicolon, 3: number, 4: quoted string, 5: quoteless string
	for (std::size_t i = 0; i < script.length(); ++i)
	{
		//skip whitespace
		if (script[i] == ' ' || script[i] == '\t' || script[i] == '\n')
		{
			continue;
		}
		
		//grab lonely special characters
		if (script[i] == '(' || script[i] == ')' || script[i] == ';')
		{
			tokens.push_back(script.substr(i,1));
			token_types.push_back(script[i] == '(' ? 0 : script[i] == ')' ? 1 : 2);
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
			token_types.push_back(4);
			continue;
		}
		
		//grab numbers
		if ((script[i] >= '0' && script[i] <= '9') || ((script[i] == '-' || script[i] == '+') && (i+1) < script.size() && script[i+1] >= '0' && script[i+1] <= '9'))
		{
			std::size_t e = i;
			//grab the first chunk of numerals
			for(++e; e < script.length() && script[e] >= '0' && script[e] <= '9'; ++e);
			
			//the next character could be a period
			if (script[e] == '.')
			{
				#ifdef DEBUG
					if ((e+1) >= script.size() || script[e+1] < '0' || script[e+1] > '9')
					{
						Log::write("\tERROR: malformed numeric literal has no numerals after period character.");
					}
				#endif
				
				//grab some more numerals after this
				for (++e; e < script.length() && script[e] >= '0' && script[e] <= '9'; ++e);
			}
			
			//the next charcter can be an e or E
			if (script[e] == 'e' || script[e] == 'E')
			{
				//there can be a sign symbol in front of the numerals
				#ifdef DEBUG
					if ((e+1) >= script.size())
					{
						Log::write("\tERROR: malformed numeric literal has nothing after E character.");
					}
				#endif
				if (script[e+1] == '-' || script[e+1] == '+')
				{
					++e;
				}
				
				//there can be numerals after that, and that's it
				#ifdef DEBUG
					if ((e+1) >= script.size() || script[e+1] < '0' || script[e+1] > '9')
					{
						Log::write("\tERROR: malformed numeric literal has no numerals after E character.");
					}
				#endif
				
				for (++e; e < script.length() && script[e] >= '0' && script[e] <= '9'; ++e);
			}
			
			tokens.push_back(script.substr(i, e - i));
			token_types.push_back(3);
			i = e - 1;
			
			#ifdef DEBUG
				if (e < script.length() && script[e] != ' ' && script[e] != ')' && script[e] != '\t' && script[e] != '\n')
				{
					Log::write("\tWarning: numeric literal directly followed by non-separater. Proceeding as if a space existed.");
				}
			#endif
			
			continue;
		}
		
		//grab strings that are not in quotations
		std::size_t e;
		for (e = i + 1; e < script.length() && script[e] != ' ' && script[e] != ')' && script[e] != '\t' && script[e] != '\n'; ++e);
		tokens.push_back(script.substr(i, e - i));
		token_types.push_back(5);
		i = e - 1;
	}
	
	//now that the string is tokenized, interpret those tokens
	for (unsigned i = 0; i < tokens.size(); ++i)
	{
		bool seems_valid = true;
		seems_valid &= token_types[i] == 0;
		seems_valid &= tokens.size() >= 3 && token_types[i+1] == 5;
		
		//if it seems valid so far, push a little further to find out
		unsigned j;
		unsigned lefts = 1;
		unsigned rights = 0;
		for (j = i + 2; j < tokens.size(); ++j)
		{
			if (token_types[j] == 0)
			{
				++lefts;
			}
			else if (token_types[j] == 1)
			{
				++rights;
			}
			
			if (lefts == rights)
			{
				break;
			}
		}
		
		seems_valid &= j < tokens.size();
		
		//if it all checks out then recursively create the expression
		if (seems_valid)
		{
			std::vector<std::string> tokens_subset(tokens.begin() + i + 1, tokens.begin() + j); //j - 1
			std::vector<int> token_types_subset(token_types.begin() + i + 1, token_types.begin() + j); //j - 1
			
			Expression* ex = recursively_resolve(tokens_subset, token_types_subset);
			expressions.push_back(ex);
		
			//finally, check if there's a semicolon afterwards
			if ((j+1) < tokens.size() && token_types[j+1] == 2)
			{
				//consume it, but don't do anything with it
				++j;
				
			}
			//send out a warning, but otherwise don't do anything
			#ifdef DEBUG
				else
				{
					Log::write("Warning: No semicolon after expression. Continuing anyway.");
				}
			#endif
			
			i = j;
		}
		//if it doesn't check out, then something went wrong
		else
		{
			#ifdef DEBUG
				Log::write("Function parens or operator malformed");
			#endif
			
			Value* v = new Value;
			v->type = Value::Value_Type::Error;
			expressions.push_back(v);
			
			//skip ahead to the next expression
			for (;i < token_types.size() && token_types[i] != 2; ++i);
		}
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

void Script::evaluate(ScriptingVariables& pv)
{
	for (unsigned i = 0; i < expressions.size(); ++i)
	{
		Value* v = expressions[i]->evaluate(pv,registers);
		delete v;
	}
}



ScriptSet::ScriptSet()
{
	registers = nullptr;
	on_creation_script = on_sight_script = on_use_script = nullptr;
}

void ScriptSet::construct(std::string on_creation, std::string on_sight, std::string on_use)
{
	registers = new Register();
	on_creation_script = on_sight_script = on_use_script = nullptr;
	
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
		on_use_script = new Script(on_use,registers);
	}
}

ScriptSet::~ScriptSet()
{
	if (registers != nullptr)
	{
		for (unsigned i = 0; i < registers->size(); ++i)
		{
			delete (*registers)[i];
			(*registers)[i] = nullptr;
		}
		delete registers;
	}
	
	if (on_creation_script != nullptr)
	{
		delete on_creation_script;
	}
	if (on_sight_script != nullptr)
	{
		delete on_sight_script;
	}
	if (on_use_script != nullptr)
	{
		delete on_use_script;
	}
}

void ScriptSet::execute_on_creation(ScriptingVariables& pv)
{
	if (on_creation_script != nullptr)
	{
		on_creation_script->evaluate(pv);
	}
}

void ScriptSet::execute_on_sight(ScriptingVariables& pv)
{
	if (on_sight_script != nullptr)
	{
		on_sight_script->evaluate(pv);
	}
}

void ScriptSet::execute_on_use(ScriptingVariables& pv)
{
	if (on_use_script != nullptr)
	{
		on_use_script->evaluate(pv);
	}
}

std::string ScriptSet::to_string()
{
	std::string retval;
	
	//first off, build the creation string from the variables in the register
	if (registers != nullptr)
	{
		for (unsigned i = 0; i < registers->size(); ++i)
		{
			retval += "(set " + StringUtils::to_string((int)i) + " " + (*registers)[i]->to_string() + ");";
		}
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
