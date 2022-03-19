// Tessellation Hull Shader
// Prepares control points for tessellation


cbuffer DataBuffer : register(b0)
{
    float2 pos;
    float scale;
    float time;
    int tessellationFactor;
    float3 manipulationDetails;
    float2 globalPosition;/// careful, water shader wont work anymore
    float2 padding_;
};
cbuffer CameraBuffer : register(b1)
{
    float3 viewpos;
    float padding; //
};
cbuffer MatrixBuffer : register(b2)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};
struct InputType
{
    float4 position : POSITION;
    float4 worldPosition : POSITION1;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    //float4 lightViewPos : TEXCOORD1;
};

struct ConstantOutputType
{
    float edges[4] : SV_TessFactor;
    float insides[2] : SV_InsideTessFactor;
};

struct OutputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float heightMapAmplitude : PSIZE0; //one of the only float semantics, simpler to pass here than use another buffer
    
};
float calculateLOD(float3 v)// inspired by frank luna
{
    float viewangle = 1- saturate(dot(float3(viewMatrix[0][2], 0, viewMatrix[2][2]), normalize(v - viewpos)));
    float dist =  sqrt(length(v - viewpos)); // how near the cam is
    float LOD = viewangle * saturate(( dist - manipulationDetails.y) / (manipulationDetails.y - manipulationDetails.z)); // (dist - near threshold) / (near threshold - far theshold)
    return pow(2, lerp(tessellationFactor, 0, LOD));
    //tesselation factor will be 2^n, where 0 >= n <= 6 & is based on the distance of the patch (maxing out at 250, with no tess, minimum at 40, with 2^6=64 tes'ns) 
}

ConstantOutputType PatchConstantFunction(InputPatch<InputType, 4> inputPatch, uint patchId : SV_PrimitiveID)
{
    ConstantOutputType output;
    

    float3 edgePositions[4];
    edgePositions[0] = (inputPatch[0].worldPosition + inputPatch[1].worldPosition) / 2; // the Western edge of the quad
    edgePositions[1] = (inputPatch[0].worldPosition + inputPatch[3].worldPosition) / 2; // north edge
    edgePositions[2] = (inputPatch[2].worldPosition + inputPatch[3].worldPosition) / 2; // east edge
    edgePositions[3] = (inputPatch[1].worldPosition + inputPatch[2].worldPosition) / 2; //south edge

    float tessFactors[5] =
    {
        calculateLOD(edgePositions[0]),
                            calculateLOD(edgePositions[1]),
                            calculateLOD(edgePositions[2]),
                            calculateLOD(edgePositions[3]),
        calculateLOD((edgePositions[0] + edgePositions[2]) / 2)
    };
    // based on distance to camera, set the tessellation factors for the edges of the quad //triangle. (cannot loop or use conditional statements)
    output.edges[0] = tessFactors[0]; //tessellationFactor;
    output.edges[1] = tessFactors[1]; //tessellationFactor;
    output.edges[2] = tessFactors[2]; //tessellationFactor;
    output.edges[3] = tessFactors[3]; //tessellationFactor;
    
    // Set the tessellation factor for tessallating inside the quad 
    output.insides[0] = tessFactors[4]; // tessellationFactor;//
    output.insides[1] = output.insides[0]; //tesselationFactor;

    return output;
}


[domain("quad")] //tri, quad, isocline
[partitioning("fractional_even")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)] // the number of times the shader executes. 1 ctrl pt per output
[patchconstantfunc("PatchConstantFunction")]
OutputType main(InputPatch<InputType, 4> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)// this bit is like the vertex shader
{
    OutputType output;


    // Set the position for this control point as the output position.
    output.position = patch[pointId].position;

    // Set the input as the output.
    //output.colour = patch[pointId].colour;
    output.normal = patch[pointId].normal;
    output.tex = patch[pointId].tex;
    
    //output.heightMapAmplitude = manipulationDetails.x;// keep? no

    return output;
}