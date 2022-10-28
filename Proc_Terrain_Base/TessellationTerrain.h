#pragma once
#include "TessellationPlane.h"
#include <array>

#define HEIGHTMAP_DIM 1600
typedef std::array<XMFLOAT4, HEIGHTMAP_DIM*HEIGHTMAP_DIM> HeightmapType;


class TessellationTerrain :
    public TessellationPlane
{


public:
    // pos: position of this mesh's centre, relative to HM edges [0-1].
    // size: size of this mesh relative to heightmap [0-1]
    TessellationTerrain(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HeightmapType* Heightmap, XMFLOAT2 pos, float size, int resolution = 2);
    //void SetHeightmap(std::array<char, HEIGHTMAP_RES>* HeightmapType) { _heightmap = HeightmapType; }
protected:
    void initBuffers(ID3D11Device* device, XMFLOAT2 pos, float size);

public:
    

protected:
    HeightmapType* _heightmap;
};

