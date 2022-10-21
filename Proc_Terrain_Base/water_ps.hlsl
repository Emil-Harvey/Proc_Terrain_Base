Texture2D tWater : register(t0); // water
SamplerState s0 : register(s0);

cbuffer LightBuffer : register(b0)
{
    float4 ambient;
    float4 diffuseColour;
    float3 lightDirection;
    float specular;
};

cbuffer CameraBuffer : register(b1)
{
    float3 viewpos;
    float time;
};

struct InputType
{
    float4 position : SV_POSITION;
    float4 world_position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLighting(float3 lightDirection, float3 normal, float4 diffuse)
{
    float intensity = saturate(dot(normal, lightDirection));
    float4 colour = saturate(diffuse * intensity + ambient);
    return colour;
}
float4 calculateSpecular(float3 lightDirection, float3 normal, float4 ldiffuse, float3 view)
{
    float3 vHalf = normalize(view + lightDirection);
    float intensity = saturate(pow(dot(normal, vHalf), specular *30.0));
    float4 colour = saturate(ldiffuse * intensity);
    return colour;
}
float bfm(float2 pos, int octaves, float speed = 1.0);///
float perlin(float2 world_position, float speed);

float3 calculateNormals(float2 pos)//,float h = 1.0 / 200.0f)
{
    int octaves = int(lerp(3.5, 1, saturate(length(viewpos.xz - pos) / 495.0)));
    
    float h = 1.0 / 97.0f; // 1/size of plane/quad (arbitrary)
    
    //calculate the tangent & bitangent using the heightMap    
    float3 tangent;
    float3 bitangent;
    const float2 ux = float2(pos.x - h, pos.y) + 800; //neighbour to left
    const float2 vx = float2(pos.x + h, pos.y) + 800; //neighbour to right
    float xdy =  0.75 * bfm(ux / 2.0, octaves, 34.0) - 0.75 * bfm(vx / 2.0, octaves, 34.0);
    //float xdy = 0.75 * perlin(ux / 2.0, 34.0) - 0.75 * perlin(vx / 2.0, 34.0);
    tangent = normalize(float3(2 * h, xdy, 0));
    // same for binormal but in z dimension
    const float2 uz = float2(pos.x, pos.y - h) + 800; //behind
    const float2 vz = float2(pos.x, pos.y + h) + 800; //in front
    float zdy = 0.75 * bfm(uz / 2.0, octaves, 34.0) - 0.75 * bfm(vz / 2.0, octaves, 34.0);
    //float zdy = 0.75 * perlin(uz / 2.0,  34.0) - 0.75 * perlin(vz / 2.0, 34.0);
    bitangent = normalize(float3(0, zdy, -2 * h));
    
    return normalize(cross(tangent, bitangent));
    
}

float4 main(InputType input) : SV_TARGET
{

    float dist = length(input.world_position.xz - viewpos.xz);// nice little dither fade of render distance
    const float clip_near = 18000.0;
    const float fade_dist = 2500.0;
    clip(dist > clip_near ? ((clip_near + fade_dist - dist) / fade_dist) + (((input.position.x % 2.5) - 1.25) * ((input.position.y % 2.5) - 1.25)) : 1);

    float2 uv = input.tex;
    //const float altitude = input.world_position.y;
    //const float latitude = input.world_position.z;
   // const float longditude = input.world_position.x;
   // const float slope = input.normal.y;
    
    //MaterialSample currentMat;
    //float4 textureHeight = 0;
    float4 textureColour = 0;
    float3 textureNormal = 0;
    float4 textureShine = 0;
    float4 lightColour;
	// Sample the texture. Calculate light intensity and colour, return light*texture for final pixel colour.
    
    // texture the mesh based on parameters
    
    //altitude
    textureColour = float4(max(0 / 7.0, 0), max(0 / 7.0, 0.02), max((0 + 3.0) * 0.05, 0.05), 1.0f); //((1 + tWater.Sample(s0, input.tex * 20))/3.0f) * tWater.Sample(s0, input.tex * 15000);
    textureNormal = calculateNormals(float2(input.world_position.x, input.world_position.z)); // pixel based lighting
    //textureNormal = input.normal;   //polygon based lighting          //nWater.Sample(s0, input.tex * 15000);
    //textureNormal.r *= -1;
    //textureNormal = normalize(textureNormal);
    textureShine = 0.60f;
    
    lightColour = calculateLighting(-lightDirection, textureNormal, diffuseColour);
	
    float3 view = normalize(viewpos - input.world_position.xyz);
    // simulate critical angle
    float a = saturate(abs(4.0 * dot(view, textureNormal))); //float3(0, 1, 0)
    textureColour.rgb += 0.16 * (1 - a);// add lightness
    
    //return float4(input.normal, 1.0f);
    //return float4(textureNormal, 1.0f);
    //float p = saturate(0.5*perlin(input.world_position.xz, 1)+0.5);//return float4 (p, (pow(1 - p, 30)) + 0.5, 1 - p, 1.0);
    float4 pixel = textureColour * lightColour + (lightColour * textureShine.x * calculateSpecular(-lightDirection, textureNormal, diffuseColour, view));
    return float4(pixel.xyz, 0.45 + 0.3 * (1- saturate(dot(view, textureNormal))));
}


//                              perlin
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
    //float random = (seed / 100) * sin(x * (seed / 11.0923) + y * (seed / 2.62349) + seed / 32.12379) * cos(x * (seed / 8.812934) * y + y * y * seed + (seed / 23.792173));
    float random = seed * 9.01230387 + sin(x + y * seed) * cos(x * y + seed) * seed + x + x/y + seed * y - sin(seed - y);// ^ was producing artefacts
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