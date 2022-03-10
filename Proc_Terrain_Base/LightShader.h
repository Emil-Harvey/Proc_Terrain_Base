#pragma once

#include "DXF.h"

using namespace std;
using namespace DirectX;

class LightShader : public BaseShader
{
protected:
	struct LightBufferType
	{
		XMFLOAT4 ambient;
		XMFLOAT4 diffuse;
		XMFLOAT3 direction;
		float specular;
	};
	struct CameraBufferType
	{
		XMFLOAT3 position;
		float padding;
	};
	struct DataBufferType
	{
		XMFLOAT2 position = { 0,0 };
		float scale = 1;
		float time = 0;
		int factor = 1;// tessellation factor
		XMFLOAT3 extra = { 0,0,0 };// manipulation details
	};

public:
	LightShader(ID3D11Device* device, HWND hwnd);
	~LightShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView** textures, Light* light, FPCamera* camera, XMFLOAT2 chunkPos, float scale, float timeOfYear);

protected:
	void initShader(const wchar_t* vs, const wchar_t* ps);

	
protected:
	ID3D11Buffer * matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* cameraBuffer;
	ID3D11Buffer* chunkBuffer;
};

