#include "mymath.hpp"
#include <random> //apparently MinGW doesn't fully support C++11 random functionality yet, so I'll use the following for RNG until it does. Linux builds can easily be updated with the better code.
#include <cstdint>

//std::mt19937* rng;

uint32_t w,x,y,z;

int MyMath::start_rng()
{
	std::random_device rd;
	int seed = rd();
	return start_rng(seed);
	//rng = new std::mt19937(rd);
	//w = (uint32_t)rd();
	//x = (uint32_t)rd();
	//y = (uint32_t)rd();
	//z = (uint32_t)rd();
}

int MyMath::start_rng(int seed)
{
	w = (uint32_t)seed & 0x000000FF;
	x = (uint32_t)seed & 0x0000FF00;
	y = (uint32_t)seed & 0x00FF0000;
	z = (uint32_t)seed & 0xFF000000;
	
	return seed;
}

int MyMath::random_int(int minimum, int maximum)
{
	//std::uniform_int_distribution<int> dist(minimum,maximum);
	//return dist(*rng);
	
	if (minimum >= maximum)
	{
		return minimum;
	}
	
	//use the XORshift algorithm for RNG
	uint32_t t = x ^ (x << 11);
    x = y; y = z; z = w;
	w = w ^ (w >> 19) ^ t ^ (t >> 8);
    return (w % (1 + maximum - minimum)) + minimum;
}

float MyMath::random_float(float minimum, float maximum)
{
	//std::uniform_real_distribution<float> dist(minimum, maximum);
	//return dist(*rng);
	
	return (float)MyMath::random_int(0,8388607) / 8388607.0f;
}

bool MyMath::between(int a, int minimum, int maximum)
{
	return (a >= minimum && a <= maximum);
}