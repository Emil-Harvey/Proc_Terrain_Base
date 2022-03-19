#pragma once
#include "DepthShader.h"
class TnDepthShader :
    public DepthShader
{
public:
    struct CameraBufferType
    {
        XMFLOAT3 position;
        float padding;
    };

    TnDepthShader(ID3D11Device* device, HWND hwnd);
    void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* displacementMap, XMFLOAT4 TessellationDetails, Camera* camera);

protected:
    void initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* psFilename);

    ID3D11Buffer* cameraBuffer;
};
