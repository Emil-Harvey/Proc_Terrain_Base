#pragma once

#include "Simplex.h"
//#include <random>

class Noise
{
public:
	//Noise();
	//~Noise();
	typedef struct {
    float x, y;
	} vector2;

	static float perlin(float x, float y);// standard perlin noise
	static float ridges(float x, float y);// make ridge-like noise using absolute values
	static float liquid(float x, float y, float o);// noise where x & y are offset using noise
	
	static float flow(vect3 i, int octaves);

	static float NoiseTexture(vect3 coords, float scale, int octs, float roughness, float distortion);//Blender-style noise

	static float terrain_height(float x, float y, float z = 0.f, int octaves = 10);


private:
	static float interpolate(float a0, float a1, float weight);
	static float lerp(float a0, float a1, float weight);

	static float smooth_min(const float& a, const float& b, const float fac);

	static vector2 gradient(const int& x,const int& y);

	static float dotGradient(int ix, int iy, float x, float y);

	static float sign(float x) { return signbit(x) ? -1.0f : 1.0f; }


};

