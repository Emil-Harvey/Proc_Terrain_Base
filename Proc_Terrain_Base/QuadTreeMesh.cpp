#include "QuadTreeMesh.h"

float QuadTreeMesh::QuadtreeNode::total_size = 0;
HeightmapType* QuadTreeMesh::QuadtreeNode::heightmap = nullptr;

QuadTreeMesh::~QuadTreeMesh()
{
	Root->~QuadtreeNode();
	delete Root;
	Root = 0;
}

void QuadTreeMesh::render(ID3D11DeviceContext* dc, TerrainShader* shader, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, Frustum* viewFrustum, ID3D11ShaderResourceView** textures, Light* light, FPCamera* camera, ShaderVariables* SVars, ID3D11ShaderResourceView* heightmap)
{
    if (Root)
        Root->render(dc, shader, world, view, projection, viewFrustum, textures, light, camera, SVars, heightmap);
}

