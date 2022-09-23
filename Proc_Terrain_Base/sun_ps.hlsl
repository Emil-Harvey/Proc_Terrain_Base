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

struct HSLcolour {// technically HSV
    float h;
    float s;
    float l;
    float3 toRGB() {
        float3 output = { 0,0,0 };
        //hue           min(, 3.14159)max(, 3.14159)
        while (h > 1.0) { h -= 1.0; }
        output.r = saturate(2 * cos(h * 3.14159));
        output.g = saturate(2 * sin(h * 3.14159));
        output.b = saturate(-2 * cos(h * 3.14159));
        //saturation
        //float grey = (output.r + output.g + output.b) / 3.0;
        output = lerp(float3(1.0, 1.0, 1.0), output, s);
        //brightness
        output.rgb *= l;
        return output;
    }
    HSLcolour toHSLcolour(float3 col) {
        HSLcolour ret;
        ret.l = length(col);
        if (col.r == col.g && col.g == col.b)  //  saturation
            ret.s = 0;
        else {
            //S = (Max(RGB) - Min(RGB)) / (1 - |2L - 1|)
            ret.s = ((max(col.r, max(col.g, col.b))) - (min(col.r, min(col.g, col.b)))) / (1 - abs(2 * ret.l - 1));
        }
        // hue

    //    (red/yellow)   If R >= G >= B | H = 60' x[(G - B) / (R - B)]
    //    (yellow/green) If G > R >= B | H = 60' x[2 - (R - B) / (G - B)]
    //    (green/cyan)   If G >= B > R | H = 60' x[2 + (B - R) / (G - R)]
    //    (cyan/blue)    If B > G > R | H = 60' x[4 - (G - R) / (B - R)]
    //    (blue/purple)  If B > R >= G | H = 60' x[4 + (R - G) / (B - G)]
    //    (purple/red)   If R >= B > G | H = 60' x[6 - (B - G) / (R - G)]

       //   alternatively...        // <this does not return a hue between 0 & 1>
        if (col.r > col.g && col.r > col.b)
            ret.h = 0.0 + (col.g - col.b) / ((max(col.r, max(col.g, col.b))) - (min(col.r, min(col.g, col.b))));// (max - min)
        else if (col.g > col.r && col.g > col.b)
            ret.h = 2.0 + (col.b - col.r) / ((max(col.r, max(col.g, col.b))) - (min(col.r, min(col.g, col.b))));
        else if (col.b > col.r && col.b > col.g)
            ret.h = 4.0 + (col.r - col.g) / ((max(col.r, max(col.g, col.b))) - (min(col.r, min(col.g, col.b))));

        return ret;
    }
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
    float sunAltitude = clamp(lengthOfDay - (cos(time / 3.81)/0.8), -1.0, 1.0);
    //float skyR = 0.7* (1.0+ pow(sunAltitude, 1.0 / 3.0));//max(-0.5 * (-(lengthOfDay - (cos(2.f * time / 3.81f)/0.8)) - 1),0);
    //float skyB = 0.5+sunAltitude;//max(0.5 * (sunAltitude + 1),0);
    //float skyG = max((skyB * 0.5) + (skyR * 0.5),min(skyR,skyB)+0.1); //pow((pow(skyR, 8) + skyB) / 1.532f, lerp(0.5, 8, lengthOfDay * 0.5 + 0.5));

    //
    HSLcolour highSkyHSL;
    highSkyHSL.h = 5.0;
    highSkyHSL.s = clamp(saturate(sunAltitude), 0.1, 0.63 + lengthOfDay * 0.2);
    highSkyHSL.l = max(saturate(sunAltitude), highSkyHSL.s);

    HSLcolour lowSkyHSL;
    lowSkyHSL.h = lerp(0.0, 4.0, saturate(sunAltitude));
    lowSkyHSL.s = clamp(saturate(-sunAltitude),0.32, 0.57);
    lowSkyHSL.l = max(saturate(sunAltitude),0.1);

   // const
        float3 highSkyColour = highSkyHSL.toRGB();// saturate(float3(max(skyR / 30.0,skyB/4.0), skyG / 20.0, skyB));
   // const 
        float3 lowSkyColour = lowSkyHSL.toRGB();//saturate(float3(skyR, skyG, skyB));
    
        highSkyColour.g = (highSkyColour.b * 0.45) + (highSkyColour.r * 0.45);
        lowSkyColour.g = (lowSkyColour.b * 0.45) + (lowSkyColour.r * 0.45);

    //const float G = pow(saturate(i.position.y * 0.001), 0.3);
    //const float R = pow(saturate(i.position.y * 0.001), 0.5) - 0.1;
    //return float4(R, G, 1.0f, 1.0f);      
    //              i.position.y * 0.001
    float sky_zenithness = i.position.y * 0.0005 + 0.5 - viewMatrix[1][2];// height of forward vector
    float3 sky = lerp(highSkyColour, lowSkyColour, pow(sky_zenithness,1.0));
    if (length(sky) < 0.5) {// if sky dark
        float g = sky;
        //display stars at night sky
        if (hash(i.position.x, i.position.y + 0.5 - viewMatrix[1][2]) == 1.0)
            g += -0.450 + 0.5*hash(i.position.x/100.0, i.position.y/100.0 + 0.5 - viewMatrix[1][2]);// make pixel brighter
        //sky = lerp(g, sky, saturate(length(sky) * 2.0));
    }
    return float4(sky, 1.0);//  

    }
    else {
        //sun
        return float4(1.0,0.98,0.89,1.0);
}
}