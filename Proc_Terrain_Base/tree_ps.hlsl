// Calculate diffuse lighting for a single directional light (also texturing)

Texture2D trees[4] : register(t0);


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

float4 main(InputType input) : SV_TARGET
{
    float4 textureColour;
    float4 lightColour;
    
    const float latitude = input.world_position.z;
    
   // const float temperature = 1 -((2 + 2.4 * cos(latitude * 0.0035) + (cos(160.0 / PI180) * -sin(latitude * 0.00175))));
    
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
    else
        textureColour = trees[3].Sample(s0, input.tex);
    
    // handle transparent pixels (from https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-clip )
    clip(textureColour.a < 0.623 ? -1 : 1);

    if (true && textureColour.g > (textureColour.r + textureColour.b) / 1.35) // if pixel is green
    {// grass dryness
        textureColour.g *= input.colourMultiplier.g;

    }
    textureColour.rgb *= textureColour.a;// avoid whitened edges


   
    LightInfo sunlight = calculateSunlightInfo(input.world_position);
    lightColour = calculateLighting(-sunlight.direction, input.normal, sunlight.colour);//calculateLighting(-lightDirection, input.normal + textureNormal, diffuseColour); 


    lightColour = (sunlight.ambient + lightColour);//calculateLighting(-lightDirection, input.normal, diffuseColour));
	
    return textureColour * lightColour;//* 2 * //
}