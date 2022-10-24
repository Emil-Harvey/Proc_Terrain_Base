//float4 main() : SV_TARGET
//{
//	float density = worley(position.xz);
//	cull(density)
//	return 
//}

//cbuffer DataBuffer : register(b2)
//{
//    float2 chunkPos;
//    float scale;
//    float timeOfYear;
//    int tessellationFactor;
//    float3 manipulationDetails;
//};


// Texture and sampler registers
Texture2D texture0 : register(t0);
SamplerState Sampler0 : register(s0);

cbuffer timeBuffer: register(b0) {
    float timeP;
    float timeOfYear;
    int day;
    float padding;
}
/*
cbuffer CameraBuffer : register(b1)
{
    //float3 viewpos;
    //float pudding;
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};
*/

struct InputType
{
    float4 position : SV_POSITION;
    float4 world_position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

// functions
float bfm(float2 pos, int octaves, float speed = 1.0);

struct LightInfo {
    float3 direction;
    float4 colour;
    float4 ambient;
};

LightInfo calculateSunlightInfo(float4 coords) {
    const float latitude = 0.0f; // -(coords.z + globalPosition.y) / (planetDiameter * 3.14159265359);
    const float longditude = 0.0f; // (coords.x + globalPosition.x) / (planetDiameter * 3.14159265359);
    const float AxialTilt = 0.10;
    // this is a relative value rather than absolute (add 12 for abs)
    const float lengthOfDay = -12 * -sin(latitude) * cos(radians(timeOfYear));// replace -6 [-1] w/ -12*sin(latitude)  <--- to be moved to VS
    const float time = 24 * (timeOfYear - int(timeOfYear) + (longditude / 3.14159265));//
    const float sunAltitude = lengthOfDay - (cos(time / 3.81) / AxialTilt);
    const float skyR = saturate(-0.5 * (-(lengthOfDay - (cos(time / 3.81f) / AxialTilt)) - 1));
    const float skyB = saturate(-0.5 * (-sunAltitude - 1));
    //const float skyG = pow((pow(skyR, 8) + skyB) / 1.532f, lerp(0.5, 8, lengthOfDay * 0.5 + 0.5));

    LightInfo li;

    //calculate sun colour and position (red at night, white at day)
    //sin((time / 3.81) + 1.78) / (1.75 + sin(timeOfYear / 57.29577))
    li.direction = float3(sin(time / 3.81), -0.0833333 * sunAltitude, -sin(AxialTilt * cos(timeOfYear / 57.29577) + latitude));
    //light->setDiffuseColour(lerp(0.6, 0.9, -0.5 * (cos(time / 3.81) - 1)), lerp(0.14, 0.85, -0.5 * (cos(time / 3.81) - 1)), lerp(0.01, 0.8, -0.5 * (cos(time / 3.81) - 1)), 1.0f);
    li.colour = float4(pow(max(skyR, skyB + 0.5), 0.30), pow(max(skyB, 0.5 * (skyR + skyB)), 0.30), skyB, 1.0f);
    li.colour.r = min(li.colour.g * 1.75, li.colour.r);

    li.ambient = 0.048 * li.colour + 0.03f;
    li.ambient.b = max(li.ambient.b, li.ambient.r);
    //li.colour = 0;
    return li;
}

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLighting(float3 lightDirection, float3 normal, float4 diffuse)
{
    float intensity = saturate(dot(normal, lightDirection));
    float4 colour = saturate(diffuse * intensity + 0.1);
    colour.b = max(colour.b, 0.2);
    return colour;
}

float4 main(InputType i) : SV_TARGET
{
    float4 textureColour2 = texture0.Sample(Sampler0, 0.31 * i.tex.yx + float2(-timeOfYear/6.7, timeOfYear /10.37f));
    float noise2 = bfm(i.tex / 1.80f, 4, 0.736f) * length(textureColour2);
    
    float4 textureColour = texture0.Sample(Sampler0, i.tex + float2((noise2*.07f) +timeOfYear, timeOfYear/13.07f));

    float noise = bfm(i.tex * 6.03f, 4, 3.576f) + 0.8;

    textureColour.a = ( (length(textureColour.rgb) + noise) * noise2 );
    textureColour.rgb = textureColour.a;

    // fade with distance
    const float2 center_offset = float2(7.0, 7.0);
    textureColour.rgba = saturate( textureColour.a - pow(length(i.world_position.xz - center_offset) / 7.0f, 2.0) );

    clip(textureColour.a < 0.0623 ? -1 : 1);

    // add light from sun/moon
    LightInfo sunlight = calculateSunlightInfo(i.world_position);
    const float4 lightColour = calculateLighting(-sunlight.direction, float3(0,1,0), sunlight.colour) + sunlight.ambient; 

    // finished
    return textureColour * lightColour; //float4(1.0,0.98,0.89,1.0);

}

///// perlin /////

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
    float random = seed * 9.01230387 + sin(x + y * seed) * cos(x * y + seed) * seed + x + x / y + seed * y - sin(seed - y);
    float2 ret;
    ret.x = (float)cos(frac(random));
    ret.y = (float)sin(random);
    return ret;
}

// Computes the dot product of the distance and gradient vectors.
float dotGradient(int ix, int iy, float x, float y)
{
    // Get gradient from integer coordinates
    float2 grad = gradient(ix, iy);

    //distance to the given grid point 
    float xDistance = x - (float)ix;
    float yDistance = y - (float)iy;

    //dot-product of the distance & gradient vectors
    return (xDistance * grad.x + yDistance * grad.y);
}

float perlin(float2 world_position, float speed)
{
    float x = abs(world_position.x) + timeOfYear * speed;
    float y = abs(world_position.y) + 0.3291f * timeOfYear * speed;

    int x0 = (int)x;
    int x1 = x0 + 1;
    int y0 = (int)y;
    int y1 = y0 + 1;

    // get interpolation weights based on proximity
    float xWeight = x - (float)x0;
    float yWeight = y - (float)y0;

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


float bfm(float2 coords, int octs, float s)
{
    float val = 0;
    for (int o = 0; o < octs; o++)
    {
        val += perlin(coords * pow(2, o), s) / pow(2, o);
    }
    return val;
}