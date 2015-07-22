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

std::string StringUtils::trim(std::string input)
{
	std::size_t fpos = input.find_first_not_of(' ');
	
	if (fpos == std::string::npos)
		return "";
	
	std::size_t lpos = input.find_last_not_of(' ');
	
	return input.substr(fpos, 1 + lpos - fpos);
}