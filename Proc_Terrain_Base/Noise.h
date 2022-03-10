#pragma once
#include "corecrt_math.h"
#include <cmath>//.abs?
class Noise
{
public:
	Noise();
	~Noise();

	float perlin(float x, float y);// standard perlin noise
	float ridges(float x, float y);// make ridge-like noise using absolute values
	float liquid(float x, float y, float o);// noise where x & y are offset using noise
	
	

	typedef struct {
    float x, y;
	} vector2;

private:
	float interpolate(float a0, float a1, float weight);

	vector2 gradient(int x, int y);

	float dotGradient(int ix, int iy, float x, float y);



};

