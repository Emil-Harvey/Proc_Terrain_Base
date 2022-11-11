#pragma once
#include "TessellationPlane.h"
#include <array>

#define HEIGHTMAP_DIM 1600
typedef std::array<XMFLOAT4, HEIGHTMAP_DIM*HEIGHTMAP_DIM> HeightmapType;


class TessellationTerrain :
    public TessellationPlane
{
    struct EdgeFlags {
        bool left = true,
            right = true,
            front = true,
            back = true;
    };

public:
    // pos: position of this mesh's centre, relative to HM edges [0-1].
    // size: size of this mesh relative to heightmap [0-1]
    TessellationTerrain(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HeightmapType* Heightmap, XMFLOAT2 pos, float size, float total_size, XMFLOAT2* PoD, int resolution = 2);
    //void SetHeightmap(std::array<char, HEIGHTMAP_RES>* HeightmapType) { _heightmap = HeightmapType; }
protected:
    void initBuffers(ID3D11Device* device, XMFLOAT2 pos, float size, EdgeFlags flags);
    void sampleElevation(int startPixelU, int startPixelV, float _size, float& positionX, float& positionZ, int rmo, float& elevation, int& samplePoint, EdgeFlags flags);
    static inline int SamplePoint(const int& startPixelU, const int& startPixelV, const float& _size, const float& positionX, const float& positionZ)
    {
        return min(
            (startPixelU + (positionX * _size * HEIGHTMAP_DIM)) +
            (startPixelV + (positionZ * _size * HEIGHTMAP_DIM)) * HEIGHTMAP_DIM,
            HEIGHTMAP_DIM * HEIGHTMAP_DIM - 1);
    }

public:
    

protected:
    HeightmapType* _heightmap;
    XMFLOAT2* _positionOfDetail;
    //*static*/ float total_size;
};

