////////////////////////////////////////////////////////////////////////////////
// Filename: quadtreeclass.h
// with help from: https://www.rastertek.com/tertut05.html
// This class is a type of tesselation plane which is partitioned on creation, into 
// chunks of a given polygon-count. Not a realtime/dynamic LOD system.
////////////////////////////////////////////////////////////////////////////////
#pragma once

///For this tutorial we will use 10, 000 triangles per quad as the criteria for splitting nodes in the quad tree.
///It is defined here as a global for easy manipulation. Note that making this number too low will cause the tree 
/// to be incredibly more complex and hence will exponentially increase the time it takes to construct it.
/////////////
// GLOBALS //
/////////////
const int MAX_TRIANGLES = 12100;

///The QuadTreePlane will also need to interface with the TerrainClass, Frustum, and TerrainShaderClass so the headers for each are included here.
#include "PlaneMesh.h"
#include "TessellationPlane.h"
#include "DXF.h"
///#include "terrainclass.h"
///#include "frustumclass.h"
#include "TerrainShader.h"


////////////////////////////////////////////////////////////////////////////////
// Class name: QuadTreePlane
////////////////////////////////////////////////////////////////////////////////
class QuadTreePlane
{
private:

	/// The QuadTreePlane will require the same definition of the VertexType that is used in the TerrainClass since 
	/// it will be taking over the storing and rendering of the terrain vertex information.
	struct VertexType
	{
			
		XMFLOAT3 position;
		XMFLOAT2 texture;
		XMFLOAT3 normal;
	};

	///Each node in the quad tree will be defined as follows with position, size, triangle count, buffers, and four child nodes :
	struct Node
	{
		float positionX, positionZ, size;
		int triangleCount;
		ID3D11Buffer* vertexBuffer, * indexBuffer;
		Node* nodes[4];
	};

public:
	QuadTreePlane(ID3D11Device*);
	QuadTreePlane(const QuadTreePlane&);
	~QuadTreePlane();

	void CreateVertexArray(void* inout_VertexList);///fix/delete

	bool Initialize(/*TessellationPlane*,*/ ID3D11Device*);
	void Shutdown();
	void Render(/*Frustum*,*/ ID3D11DeviceContext*, TerrainShader*, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& proj, ID3D11ShaderResourceView** textures, Light*, FPCamera*, ShaderVariables*, ID3D11ShaderResourceView* );

	int GetDrawCount();

private:
	void CalculateMeshDimensions(int, float&, float&, float&);
	void CreateTreeNode(Node*, float, float, float, ID3D11Device*);
	int CountTriangles(float, float, float);
	bool IsTriangleContained(int, float, float, float);
	void ReleaseNode(Node*);
	void RenderNode(Node*, /*Frustum*,*/ ID3D11DeviceContext*, TerrainShader*, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& proj, ID3D11ShaderResourceView** textures, Light*, FPCamera*, ShaderVariables*, ID3D11ShaderResourceView* heightmap);

private:
	int m_triangleCount, m_drawCount;

	///The QuadTreePlane will require a list of the vertices from the TerrainClass object for building the quad tree.The list will be stored in the following array.
	VertexType* m_vertexList;

	///The parent node is the root of the quad tree.This single node will be expanded recursively to build the entire tree.
	Node* m_rootNode;
};

