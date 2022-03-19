// depth ps
struct InputType
{
    float4 position : SV_POSITION;
    float4 depthPosition : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET
{
    float4 depthValue = float4(0, 0, 0, 1);
    // Get the depth value of the pixel by dividing the Z pixel depth by the homogeneous W coordinate.
    depthValue.x = input.depthPosition.z / input.depthPosition.w;

    if (depthValue.x >= 0.5)
    {
        //depthValue.x = 1;
    }
    return depthValue;
}
