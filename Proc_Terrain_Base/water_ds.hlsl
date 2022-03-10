// Tessellation domain shader
// After tessellation the domain shader processes the all the vertices
// once per vertex

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};
cbuffer DataBuffer : register(b1)
{
    float2 pos;
    float scale;
    float time;
    int tessellationFactor;
    float3 manipulationDetails;
};
cbuffer CameraBuffer : register(b2)
{
    float3 viewpos;
    float padding;
};

struct ConstantOutputType
{
    float edges[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
};

struct InputType
{
    float4 position : POSITION;
    
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
   
    //float amplitude : PSIZE0;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float4 world_position : TEXCOORD0;
    float2 tex : TEXCOORD1;
    float3 normal : NORMAL;
    
};
float bfm(float2 pos, int octaves, float speed = 1.0);

float3 calculateNormal(float2 pos, float h = 1.0 / 75.0f)
{
    /*//float h = 1.0 / 200.0f; // 1/size of plane/quad (arbitrary)
        //calculate the tangent & bitangent using the heightMap    
    float3 tangent;
    float3 bitangent;
    const float2 ux = float2(pos.x - h, pos.y); //neighbour to left
    const float2 vx = float2(pos.x + h, pos.y); //neighbour to right
    float xdy = manipulationDetails.x * bfm((ux + 800) / 50.0, 4) - manipulationDetails.x * bfm((vx + 800) / 50.0, 4);
    tangent = normalize(float3(2 * h, xdy, 0));
        // same for binormal but in z dimension
    const float2 uz = float2(pos.x, pos.y - h); //behind
    const float2 vz = float2(pos.x, pos.y + h); //in front
    float zdy = manipulationDetails.x * bfm((uz + 800) / 50.0, 4) - manipulationDetails.x * bfm((vz + 800) / 50.0, 4);
    bitangent = normalize(float3(0, zdy, -2 * h));
    
    return normalize(cross(tangent, bitangent));//*/
    int octaves = int(lerp(2.5, 1, saturate(length(viewpos.xz - pos) / 595.0)));
    
   // float h = 1.0 / 75.0f; // 1/size of plane/quad (arbitrary)
    
    //calculate the tangent & bitangent using the heightMap    
    float3 tangent;
    float3 bitangent;
    const float2 ux = float2(pos.x - h, pos.y) + 800; //neighbour to left
    const float2 vx = float2(pos.x + h, pos.y) + 800; //neighbour to right
    float xdy = 0.75 * bfm(ux / 2.0, octaves, 34.0) - 0.75 * bfm(vx / 2.0, octaves, 34.0);
    tangent = normalize(float3(2 * h, xdy, 0));
    // same for binormal but in z dimension
    const float2 uz = float2(pos.x, pos.y - h) + 800; //behind
    const float2 vz = float2(pos.x, pos.y + h) + 800; //in front
    float zdy = 0.75 * bfm(uz / 2.0, octaves, 34.0) - 0.75 * bfm(vz / 2.0, octaves, 34.0);
    bitangent = normalize(float3(0, zdy, -2 * h));
    
    return normalize(cross(tangent, bitangent));
}

[domain("quad")]
OutputType main(ConstantOutputType input, float2 uvwCoord : SV_DomainLocation, const OutputPatch<InputType, 4> patch)// patch should be 3 for tri, 4 for quad etc
{
    float3 vertexPosition;
    OutputType output;
 
    // Determine the position of the new vertex.
	// bilinear interpolation: 
    vertexPosition = lerp( // lerp the "horizontal" positions by the "vertical" one
                        lerp(patch[0].position, patch[1].position, uvwCoord.y),
                        lerp(patch[3].position, patch[2].position, uvwCoord.y),
                        uvwCoord.x);
    output.position = float4(vertexPosition, 1.0f);
    
    output.tex = uvwCoord;//vertexPosition.xz / 30; //
    
    // calculate un-displaced normals
    output.normal = lerp( // bilinear interpolation
                        lerp(patch[0].normal, patch[1].normal, uvwCoord.y),
                        lerp(patch[3].normal, patch[2].normal, uvwCoord.y),
                        uvwCoord.x);
    
    //output.amplitude = patch[0].amplitude;
    const float sealevel = 0.02f;
    ///*
    // Calculate the position of the new vertex against the world, view, and projection matrices. or do this in geo shader
    output.world_position = mul(float4(vertexPosition, 1.0f), worldMatrix); //float4(vertexPosition, 1.0f);  
    output.world_position.y += sealevel + manipulationDetails.x * bfm((output.world_position.xz + 800) / 50.0, 3, 14.0); // big waves (wind/storm waves)
    output.world_position.y += manipulationDetails.x * 0.75 * bfm((float2(output.world_position.x, output.world_position.z /3.0) + 800) / 2.0, 2, 34.0); // chop/capillary waves
    

    
    
    //                               Send the normal, light into the ps
    if (manipulationDetails.x != 0)
    {
        output.normal = calculateNormal(output.world_position.xz);
    // smooth normal by averaging neighbours
        //output.normal += calculateNormal(output.world_position.xz, 0.05f);
        //output.normal += calculateNormal(output.world_position.xz - 0.02f);
        //output.normal += calculateNormal(float2(output.world_position.x + 0.02f, output.world_position.z - 0.02f));
        //output.normal += calculateNormal(output.world_position.xz + 0.02f);
        //output.normal += calculateNormal(float2(output.world_position.x - 0.02f, output.world_position.z + 0.02f));
        //output.normal = normalize(output.normal);
        output.normal.r = -output.normal.r;//*/
    }
    else // this mesh is not being manipulated, so keep the normals as they were
    {
    // calculate un-displaced normals
        output.normal = lerp( // bilinear interpolation
                    lerp(patch[0].normal, patch[1].normal, uvwCoord.y),
                    lerp(patch[3].normal, patch[2].normal, uvwCoord.y),
                    uvwCoord.x);
    }
    
    ////fish-eye effect?
    //output.world_position.y -= length(viewpos.xz - output.world_position.xz);
    
       output.position = mul(output.world_position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
//*/
    return output;
}


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

float perlin(float2 world_position, float speed)
{
    float x = abs(world_position.x) + time * speed;
    float y = abs(world_position.y) + 0.3291f * time * speed;
    
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
float ridges(float2 coords, float s)
{
    return (1 - (4 * abs(perlin(coords, s)))) * perlin(coords.yx, s);
}

float bfm(float2 coords, int octs, float s)
{
    float val = 0;
    for (int o = 0; o < octs; o++)
    {
        val += ridges(coords * pow(2, o), s) / pow(2, o);
    }
    return val;
}
///#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#///