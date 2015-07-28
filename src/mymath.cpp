#include "mymath.hpp"
//#include <random>

//TODO: Fix the random number generation stuff

//std::mt19937* rng;

void MyMath::start_rng()
{
	// std::random_device rd;
	//rng = new std::mt19937(rd);
}

int MyMath::random_int(int minimum, int maximum)
{
	//std::uniform_int_distribution<int> dist(minimum,maximum);
	//return dist(*rng);
	return 6;
}

float MyMath::random_float(float minimum, float maximum)
{
	//std::uniform_real_distribution<float> dist(minimum, maximum);
	//return dist(*rng);
	return 6.0f;
}