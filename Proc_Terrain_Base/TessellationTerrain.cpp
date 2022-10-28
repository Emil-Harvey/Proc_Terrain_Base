#include "TessellationTerrain.h"


TessellationTerrain::TessellationTerrain(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HeightmapType* Heightmap, XMFLOAT2 pos, float size, int resolution)
	: TessellationPlane(device, deviceContext, resolution)
{
	
	_heightmap = Heightmap;
	initBuffers(device, pos ,size);
}



// Identical to TessellationPlane, however the vertices are displaced by a heightmap, and potential seams made by a QuadTree are sorted
void TessellationTerrain::initBuffers(ID3D11Device* device, XMFLOAT2 _pos, float _size)
{
	VertexType* vertices;
	unsigned long* indices;
	int index;//

	// positionX/Z represents the position of this vertex relative to this whole mesh [0-1]
	float positionX, positionZ, u, v, increment;//
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	// Calculate the number of vertices in the terrain mesh.
	vertexCount = (resolution - 1) * (resolution - 1) * 6;//
	indexCount = vertexCount;//

	vertices = new VertexType[vertexCount];
	indices = new unsigned long[indexCount];

	int startPixelU = (_pos.x + 0.5) * HEIGHTMAP_DIM;
	int startPixelV = (_pos.y + 0.5) * HEIGHTMAP_DIM;
	int samplePoint;
	float elevation = 90.0f;


	index = 0;//
	// UV coords.
	u = 0;//
	v = 0;//
	increment = 1.0f / resolution;//
	const float resMinusOne = resolution - 1.0f; 
	const float unit = 0.5;
	for (int j = 0; j < (resolution - 1); j++)
	{
		for (int i = 0; i < (resolution - 1); i++)
		{
			// x and y shift i & j from 0 - resolution to ~(-resolution/2) - (+resolution/2)
			const float x = i - unit * (resolution - 2);
			const float y = j - unit * (resolution - 2);
			// Bottom right
			positionX = (float)(x + unit) / resMinusOne;
			positionZ = (float)(y - unit) / resMinusOne;

			// sample the heightmap at the right location
			if (_heightmap) {
				samplePoint = (startPixelU + (positionX * _size * HEIGHTMAP_DIM)) +
					(startPixelV + (positionZ * _size * HEIGHTMAP_DIM)) * HEIGHTMAP_DIM;
				if(samplePoint < HEIGHTMAP_DIM* HEIGHTMAP_DIM)
					elevation = (*_heightmap)[samplePoint].w; // alpha channel of heightmap
			}

			vertices[index].position = XMFLOAT3(positionX, elevation, positionZ);
			vertices[index].texture = XMFLOAT2(u + increment, v);
			vertices[index].normal = XMFLOAT3(0.0, 1.0, 0.0);
			indices[index] = index;
			index++;

			// Upper right.
			positionX = (float)(x + unit) / resMinusOne;
			positionZ = (float)(y + unit) / resMinusOne;

			if (_heightmap) {
				samplePoint = (startPixelU + (positionX * _size * HEIGHTMAP_DIM)) +
					(startPixelV + (positionZ * _size * HEIGHTMAP_DIM)) * HEIGHTMAP_DIM;
				if (samplePoint < HEIGHTMAP_DIM * HEIGHTMAP_DIM)
					elevation = (*_heightmap)[samplePoint].w;
			}

			vertices[index].position = XMFLOAT3(positionX, elevation, positionZ);
			vertices[index].texture = XMFLOAT2(u + increment, v + increment);
			vertices[index].normal = XMFLOAT3(0.0, 1.0, 0.0);
			indices[index] = index;
			index++;


			// lower left
			positionX = (float)(x - unit) / resMinusOne;
			positionZ = (float)(y + unit) / resMinusOne;
			
			if (_heightmap) {
				samplePoint = (startPixelU + (positionX * _size * HEIGHTMAP_DIM)) +
					(startPixelV + (positionZ * _size * HEIGHTMAP_DIM)) * HEIGHTMAP_DIM;
				if (samplePoint < HEIGHTMAP_DIM * HEIGHTMAP_DIM)
					elevation = (*_heightmap)[samplePoint].w;
			}

			vertices[index].position = XMFLOAT3(positionX, elevation, positionZ);
			vertices[index].texture = XMFLOAT2(u, v + increment);
			vertices[index].normal = XMFLOAT3(0.0, 1.0, 0.0);
			indices[index] = index;
			index++;

			// Upper left.
			positionX = (float)(x - unit) / resMinusOne;
			positionZ = (float)(y - unit) / resMinusOne;

			if (_heightmap) {
				samplePoint = (startPixelU + (positionX * _size * HEIGHTMAP_DIM)) +
					(startPixelV + (positionZ * _size * HEIGHTMAP_DIM)) * HEIGHTMAP_DIM;
				if (samplePoint < HEIGHTMAP_DIM * HEIGHTMAP_DIM)
					elevation = (*_heightmap)[samplePoint].w;
			}

			vertices[index].position = XMFLOAT3(positionX, elevation, positionZ);
			vertices[index].texture = XMFLOAT2(u, v);
			vertices[index].normal = XMFLOAT3(0.0, 1.0, 0.0);
			indices[index] = index;
			index++;

			u += increment;

		}

		u = 0;
		v += increment;
	}


	// Set up the description of the static vertex buffer.
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
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;
	// Create the index buffer.
	device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);

	// Release the arrays now that the buffers have been created and loaded.
	delete[] vertices;
	vertices = 0;
	delete[] indices;
	indices = 0;
}
