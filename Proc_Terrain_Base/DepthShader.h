// depth shader.h
// for depth map(s)
#pragma once

#include "DXF.h"

using namespace std;
using namespace DirectX;


class DepthShader : public BaseShader
{
public:
	struct ManipulationBufferType {
		int tessellationFactor = 1;
		XMFLOAT3 extra = { 0,0,0 };
	};

	DepthShader(ID3D11Device* device, HWND hwnd);
	~DepthShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* heightMap = NULL, float amplitude = 0);

protected:
	void initShader(const wchar_t* vs, const wchar_t* ps);


	ID3D11Buffer* manBuffer;
	ID3D11Buffer* matrixBuffer;
};