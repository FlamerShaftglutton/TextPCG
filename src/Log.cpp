#include "Log.hpp"
#include <fstream>

void Log::write(std::string message)
{
	std::ofstream o;
	o.open("log.txt",std::ios::out | std::ios::app);
	o << message << std::endl;
	o.close();
}