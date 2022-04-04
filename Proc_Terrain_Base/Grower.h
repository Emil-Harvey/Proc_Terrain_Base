#pragma once
#include "QuadMesh.h"/// not necessary
#include "DXF.h"
#include "LightShader.h"
#include "ShaderVariables.h"

using namespace std;
using namespace DirectX;


class Grower : public LightShader
{
public:

	struct PositionBufferType {
		XMFLOAT4 position;
	};
	Grower(ID3D11Device* device, HWND hwnd);

	
	//override to take 1 texture
	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* extra_tex, ID3D11ShaderResourceView* tex[], Light* light, FPCamera* camera, ShaderVariables* SVars, ID3D11ShaderResourceView* heightmap, bool trees = false);

	//QuadMesh TreeHere(ID3D11Device* device, ID3D11DeviceContext* deviceContext, XMFLOAT3 position);
private:
	void initShader(const wchar_t* vsFilename, const wchar_t* gsFilename, const wchar_t* psFilename);

	
};

