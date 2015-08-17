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

std::string StringUtils::to_string(float input)
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

int StringUtils::stoi(std::string input)
{
	std::stringstream s(input);
	int output = 0;
	s >> output;
	
	return output;
}

float StringUtils::stof(std::string input)
{
	std::stringstream s(input);
	float output = 0.0f;
	s >> output;
	
	return output;
}

std::vector<std::string> StringUtils::split(std::string input, char delimiter)
{
	std::vector<std::string> retval;
    std::stringstream ss(input);
    std::string item;
    while (std::getline(ss, item, delimiter)) {
        retval.push_back(item);
    }
    return retval;
}

std::string StringUtils::replace(std::string subject, const std::string& search, const std::string& replace)
{
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos)
    {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
    return subject;
}
