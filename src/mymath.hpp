#pragma once

namespace MyMath
{
	void start_rng();
	void start_rng(int seed);
	int random_int(int minimum, int maximum);
	float random_float(float minimum, float maximum);
	bool between(int a, int minimum, int maximum);
}