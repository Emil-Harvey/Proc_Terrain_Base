#pragma once
#include "TessellationPlane.h"
class QuadTreeMesh :
    public TessellationMesh
{
    /// a mesh made of constituent tessellationplanes, that dynamically use a quadtree system
    ///Recursion
    
    QuadTreeMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int sivisLevel = 0);
    ~QuadTreeMesh();

    QuadTreeMesh* Root;/// nevermind
    int DivisionLevel = 0;

    int minDivisions = 0;
    int maxDivisions = 8;

    XMFLOAT3* POD; // Position of Detail

};

