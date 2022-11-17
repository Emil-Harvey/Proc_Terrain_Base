#include "TessellationTerrain.h"


TessellationTerrain::TessellationTerrain(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HeightmapType* Heightmap, XMFLOAT2 pos, float size, float total_size, XMFLOAT2* PositionOfDetail, int resolution)
	: PlaneMesh(device, deviceContext, resolution)
{
	assert(resolution > 4);

	_heightmap = Heightmap;
	_positionOfDetail = PositionOfDetail;

	// sample at half resolution on edges that border bigger quads in the quad tree, to avoid seams. (i'll call this sub-sampling)
	// these edges will always face away from the "position of detail" (ie player/camera).
	// this isn't necessary on the largest quads, that don't border anything larger/lower res (ie. their outer edges are borders to the void)
		/// work out which edges need to be sub-sampled
	// Let x1 & x2 be the east & west edges; y1 & y2 north & south.
	// if pos < x1 < x2: then subsample x2.
	// if x1 < pos < x2: then do not subsample x1 or x2.
	// same logic applies in other direction, and in Y axis
	float left_edge  = pos.x + (size * -0.5);
	float right_edge = pos.x + (size * +0.5);
	float front_edge = pos.y + (size * -0.5);
	float back_edge  = pos.y + (size * +0.5);
	EdgeFlags edges;
	if (_positionOfDetail->x < left_edge)
		edges.left = false;
	else if (_positionOfDetail->x > right_edge)
		edges.right = false;
	else
		edges.left = false, edges.right = false;
	if (_positionOfDetail->y < front_edge)
		edges.front = false;
	else if (_positionOfDetail->y > back_edge)
		edges.back = false;
	else
		edges.front = false, edges.back = false;

	initBuffers(device, XMFLOAT2(pos.x / total_size, pos.y / total_size), size / total_size, edges );
}



// Identical to TessellationPlane, however the vertices are displaced by a heightmap, and potential seams made by a QuadTree are sorted
void TessellationTerrain::initBuffers(ID3D11Device* device, XMFLOAT2 _pos, float _size, EdgeFlags flags)
{
	std::vector<VertexType> vertices;//VertexType* vertices;
	std::vector<unsigned long> indices;//unsigned long* indices
	int index;//

	// positionX/Z represents the position of this vertex relative to this whole mesh [0-1]
	float positionX, positionZ, u, v, increment;//
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	// Calculate the number of vertices in the terrain mesh.
	//vertexCount = (resolution - 1) * (resolution - 1) * 6;//
	//indexCount = vertexCount;//

	//vertices = new VertexType[vertexCount];
	//indices = new unsigned long[indexCount];

	int startPixelU = (_pos.x + 0.5) * HEIGHTMAP_DIM;
	int startPixelV = (_pos.y + 0.5) * HEIGHTMAP_DIM;
	int samplePoint;
	float elevation = 90.0f; // default to a value that stands out when debugging

	

	index = 0;//
	// UV coords.
	u = 0;//
	v = 0;//
	increment = 1.0f / resolution;//
	const float resMinusOne = resolution - 1.0f; 
	const float unit = 0.5;
	for (int j = 1; j < (resolution - 2); j++)
	{
		v = increment * j;

		for (int i = 1; i < (resolution - 2); i++)
		{
			u = increment * i;
			index = (i * 6) + (j * resMinusOne * 6);

			// position of this quad. the vertices should be 0.5 units in each direction from this point
			// x and y shift i & j from 0 - resolution to ~(-resolution/2) - (+resolution/2)
			// however positionX & positionZ are values btwn -0.5 & 0.5
			const float x = i - unit * (resolution - 2);
			const float y = j - unit * (resolution - 2);

			///tri #1: ◸
			// lower left																										   
			positionX = (float)(x - unit) / resMinusOne;
			positionZ = (float)(y - unit) / resMinusOne;

			sampleElevation(startPixelU, startPixelV, _size, positionX, positionZ, elevation, flags);

			vertices[index].position = XMFLOAT3(positionX, elevation, positionZ);
			vertices[index].texture = XMFLOAT2(u, v);
			vertices[index].normal = XMFLOAT3(0.0, 1.0, 0.0);
			indices[index] = index;
			index++;								

			// Upper right.
			positionX = (float)(x + unit) / resMinusOne;
			positionZ = (float)(y + unit) / resMinusOne;

			sampleElevation(startPixelU, startPixelV, _size, positionX, positionZ, elevation, flags);
			
			vertices[index].position = XMFLOAT3(positionX, elevation, positionZ);											   
			vertices[index].texture = XMFLOAT2(u + increment, v + increment);			
			vertices[index].normal = XMFLOAT3(0.0, 1.0, 0.0);																   
			indices[index] = index;												
			index++;							

			// Upper left.
			positionX = (float)(x - unit) / resMinusOne;
			positionZ = (float)(y + unit) / resMinusOne;
			
			sampleElevation(startPixelU, startPixelV, _size, positionX, positionZ, elevation, flags);

			vertices[index].position = XMFLOAT3(positionX, elevation, positionZ);
			vertices[index].texture = XMFLOAT2(u, v + increment);
			vertices[index].normal = XMFLOAT3(0.0, 1.0, 0.0);
			indices[index] = index;
			index++;																										   			

			/// tri #2:	◿	
			// lower left																										   
			positionX = (float)(x - unit) / resMinusOne;
			positionZ = (float)(y - unit) / resMinusOne;

			sampleElevation(startPixelU, startPixelV, _size, positionX, positionZ, elevation, flags);

			vertices[index].position = XMFLOAT3(positionX, elevation, positionZ);
			vertices[index].texture = XMFLOAT2(u, v);
			vertices[index].normal = XMFLOAT3(0.0, 1.0, 0.0);
			indices[index] = index;
			index++;			
																																   
			// Bottom right
			positionX = (float)(x + unit) / resMinusOne;
			positionZ = (float)(y - unit) / resMinusOne;

			// sample the heightmap at the right location
			sampleElevation(startPixelU, startPixelV, _size, positionX, positionZ, elevation, flags);

			vertices[index].position = XMFLOAT3(positionX, elevation, positionZ);
			vertices[index].texture = XMFLOAT2(u + increment, v);
			vertices[index].normal = XMFLOAT3(0.0, 1.0, 0.0);
			indices[index] = index;
			index++;			
																																   
			// Upper right.																										   
			positionX = (float)(x + unit) / resMinusOne;																		   
			positionZ = (float)(y + unit) / resMinusOne;																		   
																																   
			sampleElevation(startPixelU, startPixelV, _size, positionX, positionZ, elevation, flags);

			vertices[index].position = XMFLOAT3(positionX, elevation, positionZ);
			vertices[index].texture = XMFLOAT2(u + increment, v + increment);
			vertices[index].normal = XMFLOAT3(0.0, 1.0, 0.0);
			indices[index] = index;
			index++;
			

		}
	}
	/// Create the edges, account for possible detail differences

	// north edge
	u = 0;
	v = 0;
	int detail_difference = 0; Direction dir;
	// default case: no LOD difference at the edge
	/*
	if (detail_difference == 0) {
		int index = 0;
		int y = GRID_RESOLUTION - 1;
		int x = 0;
		for (int i = 0; i < GRID_RESOLUTION - 1; i += 2) {
			{
				int x = i;
				RotateIndices(x, y, dir);
				index = (x)+(y * GRID_RESOLUTION);

				float PositionX = (float)(x - unit * (resolution - 2) - unit) / resMinusOne;
				float PositionZ = (float)(y - unit * (resolution - 2) - unit) / resMinusOne;

				sampleElevation(startPixelU, startPixelV, _size, positionX, positionZ, resMinusOne, elevation, flags);

				vertices[index].position = XMFLOAT3(positionX, elevation, positionZ);
				vertices[index].texture = XMFLOAT2(u + increment, v + increment);
				vertices[index].normal = XMFLOAT3(0.0, 1.0, 0.0);
				indices[index] = index;
			}
			for (int d = -1; d <= 1; d++) {
				{
					int x = i + d;
					RotateIndices(x, y, dir);
					index = (x)+(y * GRID_RESOLUTION);

					float PositionX = (float)(x - unit * (resolution - 2) - unit) / resMinusOne;
					float PositionZ = (float)(y - unit * (resolution - 2) - unit) / resMinusOne;

					sampleElevation(startPixelU, startPixelV, _size, positionX, positionZ, resMinusOne, elevation, flags);

					vertices[index].position = XMFLOAT3(positionX, elevation, positionZ);
					vertices[index].texture = XMFLOAT2(u + increment, v + increment);
					vertices[index].normal = XMFLOAT3(0.0, 1.0, 0.0);
					indices[index] = index;
				}
			}

			if (x > 0) // do it again
			{

			}
		}
	}*/
	if(detail_difference ==0)
	{
		// top edge will look like this: ◹◸◿◺◹◸ ... ◿◺◹◸
		// the amount of ◹◸s is GRID_RESOLUTION / 2
		// there is one less ◿◺ than ◹◸
		// so:
		// when i = 0; just build ◹◸
		// from i = 1 onward, build ◿◺ + ◹◸
		int j = 0;
		float y = j - unit * (resolution - 2);
		for (int i=0; i < GRID_RESOLUTION; i+=2) {
			float x;
			if (i > 0) {// build an additional ◿◺ first

				//int rotated_i = i;
				//int rotated_j = j;
				//RotateIndices(rotated_i, rotated_j, dir);

				x = (i-1) - unit * (resolution - 2);

				///tri #1: ◿
				// lower left
				
				positionX = (float)(x - unit) / resMinusOne;
				positionZ = (float)(y - unit) / resMinusOne;

				sampleElevation(startPixelU, startPixelV, _size, positionX, positionZ, elevation, flags);

				vertices[index].position = XMFLOAT3(positionX, elevation, positionZ);
				vertices[index].texture = XMFLOAT2(u, v);
				vertices[index].normal = XMFLOAT3(0.0, 1.0, 0.0);
				indices[index] = index;
				index++;

				// lower right.
				positionX = (float)(x + unit) / resMinusOne;
				positionZ = (float)(y - unit) / resMinusOne;

				sampleElevation(startPixelU, startPixelV, _size, positionX, positionZ, elevation, flags);

				vertices[index].position = XMFLOAT3(positionX, elevation, positionZ);
				vertices[index].texture = XMFLOAT2(u + increment, v);
				vertices[index].normal = XMFLOAT3(0.0, 1.0, 0.0);
				indices[index] = index;
				index++;

				// Upper right.
				positionX = (float)(x + unit) / resMinusOne;
				positionZ = (float)(y + unit) / resMinusOne;

				sampleElevation(startPixelU, startPixelV, _size, positionX, positionZ, elevation, flags);

				vertices[index].position = XMFLOAT3(positionX, elevation, positionZ);
				vertices[index].texture = XMFLOAT2(u + increment, v + increment);
				vertices[index].normal = XMFLOAT3(0.0, 1.0, 0.0);
				indices[index] = index;
				index++;

				///tri #2: ◺
				x = i - unit * (resolution - 2);
				// Upper left.
				positionX = (float)(x + unit) / resMinusOne;
				positionZ = (float)(y + unit) / resMinusOne;

				sampleElevation(startPixelU, startPixelV, _size, positionX, positionZ, elevation, flags);

				vertices[index].position = XMFLOAT3(positionX, elevation, positionZ);
				vertices[index].texture = XMFLOAT2(u, v + increment);
				vertices[index].normal = XMFLOAT3(0.0, 1.0, 0.0);
				indices[index] = index;
				index++;

				// lower left

				positionX = (float)(x + unit) / resMinusOne;
				positionZ = (float)(y - unit) / resMinusOne;

				sampleElevation(startPixelU, startPixelV, _size, positionX, positionZ, elevation, flags);

				vertices[index].position = XMFLOAT3(positionX, elevation, positionZ);
				vertices[index].texture = XMFLOAT2(u, v);
				vertices[index].normal = XMFLOAT3(0.0, 1.0, 0.0);
				indices[index] = index;
				index++;

				// lower right.
				positionX = (float)(x + unit*3.f) / resMinusOne;
				positionZ = (float)(y - unit) / resMinusOne;

				sampleElevation(startPixelU, startPixelV, _size, positionX, positionZ, elevation, flags);

				vertices[index].position = XMFLOAT3(positionX, elevation, positionZ);
				vertices[index].texture = XMFLOAT2(u + increment, v);
				vertices[index].normal = XMFLOAT3(0.0, 1.0, 0.0);
				indices[index] = index;
				index++;

				u++;
			}
			///tri #1: ◹
			x = i - unit * (resolution - 2);
			// Upper left.
			positionX = (float)(x - unit) / resMinusOne;
			positionZ = (float)(y + unit) / resMinusOne;

			sampleElevation(startPixelU, startPixelV, _size, positionX, positionZ, elevation, flags);

			vertices[index].position = XMFLOAT3(positionX, elevation, positionZ);
			vertices[index].texture = XMFLOAT2(u, v + increment);
			vertices[index].normal = XMFLOAT3(0.0, 1.0, 0.0);
			indices[index] = index;
			index++;

			// lower right.
			positionX = (float)(x + unit) / resMinusOne;
			positionZ = (float)(y - unit) / resMinusOne;

			sampleElevation(startPixelU, startPixelV, _size, positionX, positionZ, elevation, flags);

			vertices[index].position = XMFLOAT3(positionX, elevation, positionZ);
			vertices[index].texture = XMFLOAT2(u + increment, v);
			vertices[index].normal = XMFLOAT3(0.0, 1.0, 0.0);
			indices[index] = index;
			index++;

			// Upper right.
			positionX = (float)(x + unit) / resMinusOne;
			positionZ = (float)(y + unit) / resMinusOne;

			sampleElevation(startPixelU, startPixelV, _size, positionX, positionZ, elevation, flags);

			vertices[index].position = XMFLOAT3(positionX, elevation, positionZ);
			vertices[index].texture = XMFLOAT2(u + increment, v + increment);
			vertices[index].normal = XMFLOAT3(0.0, 1.0, 0.0);
			indices[index] = index;
			index++;

			///tri #2: ◸
			x = (i+1) - unit * (resolution - 2);

			
			// lower left																										   
			positionX = (float)(x - unit) / resMinusOne;
			positionZ = (float)(y - unit) / resMinusOne;

			sampleElevation(startPixelU, startPixelV, _size, positionX, positionZ, elevation, flags);

			vertices[index].position = XMFLOAT3(positionX, elevation, positionZ);
			vertices[index].texture = XMFLOAT2(u, v);
			vertices[index].normal = XMFLOAT3(0.0, 1.0, 0.0);
			indices[index] = index;
			index++;

			// Upper right.
			positionX = (float)(x + unit) / resMinusOne;
			positionZ = (float)(y + unit) / resMinusOne;

			sampleElevation(startPixelU, startPixelV, _size, positionX, positionZ, elevation, flags);

			vertices[index].position = XMFLOAT3(positionX, elevation, positionZ);
			vertices[index].texture = XMFLOAT2(u + increment, v + increment);
			vertices[index].normal = XMFLOAT3(0.0, 1.0, 0.0);
			indices[index] = index;
			index++;

			// Upper left.
			positionX = (float)(x - unit) / resMinusOne;
			positionZ = (float)(y + unit) / resMinusOne;

			sampleElevation(startPixelU, startPixelV, _size, positionX, positionZ, elevation, flags);

			vertices[index].position = XMFLOAT3(positionX, elevation, positionZ);
			vertices[index].texture = XMFLOAT2(u, v + increment);
			vertices[index].normal = XMFLOAT3(0.0, 1.0, 0.0);
			indices[index] = index;
			index++;
		}
	}

	///
	//convert vertices & indices to arrays
	VertexType* vertices_array;
	unsigned long* indices_array;
	vertices_array = ArrayFromVector(vertices);
	indices_array = ArrayFromVector(indices);
	vertexCount = vertices.size();
	indexCount = vertexCount;
	///#####################################

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices_array;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;
	// Now create the vertex buffer.
	device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices_array;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;
	// Create the index buffer.
	device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);

	// Release the arrays now that the buffers have been created and loaded.
	delete[] vertices_array;
	vertices_array = 0;
	delete[] indices_array;
	indices_array = 0;
}

	// sample the heightmap at the right location
inline bool TessellationTerrain::sampleElevation(int startPixelU, int startPixelV, float _size, float & positionX, float & positionZ,  float &elevation, EdgeFlags flags) {
	if (_heightmap) {

		if ( ((flags.left && positionX <= -0.5) || (flags.right && positionX >= 0.5)) && int(positionZ * GRID_RESOLUTION) % 2 == 0)
		{/// if edge vertex, only sample every 2nd one, to avoid SEAMS
			//odd numbered, so dont sample here but rather use the midpoint of the neighbours, as if bisecting a straight line 
			return false;
			//int neighbour_A_samplePoint = SamplePoint(startPixelU, startPixelV, _size, positionX, positionZ + (1.f/(float)resMinus1));
			//int neighbour_B_samplePoint = SamplePoint(startPixelU, startPixelV, _size, positionX, positionZ - (1.f/(float)resMinus1));
			//elevation =
			//	((*_heightmap)[neighbour_A_samplePoint].w + (*_heightmap)[max(neighbour_B_samplePoint,0)].w) / 2.0f;
		}
		else if (((flags.front && positionZ <= -0.5) || (flags.back && positionZ >= 0.5)) && int(positionZ * GRID_RESOLUTION) % 2 == 0)
		{/// same as above but for other axis
			return false;
			//int neighbour_A_samplePoint = SamplePoint(startPixelU, startPixelV, _size, positionX + (1.f / (float)resMinus1), positionZ);
			//int neighbour_B_samplePoint = SamplePoint(startPixelU, startPixelV, _size, positionX - (1.f / (float)resMinus1), positionZ);
			//elevation =
			//	((*_heightmap)[neighbour_A_samplePoint].w + (*_heightmap)[max(neighbour_B_samplePoint,0)].w) / 2.0f;	
		}
		else {// middle vertex- sample normally*/
			int samplePoint = SamplePoint(startPixelU, startPixelV, _size, positionX, positionZ);
			elevation = (*_heightmap)[samplePoint].w;// alpha channel of heightmap
			return true;
		}
	}
}
// create the border vertices of the mesh
void TessellationTerrain::BuildEdge(std::vector<VertexType>& vertices, Direction dir, const int detail_difference)
{/*
	// default case: no LOD difference at the edge
	if (detail_difference == 0) {
		int index = 0;
		int y = GRID_RESOLUTION - 1;
		int x = 0;
		for (int i = 0; i < GRID_RESOLUTION - 1; i+=2) {
			int x = i;
			RotateIndices(x, y, dir);
			index = (x) + (y * GRID_RESOLUTION);

			Position
			vertices[index] = 

			for (int d = -1; d <= 1; d++){}
		}
	}
*/}

