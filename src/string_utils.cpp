#include "string_utils.hpp"
#include <string>
#include <sstream>

std::string StringUtils::to_lowercase(std::string input)
{
	for (unsigned i = 0; i < input.length(); ++i) 
	{
		if (input[i] >= 'A' && input[i] <= 'Z')
		{
			input[i] -= ('A' - 'a');
		}
	} 
	
	return input;
}

std::string StringUtils::to_string(int input)
{
	std::stringstream s;
	
	s << input;
	
	return s.str();
}