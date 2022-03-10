////////////////////////////////////////////////////////////////////////////////
// Filename: quadtreeclass.h
////////////////////////////////////////////////////////////////////////////////
#include "quadtreeclass.h"
///The class constructor initializes the vertex listand quad tree parent node to null.

QuadTreeClass::QuadTreeClass()
{
	m_vertexList = 0;
	m_parentNode = 0;
}


QuadTreeClass::QuadTreeClass(const QuadTreeClass& other)
{
}


QuadTreeClass::~QuadTreeClass()
{
}
/*

bool QuadTreeClass::Initialize(TerrainClass* terrain, ID3D11Device* device)
{
	int vertexCount;
	float centerX, centerZ, width;
	The first thing the QuadTreeClass has to do is get all the information it will need from the TerrainClass.It first gets the number of vertices in the terrain, then it uses that value to create a vertex list which is then sent into the TerrainClassand filled with the terrain vertex information.

		// Get the number of vertices in the terrain vertex array.
		vertexCount = terrain->GetVertexCount();

	// Store the total triangle count for the vertex list.
	m_triangleCount = vertexCount / 3;

	// Create a vertex array to hold all of the terrain vertices.
	m_vertexList = new VertexType[vertexCount];
	if (!m_vertexList)
	{
		return false;
	}

	// Copy the terrain vertices into the vertex list.
	terrain->CopyVertexArray((void*)m_vertexList);
	///Once the vertex list is filled with the terrain information it can calculate the dimensions of the parent nodeand start the recursive method of building the quad tree.

		// Calculate the center x,z and the width of the mesh.
		CalculateMeshDimensions(vertexCount, centerX, centerZ, width);

	// Create the parent node for the quad tree.
	m_parentNode = new NodeType;
	if (!m_parentNode)
	{
		return false;
	}

	// Recursively build the quad tree based on the vertex list data and mesh dimensions.
	CreateTreeNode(m_parentNode, centerX, centerZ, width, device);
	///Once the quad tree is built the vertex list is no longer needed.

		// Release the vertex list since the quad tree now has the vertices in each node.
		if (m_vertexList)
		{
			delete[] m_vertexList;
			m_vertexList = 0;
		}

	return true;
}


void QuadTreeClass::Shutdown()
{
	Shutdown calls the ReleaseNode function which recursively traces down the treeand removes all the nodes.

		// Recursively release the quad tree data.
		if (m_parentNode)
		{
			ReleaseNode(m_parentNode);
			delete m_parentNode;
			m_parentNode = 0;
		}

	return;
}
///The Render function calls RenderNode which goes through the treeand renders nodes using the frustum object which determines the node visibility.The m_drawCount variable must be initialized to zero before the rendering is done as it will be incremented for each node drawn for all the triangles in each node.

void QuadTreeClass::Render(FrustumClass * frustum, ID3D11DeviceContext * deviceContext, TerrainShaderClass * shader)
{
	// Reset the number of triangles that are drawn for this frame.
	m_drawCount = 0;

	// Render each node that is visible starting at the parent node and moving down the tree.
	RenderNode(m_parentNode, frustum, deviceContext, shader);

	return;
}
///GetDrawCount returns the total number of triangles that were drawn in the previous Render function call.

int QuadTreeClass::GetDrawCount()
{
	return m_drawCount;
}
///The CalculateMeshDimensions function determines the physical quad size of the parent node.It goes through all the vertices in the terrain vertex listand uses them to calculate the center of the terrain as well as the absolute maximum width of the terrain.These values will then later be used to create the size of the parent node.

void QuadTreeClass::CalculateMeshDimensions(int vertexCount, float& centerX, float& centerZ, float& meshWidth)
{
	int i;
	float maxWidth, maxDepth, minWidth, minDepth, width, depth, maxX, maxZ;


	// Initialize the center position of the mesh to zero.
	centerX = 0.0f;
	centerZ = 0.0f;

	// Sum all the vertices in the mesh.
	for (i = 0; i < vertexCount; i++)
	{
		centerX += m_vertexList[i].position.x;
		centerZ += m_vertexList[i].position.z;
	}

	// And then divide it by the number of vertices to find the mid-point of the mesh.
	centerX = centerX / (float)vertexCount;
	centerZ = centerZ / (float)vertexCount;

	// Initialize the maximum and minimum size of the mesh.
	maxWidth = 0.0f;
	maxDepth = 0.0f;

	minWidth = fabsf(m_vertexList[0].position.x - centerX);
	minDepth = fabsf(m_vertexList[0].position.z - centerZ);

	// Go through all the vertices and find the maximum and minimum width and depth of the mesh.
	for (i = 0; i < vertexCount; i++)
	{
		width = fabsf(m_vertexList[i].position.x - centerX);
		depth = fabsf(m_vertexList[i].position.z - centerZ);

		if (width > maxWidth) { maxWidth = width; }
		if (depth > maxDepth) { maxDepth = depth; }
		if (width < minWidth) { minWidth = width; }
		if (depth < minDepth) { minDepth = depth; }
	}

	// Find the absolute maximum value between the min and max depth and width.
	maxX = (float)max(fabs(minWidth), fabs(maxWidth));
	maxZ = (float)max(fabs(minDepth), fabs(maxDepth));

	// Calculate the maximum diameter of the mesh.
	meshWidth = max(maxX, maxZ) * 2.0f;

	return;
}
///CreateTreeNode is the function that builds the quad tree.Note that it is recursiveand will call itself numerous times.It starts with the parent nodeand then calls itself for each child node, and for each child node it calls itself for their children nodesand so forth.It builds the entire node treeand at the same time it loads the vertex data into each bottom child node.

void QuadTreeClass::CreateTreeNode(NodeType * node, float positionX, float positionZ, float width, ID3D11Device * device)
{
	int numTriangles, i, count, vertexCount, index, vertexIndex;
	float offsetX, offsetZ;
	VertexType* vertices;
	unsigned long* indices;
	bool result;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	///First initialize the nodeand set its position in the world.

		// Store the node position and size.
		node->positionX = positionX;
	node->positionZ = positionZ;
	node->width = width;

	// Initialize the triangle count to zero for the node.
	node->triangleCount = 0;

	// Initialize the vertex and index buffer to null.
	node->vertexBuffer = 0;
	node->indexBuffer = 0;

	// Initialize the children nodes of this node to null.
	node->nodes[0] = 0;
	node->nodes[1] = 0;
	node->nodes[2] = 0;
	node->nodes[3] = 0;
	Then count the number of triangles that are in the dimensions of this node from the terrain.

		// Count the number of triangles that are inside this node.
		numTriangles = CountTriangles(positionX, positionZ, width);
	Case 1: If there are no triangles in this node then this part of the tree is complete.

		// Case 1: If there are no triangles in this node then return as it is empty and requires no processing.
		if (numTriangles == 0)
		{
			return;
		}
	Case 2: If there are too many triangles inside this node then it gets split into four new quads / nodes.

		// Case 2: If there are too many triangles in this node then split it into four equal sized smaller tree nodes.
		if (numTriangles > MAX_TRIANGLES)
		{
			for (i = 0; i < 4; i++)
			{
				// Calculate the position offsets for the new child node.
				offsetX = (((i % 2) < 1) ? -1.0f : 1.0f) * (width / 4.0f);
				offsetZ = (((i % 4) < 2) ? -1.0f : 1.0f) * (width / 4.0f);

				// See if there are any triangles in the new node.
				count = CountTriangles((positionX + offsetX), (positionZ + offsetZ), (width / 2.0f));
				if (count > 0)
				{
					// If there are triangles inside where this new node would be then create the child node.
					node->nodes[i] = new NodeType;

					// Extend the tree starting from this new child node now.
					CreateTreeNode(node->nodes[i], (positionX + offsetX), (positionZ + offsetZ), (width / 2.0f), device);
				}
			}

			return;
		}
	Case 3: If there are the right number of triangles then create and load the vertex and index buffer from the terrain list into this node.We have also determined that this must be a bottom child node.

		// Case 3: If this node is not empty and the triangle count for it is less than the max then 
		// this node is at the bottom of the tree so create the list of triangles to store in it.
		node->triangleCount = numTriangles;

	// Calculate the number of vertices.
	vertexCount = numTriangles * 3;

	// Create the vertex array.
	vertices = new VertexType[vertexCount];

	// Create the index array.
	indices = new unsigned long[vertexCount];

	// Initialize the index for this new vertex and index array.
	index = 0;

	// Go through all the triangles in the vertex list.
	for (i = 0; i < m_triangleCount; i++)
	{
		// If the triangle is inside this node then add it to the vertex array.
		result = IsTriangleContained(i, positionX, positionZ, width);
		if (result == true)
		{
			// Calculate the index into the terrain vertex list.
			vertexIndex = i * 3;

			// Get the three vertices of this triangle from the vertex list.
			vertices[index].position = m_vertexList[vertexIndex].position;
			vertices[index].texture = m_vertexList[vertexIndex].texture;
			vertices[index].normal = m_vertexList[vertexIndex].normal;
			indices[index] = index;
			index++;

			vertexIndex++;
			vertices[index].position = m_vertexList[vertexIndex].position;
			vertices[index].texture = m_vertexList[vertexIndex].texture;
			vertices[index].normal = m_vertexList[vertexIndex].normal;
			indices[index] = index;
			index++;

			vertexIndex++;
			vertices[index].position = m_vertexList[vertexIndex].position;
			vertices[index].texture = m_vertexList[vertexIndex].texture;
			vertices[index].normal = m_vertexList[vertexIndex].normal;
			indices[index] = index;
			index++;
		}
	}

	// Set up the description of the vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now finally create the vertex buffer.
	device->CreateBuffer(&vertexBufferDesc, &vertexData, &node->vertexBuffer);

	// Set up the description of the index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * vertexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	device->CreateBuffer(&indexBufferDesc, &indexData, &node->indexBuffer);

	// Release the vertex and index arrays now that the data is stored in the buffers in the node.
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;

	return;
}
///The CountTriangles function goes through the list of triangles from the terrain dataand determines which ones are inside the dimensions that are given as input.

int QuadTreeClass::CountTriangles(float positionX, float positionZ, float width)
{
	int count, i;
	bool result;


	// Initialize the count to zero.
	count = 0;

	// Go through all the triangles in the entire mesh and check which ones should be inside this node.
	for (i = 0; i < m_triangleCount; i++)
	{
		// If the triangle is inside the node then increment the count by one.
		result = IsTriangleContained(i, positionX, positionZ, width);
		if (result == true)
		{
			count++;
		}
	}

	return count;
}
///IsTriangleContained calculates if the given triangle is completely inside the input cube dimensions or not.

bool QuadTreeClass::IsTriangleContained(int index, float positionX, float positionZ, float width)
{
	float radius;
	int vertexIndex;
	float x1, z1, x2, z2, x3, z3;
	float minimumX, maximumX, minimumZ, maximumZ;


	// Calculate the radius of this node.
	radius = width / 2.0f;

	// Get the index into the vertex list.
	vertexIndex = index * 3;

	// Get the three vertices of this triangle from the vertex list.
	x1 = m_vertexList[vertexIndex].position.x;
	z1 = m_vertexList[vertexIndex].position.z;
	vertexIndex++;

	x2 = m_vertexList[vertexIndex].position.x;
	z2 = m_vertexList[vertexIndex].position.z;
	vertexIndex++;

	x3 = m_vertexList[vertexIndex].position.x;
	z3 = m_vertexList[vertexIndex].position.z;

	// Check to see if the minimum of the x coordinates of the triangle is inside the node.
	minimumX = min(x1, min(x2, x3));
	if (minimumX > (positionX + radius))
	{
		return false;
	}

	// Check to see if the maximum of the x coordinates of the triangle is inside the node.
	maximumX = max(x1, max(x2, x3));
	if (maximumX < (positionX - radius))
	{
		return false;
	}

	// Check to see if the minimum of the z coordinates of the triangle is inside the node.
	minimumZ = min(z1, min(z2, z3));
	if (minimumZ > (positionZ + radius))
	{
		return false;
	}

	// Check to see if the maximum of the z coordinates of the triangle is inside the node.
	maximumZ = max(z1, max(z2, z3));
	if (maximumZ < (positionZ - radius))
	{
		return false;
	}

	return true;
}
///The ReleaseNode function is used for releasing all the nodes in the quad tree as well as the data inside each node.The function is recursiveand will call itself to traverse the entire node tree.

void QuadTreeClass::ReleaseNode(NodeType * node)
{
	int i;


	// Recursively go down the tree and release the bottom nodes first.
	for (i = 0; i < 4; i++)
	{
		if (node->nodes[i] != 0)
		{
			ReleaseNode(node->nodes[i]);
		}
	}

	// Release the vertex buffer for this node.
	if (node->vertexBuffer)
	{
		node->vertexBuffer->Release();
		node->vertexBuffer = 0;
	}

	// Release the index buffer for this node.
	if (node->indexBuffer)
	{
		node->indexBuffer->Release();
		node->indexBuffer = 0;
	}

	// Release the four child nodes.
	for (i = 0; i < 4; i++)
	{
		if (node->nodes[i])
		{
			delete node->nodes[i];
			node->nodes[i] = 0;
		}
	}

	return;
}
///RenderNode is responsible for drawing all the visible nodes in the quad tree.It takes as input the frustum object which it uses to check if the user can view each quad or not.It also takes the shader that will be used to render each node, for this tutorial the shader is the TerrainShaderClass object.Just like the other functions this is also recursiveand calls itself for all the child nodes it can see.

void QuadTreeClass::RenderNode(NodeType * node, FrustumClass * frustum, ID3D11DeviceContext * deviceContext, TerrainShaderClass * shader)
{
	bool result;
	int count, i, indexCount;
	unsigned int stride, offset;
	Start by doing a frustum check on the cube to see if it is visible or not.

		// Check to see if the node can be viewed, height doesn't matter in a quad tree.
		result = frustum->CheckCube(node->positionX, 0.0f, node->positionZ, (node->width / 2.0f));

	// If it can't be seen then none of its children can either so don't continue down the tree, this is where the speed is gained.
	if (!result)
	{
		return;
	}
	If this node can be seen then recursively call this same function for each of the child nodes.

		// If it can be seen then check all four child nodes to see if they can also be seen.
		count = 0;
	for (i = 0; i < 4; i++)
	{
		if (node->nodes[i] != 0)
		{
			count++;
			RenderNode(node->nodes[i], frustum, deviceContext, shader);
		}
	}

	// If there were any children nodes then there is no need to continue as parent nodes won't contain any triangles to render.
	if (count != 0)
	{
		return;
	}
	Render the buffers in this node as normal if they can be seen using the terrain shader.

		// Otherwise if this node can be seen and has triangles in it then render these triangles.

		// Set vertex buffer stride and offset.
		stride = sizeof(VertexType);
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &node->vertexBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(node->indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Determine the number of indices in this node.
	indexCount = node->triangleCount * 3;

	// Call the terrain shader to render the polygons in this node.
	shader->RenderShader(deviceContext, indexCount);
	///Increment the count of triangles that have been drawn.

		// Increase the count of the number of polygons that have been rendered during this frame.
		m_drawCount += node->triangleCount;

	return;
}
*/