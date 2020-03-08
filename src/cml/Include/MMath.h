#pragma once

#include <cmath>
#include <cstdlib>
#include <cassert>
#include <random>

#ifndef PI
#define PI 3.14159265358979323846
#endif
#define PI_FLOAT static_cast<float>(PI)
#define TAU 6.28318530717958647692
#define TAU_FLOAT static_cast<float>(TAU)

static std::random_device rd;
static std::mt19937 mt(rd());

inline int RandomNumber(int nMin, int nMax)
{
	std::uniform_int_distribution<int> dist(nMin, nMax);
	return dist(mt);
}

inline float RandomNumber(float fMin, float fMax)
{
	std::uniform_real_distribution<float> dist(fMin, fMax);
	return dist(mt);
}

inline long Dice(unsigned char n, unsigned char sides, short mod)
{
    int result = mod;
    for(int i = 0; i < n; i++)
    {
		result += RandomNumber(1, sides);
    }
    return result;
}

inline float Roundf(float x)
{
	return floorf(x + .5f);
}

#define ToRadian( degree ) ((degree) * (PI_FLOAT / 180.0f))
#define ToDegree( radian ) ((radian) * (180.0f / PI_FLOAT))
