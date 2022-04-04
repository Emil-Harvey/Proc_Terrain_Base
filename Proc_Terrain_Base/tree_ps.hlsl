// Calculate diffuse lighting for a single directional light (also texturing)

Texture2D macro : register(t0); // macro texture variation
Texture2D trees[4] : register(t1);


SamplerState s0 : register(s0);

cbuffer LightBuffer : register(b0)
{
    float4 ambient;
    float4 diffuseColour;
    float3 lightDirection;
    float padding;
};
cbuffer ChunkBuffer : register(b1)
{
    float2 chunkPosOffset;
    float scale;
    float timeOfYear;
    int tessellationFactor;
    //float3 manipulationDetails;
    float2 globalPosition;
    float planetDiameter;
   // float padding_;
};


struct InputType
{
    float4 position : SV_POSITION;
    float4 world_position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float treeType : PSIZE0;
    float4 colourMultiplier : COLOR0;
};

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLighting(float3 lightDirection, float3 normal, float4 diffuse)
{
    float intensity = saturate(dot(normal, lightDirection));
    float4 colour = saturate(diffuse * intensity + ambient);
    return colour;
}
struct LightInfo {
    float3 direction;
    float4 colour;
    float4 ambient;
};
LightInfo calculateSunlightInfo(float4 coords) {
    const float latitude = -(coords.z + globalPosition.y) / (planetDiameter * 3.14159265359);
    const float longditude = (coords.x + globalPosition.x) / (planetDiameter * 3.14159265359);
    const float AxialTilt = 0.10;
    // this is a relative value rather than absolute (add 12 for abs)
    float lengthOfDay = -12 * -sin(latitude) * cos(timeOfYear / 57.29577);// replace -6 [-1] w/ -12*sin(latitude)  <--- to be moved to VS
    float time = 24 * (timeOfYear - int(timeOfYear) + (longditude / 3.14159265));//
    float sunAltitude = lengthOfDay - (cos(time / 3.81) / AxialTilt);
    float skyR = saturate(-0.5 * (-(lengthOfDay - (cos(time / 3.81f) / AxialTilt)) - 1));
    float skyB = saturate(-0.5 * (-sunAltitude - 1));
    float skyG = pow((pow(skyR, 8) + skyB) / 1.532f, lerp(0.5, 8, lengthOfDay * 0.5 + 0.5));

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
float noise(float2 xy) {
    // makes the random gradients for the given grid coordinate
    float seed = 74981.87126386f;//21732.37f;
    float random = seed * 9.01230387 + sin(xy.x + xy.y * seed) * cos(xy.x * xy.y + seed) * seed + xy.x + xy.x / xy.y + seed * xy.y - sin(seed - xy.y);
    //return fmod(random, 1);
    return 0.5 * sin(random) + 0.5;
}

float4 main(InputType input) : SV_TARGET
{
    float4 textureColour;
    float4 lightColour;
    
    const float latitude = input.world_position.z;
    
   // const float temperature = 1 -((2 + 2.4 * cos(latitude * 0.0035) + (cos(160.0 / PI180) * -sin(latitude * 0.00175))));
    float3 normal = input.normal;
	// Sample the texture. Calculate light intensity and colour, return light*texture for final pixel colour.
   // decide which tree is here
    if (input.treeType == 0)
    {   textureColour = trees[0].Sample(s0, input.tex);
        if (textureColour.g > (textureColour.r + textureColour.b)/1.35) // if pixel is green
        {
            textureColour.g *= input.colourMultiplier;
        }
    }
    else if (input.treeType == 1)
        textureColour = trees[1].Sample(s0, input.tex);
    else if (input.treeType < 2.9)
        textureColour = trees[2].Sample(s0, input.tex);
    else {
        textureColour = trees[3].Sample(s0, input.tex);
        normal += 1-trees[2].Sample(s0, input.tex);// + billboard normal
    }

    // handle transparent pixels (from https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-clip )
    // use noise dithering
    half thres = 0.623 + 0.15 * noise(float2(input.tex.x * 14, input.tex.y));
    clip(textureColour.a < thres ? -1 : 1);


    if (input.treeType == 0 && textureColour.g > (textureColour.r + textureColour.b) / 1.35) // if pixel is green grass
    {
        float coefficient = (macro.Sample(s0, input.tex * 0.08).x + 0.5) *(macro.Sample(s0, input.colourMultiplier.zw * 0.002).x + 0.5) * (macro.Sample(s0, input.colourMultiplier.zw * 0.05).x + 0.5) * (macro.Sample(s0, input.colourMultiplier.zw * 0.2).x + 0.5);
        textureColour.xyz *= lerp(float3(0.52, 0.51, 0.50), float3(1, 1, 1), coefficient); //       MACRO TEXTURE VARIATION ~
    
     // grass dryness
        textureColour.g *= input.colourMultiplier.g;

    }
    //textureColour.rgb *= textureColour.a;// avoid whitened edges <-- fix the textures!


   //  Lighting
    LightInfo sunlight = calculateSunlightInfo(input.world_position);
    lightColour = calculateLighting(-sunlight.direction, normal, sunlight.colour);//calculateLighting(-lightDirection, input.normal + textureNormal, diffuseColour); 

    
    lightColour = (sunlight.ambient + lightColour);//calculateLighting(-lightDirection, input.normal, diffuseColour));
	
    return textureColour * lightColour;//* 2 * //
}