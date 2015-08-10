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
	#include <fstream>
	#include <sstream>
#endif

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
		//if it's a non-quoted string
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
				//if this is a set function, grab the next argument
				Expression* e;
				if (s == "set")
				{
					if (tokens.size() < 3)
					{
						#ifdef DEBUG
							Log::write("ERROR: argument list too short for Set function. Trying to set variable or register '" + tokens[1] + "'.");
						#endif
						
						Value* v = new Value;
						v->type = Value::Value_Type::Error;
						return v;
					}
					
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
				}
				
				//either the next thing is a number or it's a non-quoted string
				int rn;
				//if it's a number...
				if (token_types[1] == 3)
				{
					rn = StringUtils::stoi(tokens[1]);
					
					//set up the register if necessary
					if (rn >= (int)registers->size())
					{
						Value* v = new Value;
						v->type = Value::Value_Type::Int;
						v->int_val = 0;
						registers->push_back(v);
					}
					
					//finally, make the node
					if (s == "get")
					{
						Get_Register_Expression* e = new Get_Register_Expression(rn);
						return e;
					}
					else
					{
						Set_Register_Expression* se = new Set_Register_Expression(rn,e);
						return se;
					}
					
				}
				//if it's a variable name...
				else if (token_types[1] == 5)
				{
					//if it's a keyword...
					std::string lower_token_1 = StringUtils::to_lowercase(tokens[1]);
					if (tokens[1].find('.') != std::string::npos || lower_token_1 == "main_text")
					{
						Expression* retval = nullptr;
						
						if (s == "get")
						{
							retval = new Get_Variable_Expression(lower_token_1);
						}
						else
						{
							retval = new Set_Variable_Expression(lower_token_1, e);
						}
						
						std::vector<Expression*> arg_list;
						
						if (!retval->construct(arg_list))
						{
							#ifdef DEBUG
								Log::write("ERROR: unable to resolve variable name or number in get or set function");
							#endif
							
							delete retval;
							Value* v = new Value;
							v->type = Value::Value_Type::Error;
							return v;
						}
						
						//if it's well formed, return it!
						return retval;
					}
					//if it's not a keyword, then make it into a register
					else
					{
						//someday...
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
				else if (s == "say")
				{
					retval = new Say_Expression;
				}
				else if (s == "if")
				{
					retval = new If_Expression;
				}
				else if (s == "and" || s == "&")
				{
					retval = new And_Expression;
				}
				else if (s == "not" || s == "!")
				{
					retval = new Not_Expression;
				}
				else if (s == "or" || s == "|")
				{
					retval = new Or_Expression;
				}
				else if (s == "xor")
				{
					retval = new Xor_Expression;
				}
				else if (s == "<")
				{
					retval = new LessThan_Expression;
				}
				else if (s == ">")
				{
					retval = new GreaterThan_Expression;
				}
				else if (s == "<=")
				{
					retval = new LessThanEqual_Expression;
				}
				else if (s == ">=")
				{
					retval = new GreaterThanEqual_Expression;
				}
				else if (s == "=")
				{
					retval = new Equal_Expression;
				}
				else if (s == "!=")
				{
					retval = new NotEqual_Expression;
				}
				else if (s == "between")
				{
					retval = new Between_Expression;
				}
				else if (s == "feoir")
				{
					retval = new FEOIR_Expression;
				}
				else if (s == "attack")
				{
					retval = new Attack_Expression;
				}
				else if (s == "defend")
				{
					retval = new Defend_Expression;
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
	#ifdef DEBUG
		Log::write("ERROR: Unable to parse expression.");
	#endif
	
	Value* v = new Value;
	v->type = Value::Value_Type::Error;
	return v;
}

Script::Script(std::string script, Register* regs)
{
	registers = regs;
	raw_script = script;
	
	#ifdef DEBUG
		//if debugging, then the script can just specify a file to read from
		if (script.substr(0,5) == "file:")
		{
			Log::write("Warning: using a script from a file. This will not work when compiled without the DEBUG flag!");
			Log::write("\tFilename: " + script.substr(5));
			
			//read the whole thing in and assign the value to script
			std::ifstream infile(script.substr(5).c_str());
			
			if (infile.is_open())
			{
				std::stringstream ss;
				ss << infile.rdbuf();
				script = StringUtils::replace(ss.str(),"\\n","\n");
			
				Log::write("\tScript pulled successfully.");
			}
			else
			{
				Log::write("\tCouldn't open the file...");
			}
		}
	#endif
	
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
			
			//handle an empty string
			if (e - i == 1)
			{
				tokens.push_back("");
			}
			//handle a normal string
			else
			{
				tokens.push_back(script.substr(i + 1, e - i - 1));
			}
			
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
			
			#ifdef DEBUG
				std::vector<std::string> tokens_subset_copy(tokens_subset);
			#endif
			
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
					std::string outval = "";
					for (unsigned l = 0; l < tokens_subset_copy.size(); ++l)
					{
						outval += tokens_subset_copy[l] + " ";
					}
					Log::write("Warning: No semicolon after expression '" + outval + "'. Continuing anyway.");
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
	on_creation_script = on_sight_script = on_use_script = on_attack_step_script = nullptr;
}

void ScriptSet::construct(std::string on_creation, std::string on_sight, std::string on_use, std::string on_attack_step)
{
	registers = new Register();
	
	if (on_creation != "")
	{
		on_creation_script = new Script(on_creation, registers);
	}
	if (on_sight != "")
	{
		on_sight_script = new Script(on_sight, registers);
	}
	if (on_use != "")
	{
		on_use_script = new Script(on_use, registers);
	}
	if (on_attack_step != "")
	{
		on_attack_step_script = new Script(on_attack_step, registers);
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
	if (on_attack_step_script != nullptr)
	{
		delete on_attack_step_script;
	}
}

void ScriptSet::execute_on_creation()
{
	if (on_creation_script != nullptr)
	{
		ScriptingVariables pv;
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

void ScriptSet::execute_on_attack_step(ScriptingVariables& pv)
{
	if (on_attack_step_script != nullptr)
	{
		on_attack_step_script->evaluate(pv);
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
		
	//then add in the other Scripts
	if (on_sight_script != nullptr)
	{
		retval += on_sight_script->to_string();
	}
	retval += char(4);
	if (on_use_script != nullptr)
	{
		retval += on_use_script->to_string();
	}
	retval += char(4);
	if (on_attack_step_script != nullptr)
	{
		retval += on_attack_step_script->to_string();
	}
	
	return retval;
}
