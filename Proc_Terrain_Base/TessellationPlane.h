#pragma once
#include "TessellationMesh.h"
class TessellationPlane :
    public TessellationMesh
{
public:
    TessellationPlane(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int resolution = 2);
    ~TessellationPlane();
protected:
    void initBuffers(ID3D11Device* device);
    int resolution;// minimum resolution - number of vertices along the side of the grid plane. must be >= 2 as there is an extra row of vertices for the final quad(s)
};
