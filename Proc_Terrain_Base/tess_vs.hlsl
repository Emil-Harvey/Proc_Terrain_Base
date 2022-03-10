// Tessellation vertex shader.
// Doesn't do much, could manipulate the control points
// [does nothing except calculate a world position]

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;

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
    float4 worldPosition : POSITION1;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;

};

OutputType main(InputType input)
{
    OutputType output;

	 // Pass the vertex position etc into the hull shader.
    output.position = input.position;

    output.tex = input.tex;
    output.normal = input.normal;
    output.worldPosition = mul(output.position, worldMatrix);
    
    return output;
}