#pragma once
#include "LightShader.h"
class TessShader :
    public LightShader
{
public:

    TessShader(ID3D11Device* device, HWND hwnd, bool includeGeoShader = false);
    void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* texture, Light* light,  Camera* camera, XMFLOAT4 TessellationDetails, float time);
protected:
    void initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* gsFilename, const wchar_t* psFilename);

    bool GeoShaderActive;
    ID3D11Buffer* geoBuffer;
};

