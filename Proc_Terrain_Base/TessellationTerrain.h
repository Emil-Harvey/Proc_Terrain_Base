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
enum Corner // AKA QuadtreeIndex
{
    // in binary order- if x>=0: 0, else: 1. same for y but in second digit. ergo +X,+Y = 00; -X,-Y = 11; etc.
    northeast = 0, // +X, +Y    | false, false; 00
    southeast = 1, // +X, -Y    | false, true;  01
    northwest = 2, // -X, +Y    | true, false;  10
    southwest = 3  // -X, -Y    | true, true;   11
};
#define isNorth(corner) (int(corner) & 1) != 1
#define isEast(corner) (int(corner) & 2) != 2
#define is_N_or_S(CompassDirection) (int(CompassDirection) & 1) == 1
#define is_S_or_E(CompassDirection) (int(CompassDirection) % 3) != 0
// reverse the binary of a 2 bit number: 2 (10) becomes 1 (01) etc
#define binReverse(b) ((b & 2) >> 1 | (b & 1) << 1)

class TessellationTerrain :
    public PlaneMesh
{
    enum Direction {
        NORTH = 0,
        EAST = 1,
        SOUTH = 2,
        WEST = 3
    };
    struct EdgeFlags {
        // true means, build edge at half resolution.
        // false means build edge at full resolution
        bool back = true;
        bool right = true;
        bool front = true;
        bool left = true;
    };

public:
    // pos: position of this mesh's centre, relative to HM edges [0-1].
    // size: size of this mesh relative to heightmap [0-1]
    TessellationTerrain(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HeightmapType* Heightmap, XMFLOAT2 pos, float size, float total_size, XMFLOAT2* PoD, int resolution = 2);
    //~TessellationTerrain();
    //void SetHeightmap(std::array<char, HEIGHTMAP_RES>* HeightmapType) { _heightmap = HeightmapType; }

protected:
    void initBuffers(ID3D11Device* device, XMFLOAT2 pos, float size, EdgeFlags EdgeLODTransitionFlags);
    static void sampleElevation(int startPixelU, int startPixelV, float _size, float& positionX, float& positionZ, float& elevation);
    
    static void BuildEdge(std::vector<VertexType>& vertices, std::vector<unsigned long>& indices, Direction dir, const int detail_difference, int startPixelU, int startPixelV, float _size, int &index);

    
    static inline int SamplePoint(const int& startPixelU, const int& startPixelV, const float& _size, const float& positionX, const float& positionZ)
    {
        return min(
            (startPixelU + (positionX * _size * HEIGHTMAP_DIM)) +
            (startPixelV + (positionZ * _size * HEIGHTMAP_DIM)) * HEIGHTMAP_DIM,
            HEIGHTMAP_DIM * HEIGHTMAP_DIM - 1);
    }
    static void RotateIndices(int& x, int& y, const Direction rotation)
    {
        // 'rotates' mesh indices so they fit with the side given, north is default
        switch (rotation)
        {
        case NORTH: // no rotation
        default:
            return;
        case EAST: // reflect diagonally
            swap(x, y);
            y = grid_resolution -1 - y;
            return;
        case SOUTH: // flip both
            x = grid_resolution -1 - x;
            y = grid_resolution -1 - y;
            return;
        case WEST: // reflect diagonally, flip horizontally
            swap(x, y);
            x = grid_resolution -1 - x;
            return;
        }
    }
    static Corner RotateCorner(Corner c, Direction rotation) {
        switch (rotation)
        {
        case NORTH:
        default:
            return c; // no rotation
        case EAST:
            if (c == 3) return northwest;
            return Corner( binReverse(int(c)) ^ 1 ); // reflect diagonally, flip
        case SOUTH:
            return Corner(int(c)^3); // flip both axes
        case WEST:
            if (c == 3) return southeast;
            return Corner( binReverse(int(c)) ^ 2 ); // reflect diagonally, flip horizontally
        }
    }
    static void AddVertex(const int& startPixelU, const int& startPixelV, const float& u, const float& v, const float& x, const float& y, const float& size, int& index, std::vector<VertexType>& vertices, std::vector<unsigned long>& indices, Corner corner) {
        float elevation = 0.5f;
        static const float increment = 1.0f / float(grid_resolution+1);
        static const float unit = 0.5;
        float positionX, positionZ;
        positionX = (float)(isEast(corner) ? x + unit : x - unit) / float(grid_resolution);
        positionZ = (float)(isNorth(corner)? y + unit : y - unit) / float(grid_resolution);      

        sampleElevation(startPixelU, startPixelV, size, positionX, positionZ, elevation);

        VertexType vert;
        vert.position = XMFLOAT3(positionX, elevation, positionZ);
        vert.texture = XMFLOAT2(isEast(corner) ? u + increment : u, isNorth(corner) ? v + increment : v);//vert.texture = XMFLOAT2(u + increment, v + increment);
        vert.normal = XMFLOAT3(0.0, 1.0, 0.0);
        vertices.push_back(vert);
        indices.push_back(index);
        index++;
    }

public:

protected:
    static int grid_resolution;// one less than the vertex resolution
    static HeightmapType* _heightmap;
    XMFLOAT2* _positionOfDetail;
    //*static*/ float total_size;
};

