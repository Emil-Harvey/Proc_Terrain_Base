// Light vertex shader
// Standard issue vertex shader, apply matrices, pass info to pixel shader
cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};
cbuffer ChunkBuffer : register(b1)
{
    float2 chunkPosition;
    float scale;
    float time;
    int tessellationFactor;
    float3 manipulationDetails;

    float2 globalPosition;/// 
    float2 padding_;
};

struct InputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;

};

struct OutputType
{
    float4 position : POSITION;
    float4 world_position : TEXCOORD0;
    float2 tex : TEXCOORD1;
    float3 normal : NORMAL;

    float steepness : PSIZE0;
    float temperature : PSIZE6;
    float snowness : PSIZE1;
    float beachness : PSIZE2;
    float noise : PSIZE3;
    float noise2 : PSIZE4;
    float humidity : PSIZE5;
    float2 wind : TEXCOORD2;
};
float bfm(float4 coords, int octs);

OutputType main(InputType input)
{
	OutputType output;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = input.position;

    //output.position.x += chunkPosition.x;
    //output.position.z += chunkPosition.y;
    
	
    output.world_position = mul(output.position, worldMatrix);
	
    

	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;

	// Calculate the normal vector against the world matrix only and normalise.
	output.normal = mul(input.normal, (float3x3)worldMatrix);
	output.normal = normalize(output.normal);
		
    const float terrainSize = 800;
    const float planetDiameter = 2;
    const float yOffset = 0; // distance from world y0 to equator
    
    const float PI180 = 57.29577;
    const float PI = 1.0 / (PI180 / 180.0);
    const float2 uv = input.tex;
    const float altitude = output.world_position.y;
    const float latitude = (output.world_position.z + yOffset) / (terrainSize * planetDiameter) * PI;
    const float longditude = output.world_position.x; 
    const float4 coords = output.world_position + float4(400, 0, 324, 0);
    const float slope = input.normal.y;
    const float2 aspect = input.normal.xz;// compass direction of slope
    
	    // flat to cliff attenuation
    output.steepness = saturate(pow(saturate(slope) + 0.18 + (0.12 * bfm(scale * coords / (scale * 12), 4)), 100)); 
    
        // beachline
    const float beachline = 0.2; //altitude at which beaches meet soil
    output.beachness = saturate(slope) * saturate(2 * altitude - bfm(coords / (scale * 12), 4) - beachline);
    
     // PREVAILING WINDS VECTOR
    const float prevailingWindY = tan(2 * latitude) / (1.74 / abs(latitude * 0.3183)); // or something
    const float prevailingWindX = -tan(abs(1 *  latitude) + (3.14159 * 0.5)) / 5.0; //
    /*const float2 prevailingWind*/
    output.wind = normalize(float2(prevailingWindX, prevailingWindY)); //
    //output.world_position.y = prevailingWindX * 30;
    //float3 rightVector = float3(viewMatrix[0][0],
    //                            viewMatrix[1][0],
    //                            viewMatrix[2][0]);
    
    const float minGlobalTemp = -37.0;//`C
    const float maxGlobalTemp = 36.6;//`C
    output.temperature = lerp(minGlobalTemp, maxGlobalTemp, 
                                                        (0.5 + 0.5 * cos(latitude)) // cold @ poles hot @ equator
                                                        - (altitude / (23 * scale))//   cold @ high altitude
                                                        + dot(aspect.y, sin(latitude*0.5))) * 0.2;// cold on polar aspect slopes
    
    // snowline //height value - high near the middle (equator)
    const float snowline = (2 + 2.4 * cos(latitude)
                     + (cos(time / PI180) * -sin(latitude * 0.5)))
                     * scale;
    // this represents temperature/climate, specifically how cold
    output.snowness = saturate(0.5 / scale * (0.2 * altitude - bfm(coords / (scale * 2), 4) - snowline));
    //  apply deep snow effect 
    output.world_position.y += max(output.snowness * output.steepness - 0.5, 0) * scale;
    
    
    
    // relative humidity - usually high when cold & low at altitude
    output.humidity = saturate((abs(bfm(coords / (scale * 19), 2)) * 3
                                + (dot(output.wind, -aspect) / altitude) // rainshadow effect - it is more humid on slopes that face the wind
                                //+ altitude / (1 + output.temperature - minGlobalTemp) * 0.2
                                 + 3.75) / ((5.6 / maxGlobalTemp) * (output.temperature - minGlobalTemp) + 0.1)); // the lower the temperature, the more humidity is raised
    //output.humidity = pow((output.humidity - 0.5) * 1.75, 5) + 0.5;// shift most values closer to 0.5
    
    // noise to determine between plains and woodland, slightly higher chance of the former
    output.noise = pow(saturate(1 - bfm(coords / (scale * 10), 4)), 3); 
    
    // similar noise for beach pebbles, but offset by 99 so the noise does not line up
    output.noise2 = pow(saturate(1 - bfm(99 + coords / (scale * 12), 4)), 3); 
   //output.var = cos(output.world_position.x);

    
    
    
    output.position = mul(output.world_position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
	return output;
}

///////////////////////--------------------------//////////////////////////
///                          perlin noise!
///////////////////////--------------------------//////////////////////////



float interpolate(float a0, float a1, float w)
{
    return pow(w, 2.0) * (3.0 - 2.0 * w) * (a1 - a0) + a0;
} //*/

 

/* Create random direction vector
 */
float2 gradient(int x, int y)
{
    // makes the random gradients for the given grid coordinate
    float seed = 21732.37f;
    float random = (seed / 100) * sin(x * (seed / 11.0923) + y * (seed / 2.62349) + seed / 32.12379) * cos(x * (seed / 8.812934) * y + y * y * seed + (seed / 23.792173));
    //float random = sin(x + y) * cos(x * y);
    float2 ret;
    ret.x = (float) cos(random);
    ret.y = (float) sin(random);
    return ret;
}

// Computes the dot product of the distance and gradient vectors.
float dotGradient(int ix, int iy, float x, float y)
{
    // Get gradient from integer coordinates
    float2 grad = gradient(ix, iy);

    //distance to the given grid point 
    float xDistance = x - (float) ix;
    float yDistance = y - (float) iy;

    //dot-product of the distance & gradient vectors
    return (xDistance * grad.x + yDistance * grad.y);
}

float perlin(float4 world_position)
{
    float x = abs(world_position.x);
    float y = abs(world_position.z);
    
    int x0 = (int) x;
    int x1 = x0 + 1;
    int y0 = (int) y;
    int y1 = y0 + 1;

    // get interpolation weights based on proximity
    float xWeight = x - (float) x0;
    float yWeight = y - (float) y0;

    // interpolate between grid point gradients
    float n0, n1, i0, i1, value;

    n0 = dotGradient(x0, y0, x, y);
    n1 = dotGradient(x1, y0, x, y);
    i0 = interpolate(n0, n1, xWeight);

    n0 = dotGradient(x0, y1, x, y);
    n1 = dotGradient(x1, y1, x, y);
    i1 = interpolate(n0, n1, xWeight);

    value = interpolate(i0, i1, yWeight);
    return value;
}
float bfm(float4 coords, int octs)
{
    float val = 0;
    for (int o = 0; o < octs; o++)
    {
        val += perlin(coords * pow(2, o)) / pow(2, o);
    }
    return val;
}