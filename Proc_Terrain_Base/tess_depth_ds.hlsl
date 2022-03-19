// Tessellation domain shader
// After tessellation the domain shader processes the all the vertices
// once per vertex
Texture2D heightMap : register(t0);
SamplerState sam : register(s0);

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
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

    float amplitude : PSIZE0;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float4 depthPosition : TEXCOORD0;
};

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

    // Calculate the position of the new vertex against the world, view, and projection matrices. apply heightmap
    output.position = mul(float4(vertexPosition, 1.0f), worldMatrix);
    output.position.y += patch[0].amplitude * (heightMap.SampleLevel(sam, vertexPosition.xz / 30, 0) - 0.5); /////-
    // 
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // Store the position value in a second input value for depth value calculations.
    output.depthPosition = output.position;

    return output;
}
