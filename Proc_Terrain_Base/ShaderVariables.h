#pragma once
static class ShaderVariables
{
public:
	/* static */ XMMATRIX* World_M = nullptr;
	/* static */ XMMATRIX* View_M = nullptr;
	/* static */ XMMATRIX* Projection_M = nullptr;// const?

	/* static */ int TessellationFactor = 2;
	/* static */ float LODnear =0.f;
	/* static */ float LODfar =64.f;

	/* static */ float Amplitude =1.f;
	/* static */ float Scale =8.f;
	/* static */ XMFLOAT2 seed{};// what if SEED WAS 3 VARIABLS? *mind blow*
	/* static */ XMFLOAT2 GlobalPosition{};

	/* static */ float TimeOfYear =0.f;
	/* static */ float PlanetDiameter =65536.f;
	// add more 
	//(IE perlinsmooth details)   <-[maybe just perfect it in-shader lol]

};

