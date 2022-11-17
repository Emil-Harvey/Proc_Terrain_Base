#pragma once
#include "PlaneMesh.h"
#include <array>
#include <algorithm>
#include <vector>

using std::swap;
#define HEIGHTMAP_DIM 1600
typedef std::array<XMFLOAT4, HEIGHTMAP_DIM*HEIGHTMAP_DIM> HeightmapType;
template <class Type>
static Type* ArrayFromVector(const std::vector< Type >& v)
{
    Type* result = new Type[v.size()];

    memcpy(result, &v.front(), v.size() * sizeof(Type));

    return result;
}

class TessellationTerrain :
    public PlaneMesh
{
    struct EdgeFlags {
        bool left = true,
            right = true,
            front = true,
            back = true;
    };
    enum Direction {
        NORTH = 0,
        EAST = 1,
        SOUTH = 2,
        WEST = 3
    };
    #define GRID_RESOLUTION (resolution - 1) // one less than the vertex resolution

public:
    // pos: position of this mesh's centre, relative to HM edges [0-1].
    // size: size of this mesh relative to heightmap [0-1]
    TessellationTerrain(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HeightmapType* Heightmap, XMFLOAT2 pos, float size, float total_size, XMFLOAT2* PoD, int resolution = 2);
    //void SetHeightmap(std::array<char, HEIGHTMAP_RES>* HeightmapType) { _heightmap = HeightmapType; }

protected:
    void initBuffers(ID3D11Device* device, XMFLOAT2 pos, float size, EdgeFlags flags);
    bool sampleElevation(int startPixelU, int startPixelV, float _size, float& positionX, float& positionZ, float& elevation, EdgeFlags flags);
    
    static void BuildEdge(std::vector<VertexType> &vertices, Direction dir, const int detail_difference);

    
    static inline int SamplePoint(const int& startPixelU, const int& startPixelV, const float& _size, const float& positionX, const float& positionZ)
    {
        return min(
            (startPixelU + (positionX * _size * HEIGHTMAP_DIM)) +
            (startPixelV + (positionZ * _size * HEIGHTMAP_DIM)) * HEIGHTMAP_DIM,
            HEIGHTMAP_DIM * HEIGHTMAP_DIM - 1);
    }
    /*static void RotateIndices(int& x, int& y, const Direction rotation)
    {
        // 'rotates' mesh indices so they fit with the side given, north is default
        switch (rotation)
        {
        case NORTH:
        default:
            return;
        case EAST:
            swap(x, y);
            y = GRID_RESOLUTION - y;
            return;
        case SOUTH:
            x = GRID_RESOLUTION - x;
            y = GRID_RESOLUTION - y;
            return;
        case WEST:
            swap(x, y);
            x = GRID_RESOLUTION - x;
            return;
        }
    }*/

public:
    

protected:
    HeightmapType* _heightmap;
    XMFLOAT2* _positionOfDetail;
    //*static*/ float total_size;
};

