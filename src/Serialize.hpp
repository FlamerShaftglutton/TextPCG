#pragma once
#include <string>
#include "GameState.hpp"

namespace Serialize
{
	void from_file(std::string fname, GameState& gs);
	void to_file(std::string fname, GameState& gs);
};