#pragma once
#include "TextureShader.h"//"BaseShader.h"//

using namespace std;
using namespace DirectX;//?

class CloudShader :
    public TextureShader //BaseShader //
{
protected:
	struct timeBuffer {// same as in tex. shader
		float time;
		float timeOfYear;
		int day;
		int padding = 0;
	};
public:
	CloudShader(ID3D11Device* device, HWND hwnd);
	~CloudShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, float variable = -1);

private:
	void initShader(const wchar_t* vs, const wchar_t* gs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	//ID3D11Buffer* SECONDmatrixBuffer;// be
	ID3D11Buffer* variableBuffer;
	ID3D11SamplerState* sampleState;
};

