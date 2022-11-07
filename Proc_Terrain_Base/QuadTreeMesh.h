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

#define HEIGHTMAP_DIM 1600
typedef std::array<XMFLOAT4, HEIGHTMAP_DIM* HEIGHTMAP_DIM> HeightmapType;

class QuadTreeMesh //: public TessellationMesh
{
    /// a mesh made of constituent tessellationplanes, that dynamically use a quadtree system
    ///Recursion
    struct Node {

    };
    enum QuadtreeIndex {
        // in binary order- if x>=0: 0, else: 1. same for y but in second digit. ergo +X,+Y = 00; -X,-Y = 11; etc.
        northeast = 0, // +X, +Y    | false, false; 00
        southeast = 1, // +X, -Y    | false, true;  01
        northwest = 2, // -X, +Y    | true, false;  10
        southwest = 3  // -X, -Y    | true, true;   11
    };
///=====================<CLASS--QUADTREENODE>=====================================>
    class QuadtreeNode {
        XMFLOAT2 position;
        float size;
        QuadtreeNode* subNodes[4] = { NULL };
        TessellationPlane* geometry = nullptr;

    public:
        static float total_size;
        static HeightmapType* heightmap;

    public:
        
        QuadtreeNode(ID3D11Device* d, ID3D11DeviceContext* dc, XMFLOAT2 pos, float size) {
            position = pos;
            this->size = size;
            const int resolution = 6;
            geometry = new TessellationTerrain(d, dc, heightmap, XMFLOAT2(pos.x/total_size,pos.y/total_size), size/total_size,  resolution);
            //geometry = new TessellationPlane(d, dc, resolution);
        }
        ~QuadtreeNode() {
            if (subNodes) {
                delete [] subNodes;
                //subNodes = 0;
            }
            if (geometry) {
                delete geometry;
                geometry = nullptr;
            }
        }
        
        XMFLOAT2 Position() { return position; }

        //QuadtreeNode* Nodes() { return subNodes; } Make Subnodes a vector? std::array?

        float Size() { return size; }

        void subdivide(ID3D11Device* d, ID3D11DeviceContext* dc, XMFLOAT2 targetPosition, int depth = 0, HeightmapType* height_map=nullptr)
        {
            delete geometry;// not a leaf node, so doesn't need geometry.
            geometry = nullptr;

            //QuadtreeIndex indexToSubdivide = getIndexOfPosition(targetPosition, position);
            for (int i=0; i< 4; ++i)
            {
                XMFLOAT2 newPos = position;
                // subnode 0 and 1 (00,01) will be the 'west' two- X < parent's center. 2 & 3 will have X > center
                if ((i & 2) == 2)
                    newPos.x -= size * 0.25f;
                else
                    newPos.x += size * 0.25f;
                // subnode 1 and 3 (01,11) will be the 'south' two- Y below parent's center. 0 & 2 will have Y above
                if ((i & 1) == 1)
                    newPos.y -= size * 0.25f;
                else
                    newPos.y += size * 0.25f;

                float weight = getWeightedIndexOfPosition(targetPosition, newPos, depth - 1);


                subNodes[i] = new QuadtreeNode(d, dc, newPos, size / 2.f);
                // if this node is not at
                if (depth > 0 && /*indexToSubdivide == i*/ weight+5.0f < size/2.f) {
                    
                    subNodes[i]->subdivide(d, dc, targetPosition, depth - 1);
                }
                
            }
        }

        bool isLeaf() { return subNodes == NULL; }//

        void render(ID3D11DeviceContext* dc, TerrainShader* shader, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, Frustum* viewFrustum, ID3D11ShaderResourceView** textures, Light* light, FPCamera* camera, ShaderVariables* SVars, ID3D11ShaderResourceView* heightmap)
        {

            if (viewFrustum->CheckCuboid(XMFLOAT3( position.x, 0.0, position.y), XMFLOAT3(size, 9999.9f, size))) {
                if (!(isLeaf() || subNodes[0] == NULL))
                {/// Recursively render all subnodes - only if they are leafs 
                    for (int i = 0; i < 4; i++) {
                        subNodes[i]->render(dc, shader, world, view, projection, viewFrustum, textures, light, camera, SVars, heightmap);
                    }
                }
                else
                {
                    const XMMATRIX positionMatrix = XMMatrixTranslation(position.x, 0.0, position.y);
                    const XMMATRIX scaleMatrix = XMMatrixScaling(size, 1.0, size);
                    const XMMATRIX transformMatrix = XMMatrixMultiply(scaleMatrix, positionMatrix);
                    geometry->sendData(dc, D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
                    shader->setShaderParameters(dc, XMMatrixMultiply(transformMatrix, world), view, projection, textures, light, camera, SVars, heightmap);
                    shader->render(dc, geometry->getIndexCount());
                    shader->unbind(dc);
                }
            }
        }
    };
///=====================</CLASS-QUADTREENODE>=====================================>

public:

    QuadTreeMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, XMFLOAT2 position, float size, int depth, XMFLOAT2 position_of_detail)
       //: TessellationMesh(device, deviceContext)
    {
        _heightmap = nullptr;
        QuadtreeNode::total_size = size;
        QuadtreeNode::heightmap = _heightmap;
        Root = new QuadtreeNode(device, deviceContext, position, size);
        Root->subdivide(device, deviceContext, position_of_detail, depth);
    }
    ~QuadTreeMesh();

    void Reconstruct(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int depth, XMFLOAT2 position_of_detail)
    {
        Root->subdivide(device, deviceContext, position_of_detail, depth, _heightmap);
    }

    void SetHeightmap(HeightmapType* Heightmap) {
        _heightmap = Heightmap;
        QuadtreeNode::heightmap = _heightmap;
    }

private:
    QuadtreeNode* Root;/// 
    //int DivisionLevel = 0;

    //int minDivisions = 0;
    //int maxDivisions = 8;
    //maxQuads = 4^maxDivisions;
    HeightmapType* _heightmap = nullptr;

///------------------------------------------------------------

public:
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
        int index = 0;
        XMVECTOR v = { lookupPosition.x - nodePosition.x,lookupPosition.y - nodePosition.y };
        return XMVector2Length(v).m128_f32[0];
        
    }

private:
    

};

