#pragma once
#include "TessShader.h"
#include "ShaderVariables.h"
class TerrainShader :
    public TessShader
{
    struct TerrainDataBufferType {
		XMFLOAT2 seed;

		float Scale;
		float TimeOfYear;
		int TessellationFactor;
		float LODnear;
		float LODfar;
		float Amplitude;

		XMFLOAT2 GlobalPosition;
		float PlanetDiameter;
		float padding =  0; /// maybe just use ShaderVariables??
    };
public:
    TerrainShader(ID3D11Device* device, HWND hwnd);
    void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView** textures, Light* light, FPCamera* camera, ShaderVariables* Variables, ID3D11ShaderResourceView* heightmap);//XMFLOAT4 TessellationDetails, float scale, XMFLOAT2 NoiseOffset, float timeOfYear);
	void unbind(ID3D11DeviceContext* deviceContext); // I thought this would solve the problem but it doesn't (?)

protected:
    void initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* gsFilename, const wchar_t* psFilename);

protected:
	ID3D11SamplerState* heightmapSampleState;
};

