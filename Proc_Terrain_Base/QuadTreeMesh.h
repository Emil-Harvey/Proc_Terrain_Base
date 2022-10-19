#pragma once
/////////////////////////////////////////////////////////////////////////////
// QuadTreeMesh class is a dynamic LOD implementation with tesselation quads
// 
/////////////////////////////////////////////////////////////////////////////
#include "TessellationPlane.h"
#include "TerrainShader.h"

class QuadTreeMesh :
    public TessellationMesh
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
    class QuadtreeNode {
        XMFLOAT2 position;
        float size;
        QuadtreeNode* subNodes[4] = { NULL };
        TessellationPlane* geometry = nullptr;

    public:
        /*QuadtreeNode() {position = { 0,0 };this->size = 0;}*/
        QuadtreeNode(ID3D11Device* d, ID3D11DeviceContext* dc, XMFLOAT2 pos, float size) {
            position = pos;
            this->size = size;
            const int resolution = 2;
            geometry = new TessellationPlane(d, dc, resolution);
        }
        ~QuadtreeNode() {
            if (subNodes) {
                delete subNodes;
                //subNodes = 0;
            }
            if (geometry) {
                delete geometry;
                geometry = 0;
            }
        }
        
        XMFLOAT2 Position() { return position; }

        //QuadtreeNode* Nodes() { return subNodes; } Make Subnodes a vector? std::array?

        float Size() { return size; }

        void subdivide(ID3D11Device* d, ID3D11DeviceContext* dc, XMFLOAT2 targetPosition, int depth = 0)
        {
            //subNodes = new QuadtreeNode[4];//?
            QuadtreeIndex indexToSubdivide = getIndexOfPosition(targetPosition, position);
            for (int i=0; i< 4; ++i)
            {
                XMFLOAT2 newPos = position;
                // subnode 0 and 1 (00,01) will be the 'west' two- X < parent's center. 2 & 3 will have X > center
                if ((i & 2) == 2)
                    newPos.x -= size * 0.25;
                else
                    newPos.x += size * 0.25;
                // subnode 1 and 3 (01,11) will be the 'south' two- Y below parent's center. 0 & 2 will have Y above
                if ((i & 1) == 1)
                    newPos.y -= size * 0.25;
                else
                    newPos.y += size * 0.25;

                subNodes[i] = new QuadtreeNode(d, dc, newPos, size / 2.f);
                if (depth > 0 && indexToSubdivide == i)
                    subNodes[i]->subdivide(d,dc, targetPosition, depth - 1);
            }
        }

        bool isLeaf() { return subNodes == NULL; }//

        void render(ID3D11DeviceContext* dc, TerrainShader* shader, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView** textures, Light* light, FPCamera* camera, ShaderVariables* SVars, ID3D11ShaderResourceView* heightmap)
        {
            if (!(isLeaf() || subNodes[0] == NULL))
            {/// Recursively render all subnodes - only if they are leafs 
                for (int i = 0; i < 4; i++) {
                    subNodes[i]->render(dc, shader, world, view, projection, textures, light, camera, SVars, heightmap);
                }
            }
            else
            {
                const XMMATRIX positionMatrix = XMMatrixTranslation(position.x, 0.0, position.y);
                const XMMATRIX scaleMatrix = XMMatrixScaling(size, 1.0, size);
                const XMMATRIX transformMatrix = XMMatrixMultiply(scaleMatrix, positionMatrix);
                geometry->sendData(dc, D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
                shader->setShaderParameters(dc, XMMatrixMultiply(transformMatrix,world), view, projection, textures, light,  camera, SVars, heightmap);
                shader->render(dc, geometry->getIndexCount());
            }
        }
    };
///============================================================

public:

    //QuadTreeMesh(ID3D11Device*, ID3D11DeviceContext*, int sivisLevel = 0);
    QuadTreeMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, XMFLOAT2 position, float size, int depth, XMFLOAT2 position_of_detail)
        : TessellationMesh(device, deviceContext)
    {
        Root = new QuadtreeNode(device, deviceContext, position, size);
        Root->subdivide(device, deviceContext, position_of_detail, depth);
    }
    ~QuadTreeMesh();

private:
    QuadtreeNode* Root;/// 
    int DivisionLevel = 0;

    int minDivisions = 0;
    int maxDivisions = 8;
    //maxQuads = 4^maxDivisions;

    //XMFLOAT3* POD; // Position of Detail

///------------------------------------------------------------

public:
    QuadtreeNode* getRoot() {
        return Root;
    }
    void render(ID3D11DeviceContext* dc, TerrainShader* shader, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView** textures, Light* light, FPCamera* camera, ShaderVariables* SVars, ID3D11ShaderResourceView* heightmap)
    {

        if (Root)
            Root->render(dc, shader, world, view, projection, textures, light, camera, SVars, heightmap);
    }

    static QuadtreeIndex getIndexOfPosition(XMFLOAT2 lookupPosition, XMFLOAT2 nodePosition)
    {
        int index = 0;
        // see definition of QuadtreeIndex
        index |= lookupPosition.x < nodePosition.x ? 2 : 0;
        index |= lookupPosition.y < nodePosition.y ? 1 : 0;

        return QuadtreeIndex(index);
    }

private:
    

};

