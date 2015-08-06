#pragma once
#include <string>
#include <vector>

namespace StringUtils
{
	std::string to_lowercase(std::string input);
	std::string to_string(int input);
	std::string to_string(float input);
	std::string trim(std::string input);
	int stoi(std::string input);
	float stof(std::string input);
	std::vector<std::string> split(std::string input, char delimiter);
	std::string replace(std::string subject, const std::string& search, const std::string& replace);
}
