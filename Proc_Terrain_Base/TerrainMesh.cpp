#include "TerrainMesh.h"
#include <cmath>

TerrainMesh::TerrainMesh( ID3D11Device* device, ID3D11DeviceContext* deviceContext, int lresolution, float size, XMFLOAT2 pos ) :
	PlaneMesh( device, deviceContext, lresolution ) 
{
	Resize( resolution );
	Regenerate( device, deviceContext , true);

	chunkPosition = pos;
	terrainSize = size;
}

//Cleanup the heightMap
TerrainMesh::~TerrainMesh() {
	delete[] heightMap;
	heightMap = 0;
}


//Fill an array of floats that represent the height values at each grid point.
//Here we are producing a Sine wave along the X-axis
void TerrainMesh::BuildHeightMap() {
	const float height = 05.0f;

	//Scale everything so that the look is consistent across terrain resolutions
	//const float scalef =  terrainSize / (float)resolution;


	for( int j = 0; j < ( resolution ); j++ ) {
		for( int i = 0; i < ( resolution ); i++ ) {
			heightMap[( j * resolution ) + i] = height;
			
		}
	}	

}

void TerrainMesh::MidpointHeightMap(int exponent) {// Not Working...
	//to ensure the heightmap is 2^n + 1
	if (exponent < 2) { exponent = 2; }
	resolution = pow(2, exponent) + 1;
	Resize(resolution);
	BuildHeightMap();
	
	float height = 0.0f;
	float scale = (terrainSize / (float)resolution);
	float spread = 100;

	//find corners & give height
	int corner1 = 0;
	int corner2 = resolution;
	int corner3 = resolution * (resolution - 1);
	int corner4 = resolution * resolution;
	int midpoint1;
	int midpoint2;
	int midpoint3;
	int midpoint4;
	int center;
	int subresolution = resolution;
	int subsPerRow;


	heightMap[corner1] = 18;//(rand() /RAND_MAX) * spread;// top left
	heightMap[corner2] = 16;//(rand() /RAND_MAX) * spread;// top right
	heightMap[corner3] = 2;//(rand() /RAND_MAX) * spread;// bottom left
	heightMap[corner4] = 12;//(rand() /RAND_MAX) * spread;// bottom right
	
	for (int i = 0; i < exponent; i++) {
		subsPerRow = sqrt(pow(4, i));
		for (int subdivision = 0; subdivision < pow(4, i); subdivision++) {
			
			
			for (int x = 0; x < subsPerRow; x++) {
				for (int y = 0; y < subsPerRow; y++) {
					corner1 = subdivision/subsPerRow *subresolution -1;
					corner2 = (subdivision / subsPerRow * subresolution -1) + subresolution;
					corner3 = (subdivision / subsPerRow * subresolution -1) + (subresolution * (resolution - 1));
					corner4 = (subdivision / subsPerRow * subresolution -1) + (subresolution * resolution);

					//heightMap[corner1] = rand() % 1 * spread;// top left
					//heightMap[corner2] = rand() % 1 * spread;// top right
					//heightMap[corner3] = rand() % 1 * spread;// bottom left
					//heightMap[corner4] = rand() % 1 * spread;// bottom right

					//find midpoints
					midpoint1 = (corner1 + corner2) / 2;// top
					midpoint2 = (corner1 + corner3) / 2;// left
					midpoint3 = (corner2 + corner4) / 2;// right
					midpoint4 = (corner3 + corner4) / 2;// bottom

					heightMap[midpoint1] = 1;//rand() % 1 * spread;
					heightMap[midpoint2] = 20;// rand() % 1 * spread;
					heightMap[midpoint3] = 130;//rand() % 1 * spread;
					heightMap[midpoint4] = -4;// rand() % 1 * spread;

					center = (midpoint1 + midpoint4) / 2;

					heightMap[center] = -36;//((rand() / RAND_MAX) * spread);
				}
			}
			
		}
		subresolution /= 2;
		spread /= 2;// half the spread each time
	}
	
	/*///find midpoints
	midpoint1 = (corner1 + corner2) / 2;// top
	midpoint2 = (corner1 + corner3) / 2;// left
	midpoint3 = (corner2 + corner4) / 2;// right
	midpoint4 = (corner3 + corner4) / 2;// bottom

	heightMap[midpoint1] = 8;//rand() % 1 * spread;
	heightMap[midpoint2] = -12;//rand() % 1 * spread;
	heightMap[midpoint3] = 1;//rand() % 1 * spread;
	heightMap[midpoint4] = -3;//rand() % 1 * spread;

	center = (midpoint1 + midpoint4) / 2;

	heightMap[center] = //rand() % 1 * spread;

	corner1 = 0;
	corner2 = resolution;
	corner3 = resolution * (resolution - 1);
	corner4 = resolution * resolution;
	//*/
}

void TerrainMesh::BrownianHtMap(int octaves,  float heightOffset) {
	
	
	float height = 0.0f;
	float amplitude = 72 * scale;

	//Scale everything so that the look is consistent across terrain resolutions
	float scaleFactor = MAXRESOLUTION/(scale *1024 * 200);

	for (int j = 0; j < (resolution); j++) {// sub-continental scale
		for (int i = 0; i < (resolution); i++) {
			// world position = i * terrainsize/resolution
			height = noise.perlin(abs((i * terrainSize / resolution) + xOffset) * scaleFactor * 0.0086, abs((j * terrainSize / resolution) + yOffset) * scaleFactor *0.0086) * amplitude * 7.60f;
			heightMap[(j * resolution) + i] = height + heightOffset;//*min(height*height, amplitude/abs(height))
		}
	}

	for (int j = 0; j < (resolution); j++) {// mountain ranges scale
		for (int i = 0; i < (resolution); i++) {
			// world position = i * terrainsize/resolution
			height = noise.ridges(abs(( i *terrainSize/ resolution) + xOffset) *scaleFactor  , abs(( j *terrainSize/ resolution) + yOffset) * scaleFactor ) * amplitude;//-12+(	((1*i /resolution) + xOffset * scale) / 10 + ( (1*j/resolution) + yOffset * scale) / 10);//noise.perlin(((i + xOffset) * scaleFactor) / terrainSize, (j + yOffset) * (scaleFactor / terrainSize)) * amplitude;
			heightMap[(j * resolution) + i] += max(height,0);// +heightOffset;
		}
	}
	amplitude *= 0.5;// for making initial octave disproportionate
	scaleFactor *= 2;
	for (int octave = 1; octave < octaves; octave++) {// sub- mountains etc scale
		
		for (int j = 0; j < (resolution); j++) {
			for (int i = 0; i < (resolution); i++) {
				height = noise.ridges(abs((i * terrainSize / resolution) + xOffset) * scaleFactor, abs((j * terrainSize / resolution) + yOffset) * scaleFactor) * amplitude;//-12+(	((1*i /resolution) + xOffset * scale) / 10 + ( (1*j/resolution) + yOffset * scale) / 10);//noise.perlin(((i + xOffset) * scaleFactor) / terrainSize, (j + yOffset) * (scaleFactor / terrainSize)) * amplitude;
				heightMap[(j * resolution) + i] += height * log(log(abs(heightMap[(j * resolution) + i])+1)+1);
			}
		}
		amplitude *= 0.5;
		scaleFactor *= 2;
	}
}

void TerrainMesh::SetOffset(float x, float y)
{	
	xOffset = x  + chunkPosition.x;
	yOffset = y  + chunkPosition.y;
}

void TerrainMesh::SetScale(float s)
{
	scale = s;
}

void TerrainMesh::SetChunkPosition(XMFLOAT2 pos)
{
	chunkPosition = pos;
}

XMFLOAT2 TerrainMesh::GetChunkPosition()
{
	return chunkPosition;
}

float TerrainMesh::GetChunkSize()
{
	return terrainSize;
}

void TerrainMesh::Resize( int newResolution ) {
	resolution = newResolution;
	heightMap = new float[resolution * resolution];
	if( vertexBuffer != NULL ) {
		vertexBuffer->Release();
	}
	vertexBuffer = NULL;
}

void TerrainMesh::Smooth() {
	float height = 0.0f;
	float* temp_hm;
	temp_hm = new float[resolution * resolution];

	///*// smooth terrain
	for (int j = 0; j < (resolution); j++) {
		for (int i = 0; i < (resolution); i++) {
			int x = 1;
			height = heightMap[(j * resolution) + i];// avoid edge vertices
			if (i <= (resolution - 2)) {
				height += (heightMap[(j * resolution) + i + 1]); //east
				x++;
			}
			if (i >= 1) {
				height += (heightMap[(j * resolution) + i - 1]); //west
				x++;
			}
			if (j <= (resolution - 2)) {
				height += (heightMap[((j + 1) * resolution) + i]); //south
				x++;
			}
			if (j >= 1) {
				height += (heightMap[((j - 1) * resolution) + i]); //north
				x++;
			}
			height = height / x;
			if (height < -10) {
				height = -10;
			}
			temp_hm[(j * resolution) + i] = height; // average

		}
	}//*/
	for (int j = 0; j < (resolution); j++) {
		for (int i = 0; i < (resolution); i++) {
			heightMap[(j * resolution) + i] = temp_hm[(j * resolution) + i];
		}
	}
}

// height based smoothing, for jagged mountains and shallow plains
void TerrainMesh::perlinSmooth(float mp, float min, float max)
{
	float height = 0.0f;
	float* temp_hm;
	temp_hm = new float[resolution * resolution];
	
// elevation at which the smoothing effect makes no change, i.e. height^2 == height.
	const float mountainpoint = mp * scale;

	///*// smooth terrain by division
	//
		for (int j = 0; j < (resolution); j++) {
			for (int i = 0; i < (resolution); i++) {
				height = heightMap[(j * resolution) + i];
				
				if (height > 0 ) {// do not smooth heights below sea level
					//
					
					height /= mountainpoint;// makes elevations below mountain height into decimals, and so exponent function will reduce their value
					
					height = height * min(max((height + (height / 3.0)) * (height - (height / 3.0)), min), max); // height cannot be reduced past 8% height, & cannot be increased past 180% height
					
					height *= mountainpoint;
					
				}
				
				temp_hm[(j * resolution) + i] = height; 

				
			}
		}//*/

		for (int j = 0; j < (resolution); j++) {// apply new heights
			for (int i = 0; i < (resolution); i++) {
				heightMap[(j * resolution) + i] = temp_hm[(j * resolution) + i];
			}
		}
	//}
}

void TerrainMesh::Fault() {
	// define fault line
	float a = 0 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 360)); // gradient - betwn 0 (horizontal) and 360
	float c =  static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (resolution))); // offset - betwn -res and res
	// y = mx + c ... m = tan(a)
	float p = (float)rand() / RAND_MAX;// random nuber between 1 & 0, to determine which side of the line 
	for (int y = 0; y < (resolution); y++) {// apply new heights
		for (int x = 0; x < (resolution); x++) {
			if ((y >= tan(a) * x + c && p < 0.5)||(y <= tan(a) * x + c && p > 0.5)) {
				heightMap[(y * resolution) + x] += 1.3;
			}
			else {
				heightMap[(y * resolution) + x] -= 1.3;
			}
		}
	}

}
// simulate rain based erosin using individual particles
void TerrainMesh::HydraulicErosion(const float erosionRate)
{ // reference: implementation of a method for hydraulic erosion, Hans B (2015); sebastian lague (https://github.com/SebLague/Erosion-Demo/blob/master/Assets/Scripts/Erosion.cs)
	XMFLOAT2 pos;
	float height;
	
	//const float erosionRate = 0.6; //	the rate sediment is eroded & picked up by a particle
	const float maxCap = 4; //	the sediment capacity of a particle (multiplier)
	const float deposition = 0.1; //	amount of sediment deposited when a particle is at max capacity
	//const float minSlope = 0.5;
	//maximum path steps? evaporation?
	for (int k = 0; k < 999000; k++) {
		float speed = 1;

		pos.x = (float)rand() / (RAND_MAX / resolution);// somehow i was getting values > resolution
		pos.x = min(pos.x, resolution - 1);
		pos.y = (float)rand() / (RAND_MAX / resolution);
		pos.y = min(pos.y, resolution - 1);

		int cellIndex = min(((int)pos.y * resolution) + (int)pos.x, resolution*resolution -2);// the SE vertex of the cell we're in
		
		height = heightMap[cellIndex];
		float water = 1;
		float inertia = 0.5;// determines how much sediment will be carried
		float sediment = 0;// amount of sediment carried/to deposit
		
		XMFLOAT2 dir = { 0, 0 };//direction of particle


		for (int lifetime = 0; lifetime < 80; lifetime++) {

			auto gradient = getGradient(pos);
			height = gradient.z;
			dir.x = (dir.x * inertia - gradient.x * (1 - inertia));// high inertia = less change in direction
			dir.y = (dir.y * inertia - gradient.y * (1 - inertia));
			//XMVector2Normalize( XMLoadFloat2(dir)) 

			// Normalize direction
			float len = sqrt(dir.x * dir.x + dir.y * dir.y);
			if (len != 0) {
				dir.x /= len;
				dir.y /= len;
			}
			// move in that direction (always 1 unit)
			pos.x += dir.x;
			pos.y += dir.y;

			cellIndex = ((int)pos.y * resolution) + (int)pos.x;// update cell

			// Stop simulating particle if it's still, out of bounds, or has been evaporated
			if ((dir.x == 0 && dir.y == 0) || (pos.x > resolution - 1) || (pos.y > resolution - 1) || (pos.x < 0) || (pos.y < 0) || (water < 0.001))
			{
				break;
			}
			// the diffence in height to the new position
			float deltaHeight = getGradient(pos).z - height;

			float capacity = min(-deltaHeight * speed * water, maxCap);// current sediment capacity at new position 

			if (sediment > capacity || deltaHeight > 0) {// deposit sediment
				// deposit as much as can fill the deltaHeight if going upwards, otherwise deposit a fraction
				float deposited = (deltaHeight > 0) ? min(deltaHeight, sediment) : (sediment - capacity) * deposition;
				sediment -= deposited;

				//add the deposit to the 4 vertices of the cell via bilinear interpolation -- [should deposit over a wider area]
				heightMap[cellIndex] += deposited * (1 - (pos.x - (int)pos.x)) * (1 - (pos.y - (int)pos.y));//SE vertex
				heightMap[cellIndex + 1] += deposited * (pos.x - (int)pos.x) * (1 - (pos.y - (int)pos.y));//SW vertex
				heightMap[cellIndex + resolution] += deposited * (1 - (pos.x - (int)pos.x)) * (pos.y - (int)pos.y);//NE vertex
				heightMap[cellIndex + resolution + 1] += deposited * (pos.x - (int)pos.x) * (pos.y - (int)pos.y);//NW vertex

			}
			else {// erode away some sediment
				//do not erode more than the difference in height
				float eroded = min((capacity - sediment) * erosionRate, -deltaHeight);
				// the amount of sediment picked up ......
				//float deltaSediment = (heightMap[cellIndex] < eroded) ? heightMap[cellIndex] : eroded;

				//erode the sediment from the 4 vertices of the cell via bilinear interpolation -- [should erode over a wider area]
				//heightMap[cellIndex] -= eroded * (1 - (pos.x - (int)pos.x)) * (1 - (pos.y - (int)pos.y));//SE vertex
				//heightMap[cellIndex + 1] -= eroded * (pos.x - (int)pos.x) * (1 - (pos.y - (int)pos.y));//SW vertex
				//heightMap[cellIndex + resolution] -= eroded * (1 - (pos.x - (int)pos.x)) * (pos.y - (int)pos.y);//NE vertex
				//heightMap[cellIndex + resolution + 1] -= eroded * (pos.x - (int)pos.x) * (pos.y - (int)pos.y);//NW vertex
				//sediment += deltaSediment;
							//if (radialFXIndices.size() / sizeof(int) <= 2) { break; }
				// Use erosion brush to erode from all vertices inside the droplet's erosion radius
				for (int brushPointIndex = 0; brushPointIndex < sizeof(radialFXIndices.at(cellIndex))/sizeof(int); brushPointIndex++) {
					int nodeIndex = radialFXIndices.at(cellIndex)[brushPointIndex];
					float weighedErodeAmount = eroded * radialFXWeights.at(cellIndex)[brushPointIndex];
					// the amount of sediment picked up ......
					float deltaSediment = (heightMap[nodeIndex] < weighedErodeAmount) ? heightMap[nodeIndex] : weighedErodeAmount;
					heightMap[nodeIndex] -= deltaSediment;
					sediment += deltaSediment;// add sediment
				}
				
				//if (sediment > 2) {
				//	//breakpoint
				//}

			}
			// Update particle speed and water content
			speed = sqrt(speed * speed + deltaHeight);
			water *= 0.9; //(1 - evaporateSpeed);

			
		}

	}
}// used for hydraulic erosion
XMFLOAT3 TerrainMesh::getGradient(XMFLOAT2 pos) {
	const int SW = 0, SE = 1, NW = 2, NE = 3;

	XMFLOAT3 gradient = { 0,0,0 };
	XMINT2 cell = { (int)pos.x,(int)pos.y };
	
	float x = (pos.x - cell.x);//(pos - cell = internal coordinates)
	float y = (pos.y - cell.y);

	float heights[4];// the heights at the 4 corners of the given cell
	heights[SW] = heightMap[cell.y * resolution + cell.x];// bottom left or SW
	heights[SE] = heightMap[cell.y * resolution + cell.x+1];// bottom right
	heights[NW] = heightMap[(cell.y+1) * resolution + cell.x];// top left
	heights[NE] = heightMap[(cell.y+1) * resolution + cell.x+1];// top right

	// bilinearly interpolate the gradient based on the proximity to each corner of the cell 
	gradient.x = (heights[NE] - heights[NW]) * (1 - y) + (heights[SE] - heights[SW]) * y;
	gradient.y = (heights[SW] - heights[NW]) * (1 - x) + (heights[SE] - heights[NE]) * x;

	//Calculate height with bilinear interpolation of the heights @ each corner (z component = height)
	gradient.z = heights[NW] * (1 - x) * (1 - y) + heights[NE] * x * (1 - y) + heights[SW] * (1 - x) * y + heights[SE] * x * y;
	// height +=

	return gradient;
}
void TerrainMesh::initRadialEffect(int radius)// for radial erosion effect
{
	//const int radius = 4;
	//erosionBrushIndices = new int[mapSize * mapSize][];
	//radialFXIndices = new int *[resolution * resolution];// [radius * radius] ;
			//std::vector<int*> radInd;
	//erosionBrushWeights = new float[mapSize * mapSize][];
	//radialFXWeights = new float *[resolution * resolution];// [] ;
			//std::vector<float*> radWts;
	//
	int* xOffsets = new int[radius * radius * 4];// distances from center of effect
	int* yOffsets = new int[radius * radius * 4];
	float* weights = new float[radius * radius * 4];// the effect's weight at the offset
	float sumOfWeights = 0;
	int currentIndex = 0;
	//
	//for (int i = 0; i < erosionBrushIndices.GetLength(0); i++) {
	for (int i = 0; i < (resolution * resolution); i++) {
	//	int centreX = i % mapSize;
		int centerX = i % resolution;
	//	int centreY = i / mapSize;
		int centerY = i / resolution;
	//
	//	if (centreY <= radius || centreY >= mapSize - radius || centreX <= radius + 1 || centreX >= mapSize - radius) {
		if (centerY <= radius || centerY >= resolution - radius || centerX <= radius + 1 || centerX >= resolution - radius) {// if the center is near the edge of terrain
	//		weightSum = 0;
			sumOfWeights = 0;
	//		addIndex = 0;
			currentIndex = 0;
			for (int y = -radius; y <= radius; y++) {
				for (int x = -radius; x <= radius; x++) {
					float sqrDst = x * x + y * y;
					if (sqrDst < radius * radius) {// check that local (x,y) is within the circle
						int coordX = centerX + x;
						int coordY = centerY + y;
	//
						if (coordX >= 0 && coordX < resolution && coordY >= 0 && coordY < resolution) {// if the coords are within the map
							float weight = 1 - sqrt(sqrDst) / radius;// simple lerp between center & edge
	//						weightSum += weight;
							sumOfWeights += weight;
							weights[currentIndex] = weight;
							xOffsets[currentIndex] = x;
							yOffsets[currentIndex] = y;
							currentIndex++;
						}
					}
				}
			}
		}
	//
		//int numEntries = currentIndex;// necessary?
	//	erosionBrushIndices[i] = new int[numEntries];
		//radialFXIndices[i] = new int[radius * radius];//[numEntries];
		radialFXIndices.push_back(new int[currentIndex]);
	//	erosionBrushWeights[i] = new float[numEntries];
		//radialFXWeights[i] = new float[radius * radius];//[numEntries];
		radialFXWeights.push_back(new float[currentIndex]);
	//
		for (int j = 0; j < currentIndex; j++) {
	//		erosionBrushIndices[i][j] = (yOffsets[j] + centreY) * mapSize + xOffsets[j] + centreX;
			//radialFXIndices[i][j] = (yOffsets[j] + centerY) * resolution + xOffsets[j] + centerX;
			radialFXIndices.back()[j] = (yOffsets[j] + centerY) * resolution + xOffsets[j] + centerX;
	//		erosionBrushWeights[i][j] = weights[j] / weightSum;
			//radialFXWeights[i][j] = weights[j] / sumOfWeights;
			radialFXWeights.back()[j] = weights[j] / sumOfWeights;
		}
	}
}

// Set up the heightmap and create or update the appropriate buffers
void TerrainMesh::Regenerate( ID3D11Device * device, ID3D11DeviceContext * deviceContext, bool build ) {

	VertexType* vertices;
	unsigned long* indices;
	int index, i, j;
	float positionX, height, positionZ, u, v, increment;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	
	//Calculate and store the height values
	if (build) { BuildHeightMap(); }

	// Calculate the number of vertices in the terrain mesh.
	// We share vertices in this mesh, so the vertex count is simply the terrain 'resolution'
	// and the index count is the number of resulting triangles * 3 OR the number of quads * 6
	vertexCount = resolution * resolution;

	indexCount = ( ( resolution - 1 ) * ( resolution - 1 ) ) * 6;
	vertices = new VertexType[vertexCount];
	indices = new unsigned long[indexCount];

	index = 0;

	// UV coords.
	u = 0;
	v = 0;
	increment = m_UVscale * 128; //max(sqrt(resolution), 80);// higher res terrain will have smaller textures, as they typically will depict larger scale terrains

	//Scale everything so that the look is consistent across terrain resolutions
	const float scaleF = terrainSize / (float)resolution;

	//Set up vertices
	for( j = 0; j < ( resolution ); j++ ) {
		for( i = 0; i < ( resolution ); i++ ) {
			positionX = (float)i * scaleF;
			positionZ = (float)( j ) * scaleF;

			height = heightMap[index];
			vertices[index].position = XMFLOAT3( positionX, height, positionZ );
			vertices[index].texture = XMFLOAT2( u, v );

			u += increment;
			index++;
		}
		u = 0;
		v += increment;
	}

	//Set up index list
	index = 0;
	for( j = 0; j < ( resolution - 1 ); j++ ) {
		for( i = 0; i < ( resolution - 1 ); i++ ) {

			//Build index array
			indices[index] = ( j*resolution ) + i;
			indices[index + 1] = ( ( j + 1 ) * resolution ) + ( i + 1 );
			indices[index + 2] = ( ( j + 1 ) * resolution ) + i;

			indices[index + 3] = ( j * resolution ) + i;
			indices[index + 4] = ( j * resolution ) + ( i + 1 );
			indices[index + 5] = ( ( j + 1 ) * resolution ) + ( i + 1 );
			index += 6;
		}
	}

	//Set up normals
	for( j = 0; j < ( resolution - 1 ); j++ ) {
		for( i = 0; i < ( resolution - 1 ); i++ ) {
			//Calculate the plane normals
			XMFLOAT3 a, b, c;	//Three corner vertices
			a = vertices[j * resolution + i].position;
			b = vertices[j * resolution + i + 1].position;
			c = vertices[( j + 1 ) * resolution + i].position;

			//Two edges
			XMFLOAT3 ab( c.x - a.x, c.y - a.y, c.z - a.z );
			XMFLOAT3 ac( b.x - a.x, b.y - a.y, b.z - a.z );
			
			//Calculate the cross product
			XMFLOAT3 cross;
			cross.x = ab.y * ac.z - ab.z * ac.y;
			cross.y = ab.z * ac.x - ab.x * ac.z;
			cross.z = ab.x * ac.y - ab.y * ac.x;
			float mag = ( cross.x * cross.x ) + ( cross.y * cross.y ) + ( cross.z * cross.z );
			mag = sqrtf( mag );
			cross.x/= mag;
			cross.y /= mag;
			cross.z /= mag;
			vertices[j * resolution + i].normal = cross;
		}
	}

	//Smooth the normals by averaging the normals from the surrounding planes
	XMFLOAT3 smoothedNormal( 0, 1, 0 );
	for (int r = 0; r < 3; r++) {// smooth 2 times
		for (j = 0; j < resolution; j++) {
			for (i = 0; i < resolution; i++) {
				smoothedNormal.x = 0;
				smoothedNormal.y = 0;
				smoothedNormal.z = 0;
				float count = 0;
				//Left planes
				if ((i - 1) >= 0) {
					//Top planes
					if ((j) < (resolution - 1)) {
						smoothedNormal.x += vertices[j * resolution + (i - 1)].normal.x;
						smoothedNormal.y += vertices[j * resolution + (i - 1)].normal.y;
						smoothedNormal.z += vertices[j * resolution + (i - 1)].normal.z;
						count++;
					}
					//Bottom planes
					if ((j - 1) >= 0) {
						smoothedNormal.x += vertices[(j - 1) * resolution + (i - 1)].normal.x;
						smoothedNormal.y += vertices[(j - 1) * resolution + (i - 1)].normal.y;
						smoothedNormal.z += vertices[(j - 1) * resolution + (i - 1)].normal.z;
						count++;
					}
				}
				//right planes
				if ((i) < (resolution - 1)) {

					//Top planes
					if ((j) < (resolution - 1)) {
						smoothedNormal.x += vertices[j * resolution + i].normal.x;
						smoothedNormal.y += vertices[j * resolution + i].normal.y;
						smoothedNormal.z += vertices[j * resolution + i].normal.z;
						count++;
					}
					//Bottom planes
					if ((j - 1) >= 0) {
						smoothedNormal.x += vertices[(j - 1) * resolution + i].normal.x;
						smoothedNormal.y += vertices[(j - 1) * resolution + i].normal.y;
						smoothedNormal.z += vertices[(j - 1) * resolution + i].normal.z;
						count++;
					}
				}
				smoothedNormal.x /= count;
				smoothedNormal.y /= count;
				smoothedNormal.z /= count;

				float mag = sqrt((smoothedNormal.x * smoothedNormal.x) + (smoothedNormal.y * smoothedNormal.y) + (smoothedNormal.z * smoothedNormal.z));
				smoothedNormal.x /= mag;
				smoothedNormal.y /= mag;
				smoothedNormal.z /= mag;

				vertices[j * resolution + i].normal = smoothedNormal;
			}
		}
	}
	//If we've not yet created our dyanmic Vertex and Index buffers, do that now
	if( vertexBuffer == NULL ) {
		CreateBuffers( device, vertices, indices );
	}
	else {
		//If we've already made our buffers, update the information
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory( &mappedResource, sizeof( D3D11_MAPPED_SUBRESOURCE ) );

		//  Disable GPU access to the vertex buffer data.
		deviceContext->Map( vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
		//  Update the vertex buffer here.
		memcpy( mappedResource.pData, vertices, sizeof( VertexType ) * vertexCount );
		//  Reenable GPU access to the vertex buffer data.
		deviceContext->Unmap( vertexBuffer, 0 );
	}

	// Release the arrays now that the buffers have been created and loaded.
	delete[] vertices;
	vertices = 0;
	delete[] indices;
	indices = 0;
}



//Create the vertex and index buffers that will be passed along to the graphics card for rendering
//For CMP305, you don't need to worry so much about how or why yet, but notice the Vertex buffer is DYNAMIC here as we are changing the values often
void TerrainMesh::CreateBuffers( ID3D11Device* device, VertexType* vertices, unsigned long* indices ) {

	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc, lodBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData, lodData;

	struct LODBufferType {
		float LOD;
		float res;
		XMFLOAT2 pos ;
	}; 
	
	

	// Set up the description of the dyanmic vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof( VertexType ) * vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;
	// Now create the vertex buffer.
	device->CreateBuffer( &vertexBufferDesc, &vertexData, &vertexBuffer );

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof( unsigned long ) * indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	device->CreateBuffer( &indexBufferDesc, &indexData, &indexBuffer );

	
}