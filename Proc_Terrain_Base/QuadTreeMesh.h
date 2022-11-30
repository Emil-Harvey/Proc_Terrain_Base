#pragma once
/////////////////////////////////////////////////////////////////////////////
// QuadTreeMesh class is a dynamic LOD implementation with tesselation quads
// 
/////////////////////////////////////////////////////////////////////////////
#include "TessellationPlane.h"
#include "TessellationTerrain.h"
#include "TerrainShader.h"
#include "Frustrum.h"
#include <array>

// already defined //#define HEIGHTMAP_DIM 4096 //1600
//typedef std::array<uint8_t, HEIGHTMAP_DIM*HEIGHTMAP_DIM * 16> HeightmapType;
typedef Corner QuadtreeIndex;

///=====================<CLASS--QUADTREENODE>=====================================>
class QuadtreeNode {
    XMFLOAT2 position;
    float size;
    std::array<unique_ptr<QuadtreeNode>, 4> subNodes{};  //QuadtreeNode* subNodes[4] = { NULL };
    TessellationTerrain* geometry = nullptr;

public:
    static float total_size;
    static HeightmapType* heightmap;

public:

    QuadtreeNode(ID3D11Device* d, ID3D11DeviceContext* dc, XMFLOAT2 pos, float size, XMFLOAT2* posOfDetail = nullptr);
    ~QuadtreeNode();

    const XMFLOAT2 Position() { return position; }

    const std::array<unique_ptr<QuadtreeNode>, 4>* Nodes() { return &subNodes; } 

    const float Size() { return size; }

    void subdivide(ID3D11Device* d, ID3D11DeviceContext* dc, XMFLOAT2 targetPosition, int depth = 0, HeightmapType* height_map = nullptr);

    const bool isLeaf() { /*return subNodes == NULL;*/ return subNodes.empty() || subNodes[0] == nullptr; }//

    void render(ID3D11DeviceContext* dc, TerrainShader* shader, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, Frustum* viewFrustum, ID3D11ShaderResourceView** textures, Light* light, FPCamera* camera, ShaderVariables* SVars, ID3D11ShaderResourceView* heightmap);
};
///=====================</CLASS-QUADTREENODE>=====================================>

class QuadTreeMesh //: public TessellationMesh
{
    /// a mesh made of constituent tessellationplanes, that dynamically use a quadtree system
    ///Recursion
    /*
    enum QuadtreeIndex {
        // in binary order- if x>=0: 0, else: 1. same for y but in second digit. ergo +X,+Y = 00; -X,-Y = 11; etc.
        northeast = 0, // +X, +Y    | false, false; 00
        southeast = 1, // +X, -Y    | false, true;  01
        northwest = 2, // -X, +Y    | true, false;  10
        southwest = 3  // -X, -Y    | true, true;   11
    };*/


public:

    QuadTreeMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, XMFLOAT2 position, float size, int depth, XMFLOAT2 position_of_detail)
       //: TessellationMesh(device, deviceContext)
    {
        _heightmap = nullptr;
        QuadtreeNode::total_size = size;
        QuadtreeNode::heightmap = _heightmap;
        _size = size;
        _position = position;
        Root = new QuadtreeNode(device, deviceContext, position, size, &position_of_detail);
        Root->subdivide(device, deviceContext, position_of_detail, depth);
    }
    ~QuadTreeMesh();



private:
    QuadtreeNode* Root;/// 
    //int DivisionLevel = 0;

    //int minDivisions = 0;
    //int maxDivisions = 8;
    //maxQuads = 4^maxDivisions;
    XMFLOAT2 _position;
    float _size;
    HeightmapType* _heightmap = nullptr;

///------------------------------------------------------------

public:

    void Reconstruct(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int depth, XMFLOAT2 position_of_detail)
    {
        //delete Root;
        //Root = new QuadtreeNode(device, deviceContext, _position, _size);
        Root->subdivide(device, deviceContext, position_of_detail, depth, _heightmap);
    }

    void SetHeightmap(HeightmapType* Heightmap) {
        _heightmap = Heightmap;
        QuadtreeNode::heightmap = _heightmap;
    }
    
    QuadtreeNode* getRoot() { return Root; }

    void render(ID3D11DeviceContext* dc, TerrainShader* shader, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, Frustum* viewFrustum, ID3D11ShaderResourceView** textures, Light* light, FPCamera* camera, ShaderVariables* SVars, ID3D11ShaderResourceView* heightmap);

    static QuadtreeIndex getIndexOfPosition(XMFLOAT2 lookupPosition, XMFLOAT2 nodePosition)
    {
        int index = 0;
        // see definition of QuadtreeIndex
        index |= lookupPosition.x < nodePosition.x ? 2 : 0;
        index |= lookupPosition.y < nodePosition.y ? 1 : 0;

        return QuadtreeIndex(index);
    }
    static float getWeightedIndexOfPosition(XMFLOAT2 lookupPosition, XMFLOAT2 nodePosition, int depth, float radius = 5.0f)
    {
        // returns length between node position and lookup position
        XMVECTOR v = { lookupPosition.x - nodePosition.x,lookupPosition.y - nodePosition.y };
        return XMVector2Length(v).m128_f32[0];
    }

private:
    

};

