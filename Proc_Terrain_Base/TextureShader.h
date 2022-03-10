
#pragma once

#include "BaseShader.h"

using namespace std;
using namespace DirectX;

class TextureShader : public BaseShader
{
protected:
	struct memeBuffer {// just to send time to sky. cba making a sky/sun shader just now
		float time;
		float timeOfYear;
		int day;
		int padding = 0;
	};
public:
	TextureShader(ID3D11Device* device, HWND hwnd, bool t = true);
	~TextureShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, float variable = -1);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* SECONDmatrixBuffer;// because i really cba making a sky shader
	ID3D11Buffer* variableBuffer;
	ID3D11SamplerState* sampleState;
};
