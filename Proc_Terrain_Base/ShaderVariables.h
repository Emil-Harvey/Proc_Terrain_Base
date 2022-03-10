#pragma once
class ShaderVariables
{
public:
	XMMATRIX* World_M = nullptr;
	XMMATRIX* View_M = nullptr;
	XMMATRIX* Projection_M = nullptr;// const?

	int TessellationFactor;
	float LODnear;
	float LODfar;

	float Amplitude;
	float Scale;
	XMFLOAT2 seed;// what if SEED WAS 3 VARIABLS? *mind blow*
	XMFLOAT2 GlobalPosition;

	float TimeOfYear;
	float PlanetDiameter;
	// add more 
	//(IE perlinsmooth details)   <-[maybe just perfect it in-shader lol]

};

