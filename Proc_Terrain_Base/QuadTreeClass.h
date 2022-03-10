////////////////////////////////////////////////////////////////////////////////
// Filename: quadtreeclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _QUADTREECLASS_H_
#define _QUADTREECLASS_H_
///For this tutorial we will use 10, 000 triangles per quad as the criteria for splitting nodes in the quad tree.It is defined here as a global for easy manipulation.Note that making this number too low will cause the tree to be incredibly more complexand hence will exponentially increase the time it takes to construct it.

/////////////
// GLOBALS //
/////////////
const int MAX_TRIANGLES = 10000;
///The QuadTreeClass will also need to interface with the TerrainClass, FrustumClass, and TerrainShaderClass so the headers for each are included here.

///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "PlaneMesh.h"
#include "DXF.h"
///#include "terrainclass.h"
///#include "frustumclass.h"
///#include "terrainshaderclass.h"


////////////////////////////////////////////////////////////////////////////////
// Class name: QuadTreeClass
////////////////////////////////////////////////////////////////////////////////
class QuadTreeClass
{
private:
	///The QuadTreeClass will require the same definition of the VertexType that is used in the TerrainClass since it will be taking over the storingand rendering of the terrain vertex information.

		struct VertexType
	{
			
		XMFLOAT3 position;
		XMFLOAT2 texture;
		XMFLOAT3 normal;
	};
	///Each node in the quad tree will be defined as follows with position, size, triangle count, buffers, and four child nodes :

	struct NodeType
	{
		float positionX, positionZ, width;
		int triangleCount;
		ID3D11Buffer* vertexBuffer, * indexBuffer;
		NodeType* nodes[4];
	};

public:
	QuadTreeClass();
	QuadTreeClass(const QuadTreeClass&);
	~QuadTreeClass();

	///bool Initialize(TerrainClass*, ID3D11Device*);
	void Shutdown();
	///void Render(FrustumClass*, ID3D11DeviceContext*, TerrainShaderClass*);

	int GetDrawCount();

private:
	void CalculateMeshDimensions(int, float&, float&, float&);
	void CreateTreeNode(NodeType*, float, float, float, ID3D11Device*);
	int CountTriangles(float, float, float);
	bool IsTriangleContained(int, float, float, float);
	void ReleaseNode(NodeType*);
	///void RenderNode(NodeType*, FrustumClass*, ID3D11DeviceContext*, TerrainShaderClass*);

private:
	int m_triangleCount, m_drawCount;
	///The QuadTreeClass will require a list of the vertices from the TerrainClass object for building the quad tree.The list will be stored in the following array.

		VertexType* m_vertexList;
	///The parent node is the root of the quad tree.This single node will be expanded recursively to build the entire tree.

		NodeType* m_parentNode;
};

#endif

