//cbuffer DataBuffer : register(b2)
//{
//    float2 chunkPos;
//    float scale;
//    float timeOfYear;
//    int tessellationFactor;
//    float3 manipulationDetails;
//};

cbuffer timeBuffer: register(b0) {
    float timeP;
    float timeOfYear;
    int day;
    float padding;
}
cbuffer CameraBuffer : register(b1)
{
    //float3 viewpos;
    //float pudding;
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};
    

struct InputType
{
    float4 position : SV_POSITION;
    float4 world_position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

float hash(float x, float y)// random value betwn -1 & 1
{
    // makes the random gradients for the given grid coordinate
    float seed = 21732.37f;
    //float random = (seed / 100) * sin(x * (seed / 11.0923) + y * (seed / 2.62349) + seed / 32.12379) * cos(x * (seed / 8.812934) * y + y * y * seed + (seed / 23.792173));
    float random = seed * 9.01230387 + sin(x + y * seed) * cos(x * y + seed) * seed + x + x / y + seed * y - sin(seed - y);// ^ was producing artefacts
    //float2 ret;
   // ret.x = (float)cos(random);
   // ret.y = (float)sin(random);
    return (float)cos(random);// ret;
}


float4 main(InputType i) : SV_TARGET
{
    if(timeOfYear >= 0){
        // fair amount of unnecesary calculations here for the sake of understandable

    float time = 24 * timeP;//(timeOfYear - day);
    float lengthOfDay = -1 * cos(timeOfYear / 57.29577);// replace -6 [-1] w/ -12*sin(latitude) 
    //////
    float sunAltitude = lengthOfDay - (cos(time / 3.81)/0.8);
    float skyR = max(-0.5 * (-(lengthOfDay - (cos(2.f * time / 3.81f)/0.8)) - 1),0);
    float skyB = max(-0.5 * (-sunAltitude - 1),0);
    float skyG = pow((pow(skyR, 8) + skyB) / 1.532f, lerp(0.5, 8, lengthOfDay * 0.5 + 0.5));


    const float3 lowSkyColour = saturate(float3((skyR), skyG, skyB));
    const float3 highSkyColour = saturate(float3(max(skyR / 30.0,skyB/4.0), skyG / 20.0, skyB));
   
    //const float G = pow(saturate(i.position.y * 0.001), 0.3);
    //const float R = pow(saturate(i.position.y * 0.001), 0.5) - 0.1;
    //return float4(R, G, 1.0f, 1.0f);      
    //              i.position.y * 0.001
    float sky_zenithness = i.position.y * 0.0005 + 0.5 - viewMatrix[1][2];// height of forward vector
    float3 sky = lerp(highSkyColour, lowSkyColour, pow(sky_zenithness,3.0));
    if (length(sky) < 0.5) {// if sky dark
        float g = sky;
        //display stars at night sky
        if (hash(i.position.x, i.position.y + 0.5 - viewMatrix[1][2]) == 0.0)
            g += 0.60 + 0.25*hash(i.position.x, i.position.y + 0.5 - viewMatrix[1][2]);// make pixel brighter
        sky = lerp(g, sky, saturate(length(sky) * 2.0));
    }
    return float4(sky, 1.0);//  

    }
    else {
        //sun
        return float4(1.0,0.98,0.89,1.0);
}
}