
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};
/*  not even needed?
cbuffer ChunkBuffer : register(b1)
{
    float2 chunkPosition;
    float scale;
    float time;
    int tessellationFactor;
    float3 manipulationDetails;

    //float2 globalPosition;/// 
    //float2 padding_;
};*/
cbuffer posBuffer : register(b1)
{
    float3 camera_pos;
    float grid_alignment_scale;
}

struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float4 worldPosition : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    
    //float treeType : PSIZE0;
    //float4 colourMultiplier : COLOR0;
    float humidity : PSIZE5;
};

Texture2D HEIGHT : register(t0);// heightmap
SamplerState s0 : register(s0);


float noise(float2 xy) // gives value between 0 and 1, tends toward 0.5
{
    float seed = 74981.87126386f;//21732.37f;
    float random = seed * 9.01230387 + sin(xy.x + xy.y * seed) * cos(xy.x * xy.y + seed) * seed + xy.x + xy.x / xy.y + seed * xy.y - sin(seed - xy.y);
    //return fmod(random, 1);
    return 1.0 * sin(random) + 0.0;
}
float3 calculateNormal(float2 pos, float h = 5.0 / 5.0f)
{
     //calculate the tangent & bitangent using the heightMap    
    float3 tangent;
    float3 bitangent;
    const float2 ux = float2(pos.x - h, pos.y); //neighbour to left
    const float2 vx = float2(pos.x + h, pos.y); //neighbour to right
    float xdy = 1 * HEIGHT.SampleLevel(s0, (ux / 11520.0) + 0.5, 1).a - 1 * HEIGHT.SampleLevel(s0, (vx / 11520.0) + 0.5, 1).a;
    tangent = normalize(float3(2 * h, xdy, 0));
    // same for bitangent but in z dimension
    const float2 uz = float2(pos.x, pos.y - h); //behind
    const float2 vz = float2(pos.x, pos.y + h); //in front
    float zdy = 1 * HEIGHT.SampleLevel(s0, (uz / 11520.0) + 0.5, 1).a - 1 * HEIGHT.SampleLevel(s0, (vz / 11520.0) + 0.5, 1).a;
    bitangent = normalize(float3(0, zdy, -2 * h));

    return normalize(cross(tangent, bitangent));
}

OutputType main(InputType input)
{
    OutputType output;

	 // Pass the vertex position etc into the geo shader.
    output.position = input.position;

    output.worldPosition = mul(output.position, worldMatrix);
    float alignment = grid_alignment_scale;// align vertices to coordinate multiples of 5 or 15
    output.worldPosition.x = int(output.worldPosition.x / alignment) * alignment;
    output.worldPosition.z = int(output.worldPosition.z / alignment) * alignment;

    float jitter_radius = alignment * (3.0 / 5.0);// try to hide the grid-alignment look on generated foliage
    float2 jitter = { noise(output.worldPosition.xz + float2(0.4115,0.31271)) * jitter_radius, noise(output.worldPosition.zx + float2(0.4115,0.31271)) * jitter_radius };
    output.worldPosition.xz += jitter;
    
    float4 heightmapSampled = HEIGHT.SampleLevel(s0, (output.worldPosition.xz / 11520.0) + 0.5, 0);
    output.worldPosition.y = heightmapSampled.a;

    output.humidity = heightmapSampled.x;

    output.tex = input.tex;

    output.normal = calculateNormal(output.worldPosition.xz);//input.normal;
    




   // output.treeType = 0;
   // output.colourMultiplier = float4(1, 1, 1, 1);
    
    return output;
}