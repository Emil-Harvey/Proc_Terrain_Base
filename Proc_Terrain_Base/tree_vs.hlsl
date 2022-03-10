
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
};

Texture2D HEIGHT : register(t0);// heightmap
SamplerState s0 : register(s0);

OutputType main(InputType input)
{
    OutputType output;

	 // Pass the vertex position etc into the geo shader.
    output.position = input.position;

    output.worldPosition = mul(output.position, worldMatrix);
    float alignment = 5.0;// align vertices to coordinate multiples of 5
    output.worldPosition.x = int(output.worldPosition.x / alignment) * alignment;
    output.worldPosition.z = int(output.worldPosition.z / alignment) * alignment;
    
    float4 heightmapSampled = HEIGHT.SampleLevel(s0, (output.worldPosition.xz / 11520.0) + 0.5, 0);
    output.worldPosition.y = heightmapSampled.a;



    output.tex = input.tex;
    output.normal = input.normal;
    




   // output.treeType = 0;
   // output.colourMultiplier = float4(1, 1, 1, 1);
    
    return output;
}