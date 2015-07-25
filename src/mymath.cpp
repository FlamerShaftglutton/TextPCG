#include "mymath.hpp"
#include <random>

std::mt19937* rng;

void MyMath::start_rng()
{
	 std::random_device rd;
	rng = new std::mt19937(rd);
}

int MyMath::random_int(int minimum, int maximum)
{
	std::uniform_int_distribution<int> dist(minimum,maximum);
	return dist(*rng);
}

float MyMath::random_float(float minimum, float maximum)
{
	std::uniform_real_distribution<float> dist(minimum, std::nextafter(maximum, FLT_MAX));
	return dist(*rng);
}