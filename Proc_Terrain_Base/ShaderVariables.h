#pragma once
static class ShaderVariables
{
public:
	XMMATRIX* World_M = nullptr;
	XMMATRIX* View_M = nullptr;
	XMMATRIX* Projection_M = nullptr;// const?

	int TessellationFactor = 2;
	float LODnear =0.f;
	float LODfar =64.f;

	float Amplitude =1.f;
	float Scale =8.f;
	XMFLOAT2 seed{};// what if SEED WAS 3 VARIABLS? *mind blow*
	XMFLOAT2 GlobalPosition{};

	float TimeOfYear =0.f;
	float PlanetDiameter =65536.f;
	// add more 
	//(IE perlinsmooth details)   <-[maybe just perfect it in-shader lol]

};

