#pragma once
#include "TessellationMesh.h"
class TessellationPlane :
    public TessellationMesh
{
public:
    TessellationPlane(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int resolution = 1);
    ~TessellationPlane();
protected:
    void initBuffers(ID3D11Device* device);
    int resolution;//   minimum resolution
};
