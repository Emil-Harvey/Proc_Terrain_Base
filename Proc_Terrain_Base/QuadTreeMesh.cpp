#include "QuadTreeMesh.h"

float QuadtreeNode::total_size = 0;
HeightmapType* QuadtreeNode::heightmap = nullptr;

QuadTreeMesh::~QuadTreeMesh()
{
	delete Root;
	Root = 0;
}

void QuadTreeMesh::render(ID3D11DeviceContext* dc, TerrainShader* shader, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, Frustum* viewFrustum, ID3D11ShaderResourceView** textures, Light* light, FPCamera* camera, ShaderVariables* SVars, ID3D11ShaderResourceView* heightmap)
{
    if (Root)
        Root->render(dc, shader, world, view, projection, viewFrustum, textures, light, camera, SVars, heightmap);
}

 QuadtreeNode::QuadtreeNode(ID3D11Device* d, ID3D11DeviceContext* dc, XMFLOAT2 pos, float size, XMFLOAT2* posOfDetail) {
    position = pos;
    this->size = size;

#ifndef CPU_TERRAIN_ENABLED
    const int resolution = 7;// has to be odd. also some numbers don't work, not sure why
    geometry = new TessellationTerrain(d, dc, heightmap, pos, size, total_size, posOfDetail, resolution);
#else
    const int resolution = 6;
    geometry = new TessellationPlane(d, dc, resolution);
#endif // CPU_TERRAIN_ENABLED

}

 QuadtreeNode::~QuadtreeNode() {
    //for (auto sn : subNodes)
    {
        //if(sn)
        //    delete sn;
    }
    /*if (subNodes) {
    delete subNodes;
    //subNodes = 0;
    }*/
    if (geometry) {
        delete geometry;
        geometry = nullptr;
    }
}

 void QuadtreeNode::subdivide(ID3D11Device* d, ID3D11DeviceContext* dc, XMFLOAT2 targetPosition, int depth, HeightmapType* height_map)
{
    delete geometry;// not a leaf node, so doesn't need geometry.
    geometry = nullptr;

    //QuadtreeIndex indexToSubdivide = getIndexOfPosition(targetPosition, position);
    for (int i = 0; i < 4; ++i)
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

        float weight = QuadTreeMesh::getWeightedIndexOfPosition(targetPosition, newPos, depth - 1);


        subNodes[i] = unique_ptr<QuadtreeNode> (new QuadtreeNode(d, dc, newPos, size / 2.f, &targetPosition));
        // if this node is not at
        if (depth > 0 && /*indexToSubdivide == i*/ weight + 5.0f < size / 2.f) {

            subNodes[i]->subdivide(d, dc, targetPosition, depth - 1);
        }

    }
}

 void QuadtreeNode::render(ID3D11DeviceContext* dc, TerrainShader* shader, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, Frustum* viewFrustum, ID3D11ShaderResourceView** textures, Light* light, FPCamera* camera, ShaderVariables* SVars, ID3D11ShaderResourceView* heightmap)
{

    if (viewFrustum->CheckCuboid(XMFLOAT3(position.x, 0.0, position.y), XMFLOAT3(size, 9999.9f, size))) {
        if (!isLeaf() )
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
            geometry->sendData(dc, D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);//
            shader->setShaderParameters(dc, XMMatrixMultiply(transformMatrix, world), view, projection, textures, light, camera, SVars, heightmap);
            shader->render(dc, geometry->getIndexCount());
            shader->unbind(dc);
        }
    }
}
