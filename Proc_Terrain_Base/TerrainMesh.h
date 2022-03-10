#pragma once
#include "PlaneMesh.h"
#include "Noise.h"
#include <vector>

class TerrainMesh :
	public PlaneMesh {
public:
	TerrainMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int resolution, float size, XMFLOAT2 pos);
	~TerrainMesh();

	Noise noise;

	void SetOffset(float x, float y);
	void SetScale(float s);
	void SetChunkPosition(XMFLOAT2 pos);
	XMFLOAT2 GetChunkPosition();
	float GetChunkSize();

	void Resize( int newResolution );
	void Smooth();
	void perlinSmooth(float mountainPoint = 36, float minimum = 0.08, float maximum = 1.80);
	void BrownianHtMap(int octaves, float hOffset);
	void MidpointHeightMap(int exponent);
	void Fault();
	void HydraulicErosion(float rate);
	void initRadialEffect(int radius);
	

	void Regenerate( ID3D11Device* device, ID3D11DeviceContext* deviceContext, bool build );

	const inline int GetResolution(){ return resolution; }
private:
	XMFLOAT3 getGradient(XMFLOAT2 pos);
	std::vector<int*> radialFXIndices;//int**
	std::vector<float*> radialFXWeights;//float**
	
	
	void CreateBuffers( ID3D11Device* device, VertexType* vertices, unsigned long* indices );
	void BuildHeightMap();
	
	
	float xOffset;
	float yOffset;
    float scale;
	XMFLOAT2 chunkPosition;

	const int MAXRESOLUTION = 1024;
	const float m_UVscale = 0.00125f;			//Tile the UV map  times across the plane
	float terrainSize = 420.0f;		//What is the width and height of our terrain
	float* heightMap;

	ID3D11Buffer* lodBuffer;
};
