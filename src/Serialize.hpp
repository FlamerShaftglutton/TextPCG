#pragma once
#include <string>

struct GameState;

namespace Serialize
{
	void from_file(std::string fname, GameState& gs);
	void to_file(std::string fname, GameState& gs);
};