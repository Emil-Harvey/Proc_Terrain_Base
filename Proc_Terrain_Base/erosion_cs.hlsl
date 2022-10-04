// Hydraulic Erosion algorithm
// based on 



Texture2D inHeightmap : register(t0);
RWTexture2D<float4> gOutput : register(u0);

#define resolution 1500
#define RADIUS 6

int radialFXIndices[resolution];//[1500] ;// fix?
float radialFXWeights[resolution];// [resolution] ;// fix?

void hydraulicErosion(const float erosionrate, uint3 DTid);
float3 getGradient(float2 pos);
void initRadialEffect(int radius);
//const float erosionRate = 1.0;



float hash12(float2 p)// one output, two inputs
{
	float3 p3 = frac(float3(p.xyx) * .1031);
	p3 += dot(p3, p3.yzx + 33.33);
	return frac((p3.x + p3.y) * p3.z);
}

[numthreads(25, 25, 1)]
void main(const uint3 DTid : SV_DispatchThreadID)
{
	float4 colour = float4(0.7, 0.5, 0.7, 10.0 *sin(float(DTid.x)));
	
	colour = inHeightmap[DTid.xy];

	gOutput[DTid.xy] = colour;

	hydraulicErosion(0.50f, DTid);

	
	gOutput[DTid.xy]; //= colour;
}
void hydraulicErosion(const float erosionRate, uint3 DTid)
{ // reference: implementation of a method for hydraulic erosion, Hans B (2015); sebastian lague (https://github.com/SebLague/Erosion-Demo/blob/master/Assets/Scripts/Erosion.cs)
	
	//initRadialEffect()

	float2 pos;
	float height;

	//const float erosionRate = 0.6; //	the rate sediment is eroded & picked up by a particle
	const float maxCap = 4; //	the sediment capacity of a particle (multiplier)
	const float deposition = 0.1; //	amount of sediment deposited when a particle is at max capacity
	//const float minSlope = 0.5;
	//maximum path steps? evaporation?

	const int NUM_DROPLETS_SIMULATED = 1;
	const int MAX_DROP_LIFE = 100;


	for (int k = 0; k < NUM_DROPLETS_SIMULATED; k++) {
		float speed = 1.0f;
		pos.xy = DTid.xy;
		//%%pos.x = hash12(DTid.xy) * (resolution - 1) ;// 0.35f + * 1.0
			pos.x = clamp(pos.x, resolution *0.35,resolution*0.65);
		//%%pos.y = hash12(DTid.zx) * (resolution - 1) *1.0 ;//0.35f + 
			pos.y = clamp(pos.y, resolution * 0.35, resolution * 0.65);
		

		int2 cellIndex = int2(pos.x, pos.y);// the SE vertex of the cell we're in

		height = gOutput[cellIndex].a;
		float water = 1;
		float inertia = 0.5;// determines how much sediment will be carried {?}
		float sediment = 0;// amount of sediment carried/to deposit

		float2 dir = { 0, 0 };//direction of particle


		for (int lifetime = 0; lifetime < MAX_DROP_LIFE; lifetime++) {

			float3 gradient = getGradient(pos);
			height = gradient.z;
			dir.x = (dir.x * inertia - gradient.x * (1 - inertia));// high inertia = less change in direction
			dir.y = (dir.y * inertia - gradient.y * (1 - inertia));
			dir = normalize(dir);

			//	// Normalize direction
			//	float len = sqrt(dir.x * dir.x + dir.y * dir.y);
			//if (len != 0) {
			//	dir.x /= len;
			//	dir.y /= len;
			//}
			
			// move in that direction (always 1 unit)
			pos.x += dir.x;
			pos.y += dir.y;

			cellIndex = int2(pos.x, pos.y);// update cell
			float2 inner_pos = abs(pos - (float2)cellIndex);

			// Stop simulating particle if it's still, out of bounds, or has been evaporated
			if ((dir.x == 0 && dir.y == 0) || (pos.x > resolution - 1) || (pos.y > resolution - 1) || (pos.x < 0) || (pos.y < 0) || (water < 0.01))
			{
				break;
			}
			// the diffence in height to the new position
			float deltaHeight = getGradient(pos).z - height;

			float capacity = min(-deltaHeight * speed * water, maxCap);// current sediment capacity at new position 

			float4 blue_alpha_mask = float4(0.f, 0.f, 0.1f, 1.f);

			if (sediment > capacity || deltaHeight > 0 || height+deltaHeight <-2) {// deposit sediment    
				// deposit as much as can fill the deltaHeight if going upwards, otherwise deposit a fraction
				float deposited = (deltaHeight > 0) ? (min(deltaHeight, sediment)) : ((sediment - capacity) * deposition);
				sediment -= deposited;

				//add the deposit to the 4 vertices of the cell via bilinear interpolation -- [should deposit over a wider area]
				gOutput[cellIndex] += blue_alpha_mask * deposited * saturate(1 - inner_pos.x) * saturate(1 - inner_pos.y);	// SE vertex
				gOutput[int2(cellIndex.x +1, cellIndex.y)] += blue_alpha_mask * deposited * saturate(inner_pos.x) * saturate(1 - inner_pos.y);// SW vertex
				gOutput[int2(cellIndex.x, cellIndex.y +1)] += blue_alpha_mask * deposited * saturate(1 - inner_pos.x) * saturate(pos.y - (int)pos.y);// NE vertex
				gOutput[int2(cellIndex.x +1, cellIndex.y +1)] += blue_alpha_mask * deposited * saturate(inner_pos.x) * saturate(pos.y - (int)pos.y);// NW vertex

			}
			else {// erode away some sediment
				//do not erode more than the difference in height
				float eroded = clamp((capacity - sediment) * erosionRate, -deltaHeight, 0.0f);
				// the amount of sediment picked up ......
				float deltaSediment = (gOutput[cellIndex].a < eroded) ? gOutput[cellIndex].a : eroded;

				//erode the sediment from the 4 vertices of the cell via bilinear interpolation -- [should erode over a wider area]
				gOutput[cellIndex] -= blue_alpha_mask* eroded * saturate(1 - (pos.x - (int)pos.x)) * saturate(1 - (pos.y - (int)pos.y));//SE vertex
				gOutput[int2(cellIndex.x + 1, cellIndex.y)] -= blue_alpha_mask * eroded * saturate(pos.x - (int)pos.x) * saturate(1 - (pos.y - (int)pos.y));//SW vertex
				gOutput[int2(cellIndex.x, cellIndex.y + 1)] -= blue_alpha_mask * eroded * saturate(1 - (pos.x - (int)pos.x)) * saturate(pos.y - (int)pos.y);//NE vertex
				gOutput[int2(cellIndex.x + 1, cellIndex.y + 1)] -= blue_alpha_mask * eroded * saturate(pos.x - (int)pos.x) * saturate(pos.y - (int)pos.y);//NW vertex
				sediment += deltaSediment;
							//if (radialFXIndices.size() / sizeof(int) <= 2) { break; }
				/*// Use erosion brush to erode from all vertices inside the droplet's erosion radius
				for (int brushPointIndex = 0; brushPointIndex < RADIUS; brushPointIndex++) {// assuming brush radius of 
					int nodeIndex = radialFXIndices[cellIndex] [brushPointIndex];//~~~
					float weighedErodeAmount = eroded * radialFXWeights[cellIndex] [brushPointIndex];//~~~
					// the amount of sediment picked up ......
					float deltaSediment = (gOutput[nodeIndex] < weighedErodeAmount) ? gOutput[nodeIndex] : weighedErodeAmount;
					gOutput[nodeIndex] -= deltaSediment;
					sediment += deltaSediment;// add sediment
				}*/

				//if (sediment > 2) {
				//	//breakpoint
				//}

			}
			// Update particle speed and water content
			speed = sqrt(speed * speed + deltaHeight);
			water *= 0.79; //(1 - evaporateSpeed);


		}

	}
}

//

float3 getGradient(float2 pos) {
	const int SW = 0, SE = 1, NW = 2, NE = 3;

	float3 gradient = { 0,0,0 };
	int2 cell = { (int)pos.x,(int)pos.y };

	float x = (pos.x - cell.x);//(pos - cell = internal coordinates)
	float y = (pos.y - cell.y);

	float heights[4];// the heights at the 4 corners of the given cell
	heights[SW] = gOutput[int2(cell.y , cell.x)].a;// bottom left or SW
	heights[SE] = gOutput[int2(cell.y, cell.x+1)].a;// bottom right
	heights[NW] = gOutput[int2(cell.y+1, cell.x)].a;// top left
	heights[NE] = gOutput[int2(cell.y+1, cell.x+1)].a;// top right

	// bilinearly interpolate the gradient based on the proximity to each corner of the cell 
	gradient.x = (heights[NE] - heights[NW]) * (1 - y) + (heights[SE] - heights[SW]) * y;
	gradient.y = (heights[SW] - heights[NW]) * (1 - x) + (heights[SE] - heights[NE]) * x;

	//Calculate height with bilinear interpolation of the heights @ each corner (z component = height)
	gradient.z = heights[NW] * (1 - x) * (1 - y) + heights[NE] * x * (1 - y) + heights[SW] * (1 - x) * y + heights[SE] * x * y;
	// height +=

	return gradient;
}
void initRadialEffect(int radius)// for radial erosion effect
{
	//const int radius = 4;
	//erosionBrushIndices = new int[mapSize * mapSize][];
	//radialFXIndices = new int *[resolution * resolution];// [radius * radius] ;
			//std::vector<int*> radInd;
	//erosionBrushWeights = new float[mapSize * mapSize][];
	//radialFXWeights = new float *[resolution * resolution];// [] ;
			//std::vector<float*> radWts;
	//
	int xOffsets[RADIUS * RADIUS * 4];// = int[radius * radius * 4];// distances from center of effect
	int yOffsets[RADIUS * RADIUS * 4];// = int[radius * radius * 4];
	float weights[RADIUS * RADIUS * 4];// = float[radius * radius * 4];// the effect's weight at the offset
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
	/*	radialFXIndices.push_back(new int[currentIndex]);*/
		//	erosionBrushWeights[i] = new float[numEntries];
			//radialFXWeights[i] = new float[radius * radius];//[numEntries];
	/*	radialFXWeights.push_back(new float[currentIndex]);*/
		//
		for (int j = 0; j < currentIndex; j++) {
			//		erosionBrushIndices[i][j] = (yOffsets[j] + centreY) * mapSize + xOffsets[j] + centreX;
					//radialFXIndices[i][j] = (yOffsets[j] + centerY) * resolution + xOffsets[j] + centerX;
			/*radialFXIndices.back()[j] = (yOffsets[j] + centerY) * resolution + xOffsets[j] + centerX;*/
			//		erosionBrushWeights[i][j] = weights[j] / weightSum;
					//radialFXWeights[i][j] = weights[j] / sumOfWeights;
			/*radialFXWeights.back()[j] = weights[j] / sumOfWeights;*/
		}
	}
}

/*
void InitializeBrushIndices(int mapSize, int radius) {
	erosionBrushIndices =  int[mapSize * mapSize][];
	erosionBrushWeights =  float[mapSize * mapSize][];

	int[] xOffsets = new int[radius * radius * 4];
	int[] yOffsets = new int[radius * radius * 4];
	float[] weights = new float[radius * radius * 4];
	float weightSum = 0;
	int addIndex = 0;

	for (int i = 0; i < erosionBrushIndices.GetLength(0); i++) {
		int centreX = i % mapSize;
		int centreY = i / mapSize;

		if (centreY <= radius || centreY >= mapSize - radius || centreX <= radius + 1 || centreX >= mapSize - radius) {
			weightSum = 0;
			addIndex = 0;
			for (int y = -radius; y <= radius; y++) {
				for (int x = -radius; x <= radius; x++) {
					float sqrDst = x * x + y * y;
					if (sqrDst < radius * radius) {
						int coordX = centreX + x;
						int coordY = centreY + y;

						if (coordX >= 0 && coordX < mapSize && coordY >= 0 && coordY < mapSize) {
							float weight = 1 - Mathf.Sqrt(sqrDst) / radius;
							weightSum += weight;
							weights[addIndex] = weight;
							xOffsets[addIndex] = x;
							yOffsets[addIndex] = y;
							addIndex++;
						}
					}
				}
			}
		}

		int numEntries = addIndex;
		erosionBrushIndices[i] = new int[numEntries];
		erosionBrushWeights[i] = new float[numEntries];

		for (int j = 0; j < numEntries; j++) {
			erosionBrushIndices[i][j] = (yOffsets[j] + centreY) * mapSize + xOffsets[j] + centreX;
			erosionBrushWeights[i][j] = weights[j] / weightSum;
		}
	}
}
*/