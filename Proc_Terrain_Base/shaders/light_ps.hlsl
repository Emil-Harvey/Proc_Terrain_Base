// lighting pixel shader for terrain
// Calculate diffuse lighting for a single directional light (also texturing)

struct Material
{
    Texture2D albedo_specular;
    Texture2D normal_height;
    float UVscale; //= 1;
};
struct MaterialSample
{
    float4 albedo_specular;
    float4 normal_height;
};

struct HSLcolour {
    float h;
    float s;
    float l;
    float3 toRGB() {
        float3 output = { 0,0,0 };
        //hue           min(, 3.14159)max(, 3.14159)
        while (h > 1.0) {h-=1.0;}
        output.r = saturate(2*cos(h * 3.14159));
        output.g = saturate(2*sin(h * 3.14159));
        output.b = saturate(-2*cos(h * 3.14159));
        //saturation
        //float grey = (output.r + output.g + output.b) / 3.0;
        output = lerp(float3(1.0, 1.0, 1.0), output, s);
        //brightness
        output.rgb *= l;
        return output;
    }
    
};
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
}

// albedo & specular
Texture2D tGrass : register(t0);// grass
Texture2D tCliff : register(t1);// low cliff
Texture2D tSnow : register(t2);// snow
Texture2D tRock : register(t3);// high cliff
Texture2D tWater : register(t4);// water
Texture2D tSand : register(t5);// sand
Texture2D tGrass2 : register(t6);// grass2
Texture2D tStone : register(t7); // sea stone / gravel
Texture2D tSavan : register(t8);
// normal & height
Texture2D nGrass : register(t9); // grass
Texture2D nCliff : register(t10); // low cliff
Texture2D nSnow : register(t11); // snow
Texture2D nRock : register(t12); // high cliff
Texture2D nWater : register(t13); // water
Texture2D nSand : register(t14); // sand
Texture2D nGrass2 : register(t15); // grass2
Texture2D nStone : register(t16); // sea stone / gravel
Texture2D nSavan : register(t17);
//...

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
    float padding;
};

cbuffer DataBuffer : register(b2)
{
    float2 chunkPosOffset;
    float scale;
    float timeOfYear;
    int tessellationFactor;
    float3 manipulationDetails;
    float2 globalPosition;
    float planetDiameter;
    float padding_;
};

struct InputType
{
    float4 position : SV_POSITION;
    float4 world_position : POSITION;
    /*noperspective*/ float2 texX : TEXCOORD0;
    /*noperspective*/ float2 texY : TEXCOORD1;
    /*noperspective*/ float2 texZ : TEXCOORD2;
    float3 blendweights : NORMAL0;
    float3 normal : NORMAL1;

    float steepness : PSIZE0;
    float temperature : PSIZE6;
    float snowness : PSIZE1;
    float beachness : PSIZE2;
    float noise : PSIZE3;
    float noise2 : PSIZE4;
    float humidity : PSIZE5;
    float2 wind : TEXCOORD3;
};
float bfm(float4 coords, int octs);

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLighting(float3 lightDirection, float3 normal, float4 diffuse)
{
	float intensity = saturate(dot(normal, lightDirection));
	float4 colour = saturate(diffuse * intensity + ambient);
    colour.b = max(colour.b, 0.2);
	return colour;
}
float4 calculateSpecular(float3 lightDirection, float3 normal, float4 ldiffuse, float3 view)
{
    float3 vHalf = normalize(view + lightDirection);
    float intensity = saturate(pow(dot(normal, vHalf), specular));
    float4 colour = saturate(ldiffuse * intensity);
    return colour;
}
struct LightInfo {
    float3 direction;
    float4 colour;
    float4 ambient;
};
LightInfo calculateSunlightInfo(float4 coords) {
    const float latitude =  -(coords.z + globalPosition.y) /  (planetDiameter * 3.14159265359);
    const float longditude = (coords.x + globalPosition.x) / (planetDiameter * 3.14159265359);
    const float AxialTilt = 0.10;
    // this is a relative value rather than absolute (add 12 for abs)
    const float lengthOfDay = -12 * -sin(latitude) * cos(radians(timeOfYear));// replace -6 [-1] w/ -12*sin(latitude)  <--- to be moved to VS
    const float time = 24* (timeOfYear - int(timeOfYear) + (longditude/3.14159265));//
    const float sunAltitude = lengthOfDay - (cos(time / 3.81) / AxialTilt);
    const float skyR = saturate(-0.5 * (-(lengthOfDay - (cos( time / 3.81f) / AxialTilt)) - 1));
    const float skyB = saturate(-0.5 * (-sunAltitude - 1));
    //const float skyG = pow((pow(skyR, 8) + skyB) / 1.532f, lerp(0.5, 8, lengthOfDay * 0.5 + 0.5));

    LightInfo li;

    //calculate sun colour and position (red at night, white at day)
    //sin((time / 3.81) + 1.78) / (1.75 + sin(timeOfYear / 57.29577))
    li.direction = float3(sin(time / 3.81), -0.0833333*sunAltitude, -sin(AxialTilt * cos(timeOfYear / 57.29577) + latitude));
    //light->setDiffuseColour(lerp(0.6, 0.9, -0.5 * (cos(time / 3.81) - 1)), lerp(0.14, 0.85, -0.5 * (cos(time / 3.81) - 1)), lerp(0.01, 0.8, -0.5 * (cos(time / 3.81) - 1)), 1.0f);
    li.colour = float4(pow(max(skyR, skyB + 0.5), 0.30), pow(max(skyB, 0.5 * (skyR + skyB)), 0.30), skyB, 1.0f);
    li.colour.r = min(li.colour.g * 1.75, li.colour.r);

    li.ambient = 0.048 * li.colour + 0.03f;
    li.ambient.b = max(li.ambient.b, li.ambient.r);
    //li.colour = 0;
    return li;
}
struct triplanarUVs {
    float2 texX;
    float2 texY;
    float2 texZ;
    float3 blendWeights;
};
triplanarUVs div(triplanarUVs tpuv, float n) {       
    //tpuv.texX = tpuv.texX/n;
    tpuv.texY = tpuv.texY/n;
    //tpuv.texZ = tpuv.texZ/n;
    return tpuv;
}

float4 Tex(Texture2D t, triplanarUVs uv)// 
{
  float4 col1 = { 0,0,0,1 };
  float4 col2 = { 0,0,0,1 };
  float4 col3 = { 0,0,0,1 };

  float blend_threshold = 0.1; // from 0-1, when blending should occur. <=0 ~ always, >=1 ~ never

  if (uv.blendWeights.x > blend_threshold) col1 = t.Sample(s0, uv.texX);
  if (uv.blendWeights.y > blend_threshold) col2 = t.Sample(s0, uv.texY);
  if (uv.blendWeights.z > blend_threshold) col3 = t.Sample(s0, uv.texZ);
  float4 texColour = col1 * uv.blendWeights.x +
                     col2 * uv.blendWeights.y +
                     col3 * uv.blendWeights.z;

  return texColour;//(t.Sample(s0, uv)); //+t.Sample(s0, i.tex * 0.1)) / 3;
    
}

// sample two materials and blend them
MaterialSample heightBlend(Material m1, Material m2, float bias, triplanarUVs uv)
{// uses a constant blending factor (0.1)
    float h1 = (Tex(m1.normal_height, div(uv, m1.UVscale)).a * 0.3) + (1 - bias);
    float h2 = (Tex(m2.normal_height, div(uv, m2.UVscale)).a * 0.3) + bias;
    float blendPoint = max(h1, h2) - 0.1; // 0.1 here is the 'size' of the blend zone
    float w1 = max(h1 - blendPoint, 0);// weight
    float w2 = max(h2 - blendPoint, 0);
    MaterialSample output;
    output.albedo_specular = (
        (Tex(m1.albedo_specular, div(uv, m1.UVscale)) * w1)
        + (Tex(m2.albedo_specular, div(uv, m2.UVscale)) * w2)
        ) / (w1 + w2);
    output.normal_height = (
        (Tex(m1.normal_height, div(uv, m1.UVscale)) * w1)
        + (Tex(m2.normal_height, div(uv, m2.UVscale)) * w2)
        ) / (w1 + w2);
    return output;
}
// blend two pre-sampled materials
MaterialSample heightBlend(MaterialSample m1, MaterialSample m2, float bias, triplanarUVs uv)
{ // uv is unused
    float h1 = (m1.normal_height.a * 0.3) + (1 - bias);
    float h2 = (m2.normal_height.a * 0.3) + bias;
    float blendPoint = max(h1, h2) - 0.21;///       *
    float w1 = max(h1 - blendPoint, 0); // weight
    float w2 = max(h2 - blendPoint, 0);
    MaterialSample output;
    output.albedo_specular = ((m1.albedo_specular * w1) + (m2.albedo_specular * w2)) / (w1 + w2);
    output.normal_height = ((m1.normal_height * w1) + (m2.normal_height * w2)) / (w1 + w2);
    return output;
}
// sample a material and blend with a pre-sampled material
MaterialSample heightBlend(MaterialSample m1, Material m2, float bias, triplanarUVs uv)
{ // uv is used for m2
    float h1 = (m1.normal_height.a * 0.3) + (1 - bias);
    float h2 = (Tex(m2.normal_height, div(uv, m2.UVscale)).a * 0.3) + bias;
    float blendPoint = max(h1, h2) - 0.1;
    float w1 = max(h1 - blendPoint, 0); // weight
    float w2 = max(h2 - blendPoint, 0);
    MaterialSample output;
    output.albedo_specular = (
        (m1.albedo_specular * w1)
        + (Tex(m2.albedo_specular, div(uv, m2.UVscale)) * w2)
        ) / (w1 + w2);
    output.normal_height = (
        (m1.normal_height * w1) 
        + (Tex(m2.normal_height, div(uv, m2.UVscale)) * w2)
        ) / (w1 + w2);
    return output;
}

float3 calculateNormals(float2 tex, Texture2D heightMap)
{
    float h = 1.0 / 150.0f; // 1/size of plane/quad (arbitrary)
        //calculate the tangent & bitangent using the heightMap    
    float3 tangent;
    float3 bitangent;
    const float2 ux = float2(tex.x - h, tex.y); //neighbour to left
    const float2 vx = float2(tex.x + h, tex.y); //neighbour to right
    float xdy = 0.5 * (heightMap.Sample(s0, ux, 0).r - 0.5) - 0.5 * (heightMap.Sample(s0, vx, 0).r - 0.5); // add the change in y on x axis (dy/x) left - right
    tangent = normalize(float3(2 * h, -xdy, 0));
        // same for binormal but in z dimension
    const float2 uz = float2(tex.x, tex.y - h); //behind
    const float2 vz = float2(tex.x, tex.y + h); //in front
    float zdy = 0.5 * (heightMap.Sample(s0, uz, 0).r - 0.5) - 0.5 * (heightMap.Sample(s0, vz, 0).r - 0.5); // add the change in y on z axis (dy/z)
    bitangent = normalize(float3(0, -zdy, -2 * h));
    
    return normalize(cross(tangent, bitangent));
}


float4 main(InputType input) : SV_TARGET
{
    float dist = length(input.world_position.xz - viewpos.xz);// nice little dither fade of render distance
    const float clip_near = 18000.0;
    const float fade_dist = 2500.0;
    clip(dist > clip_near ? ((clip_near + fade_dist - dist) / fade_dist) + (((input.position.x % 2.5) - 1.25) * ((input.position.y % 2.5) - 1.25)) : 1);

    //const float PI180 = 57.29577;
    triplanarUVs uv;
    uv.texX = input.texX;
    uv.texY = input.texY;
    uv.texZ = input.texZ;
    uv.blendWeights = input.blendweights;

    const float altitude = input.world_position.y;
    //const float latitude = input.world_position.z;
    //const float longditude = input.world_position.x; // TO DO: multiply "world position" by scale       (maybe not, if u want a tiny island planet? [in this case have a separate 'planet size' var?])
    //const float slope = input.normal.y;
    //const float aspect = input.normal.z //compass direction of the slope (only north/south matters)

    MaterialSample currentMat;
    float4 textureHeight = 0;
    float4 textureColour = 0;
    float3 textureNormal = 0;
    float4 textureShine = 0;
    float4 lightColour;

    Material grass;
    Material cliff;
    Material snow;
    Material rock;
    Material water;
    Material sand;
    Material grass2;
    Material stone;
    Material savan;// dust

    //  albedo_specular                       normal_height-map   
    grass.albedo_specular = tGrass;      grass.normal_height = nGrass;      grass.UVscale = 1;
    cliff.albedo_specular = tCliff;      cliff.normal_height = nCliff;      cliff.UVscale = 5.1f;
    stone.albedo_specular = tStone;      stone.normal_height = nStone;      stone.UVscale = 2.1f;
    sand.albedo_specular = tSand;        sand.normal_height = nSand;        sand.UVscale =  0.8f;
    rock.albedo_specular = tRock;        rock.normal_height = nRock;        rock.UVscale =  1.2f;
    water.albedo_specular = tWater;      water.normal_height = nWater;      water.UVscale = 1;
    snow.albedo_specular = tSnow;        snow.normal_height = nSnow;        snow.UVscale = 1.1f;
    grass2.albedo_specular = tGrass2;    grass2.normal_height = nGrass2;    grass2.UVscale = 0.91f;
    savan.albedo_specular = tSavan;      savan.normal_height = nSavan;      savan.UVscale = 1.07f;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Sample the texture. Calculate light intensity and colour, return light*texture for final pixel colour. //
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // texture the mesh based on parameters
    if (false)//|| (input.normal.y >= 1 && altitude > -1.1 && altitude < 1.1))// water
    {
        textureColour.b = 0.85;//(input.world_position.z / 100);
        //textureColour = ((1 + tWater.Sample(s0, input.tex * 20))/3.0f) * tWater.Sample(s0, input.tex * 15000);
        //textureNormal = calculateNormals(input.tex * 15000, hWater); //nWater.Sample(s0, input.tex * 15000);
        //textureShine =  sWater.Sample(s0, input.tex * 15000);
    }
    else // not water
    {// blend the textures based on biome parameters, and using the heightmap of each texture (height blend) for more realistic texturing
        float aridness = ((input.temperature + 1.0) / 8.0) + (1.0 - input.humidity);
        // try simplifying biome texturing by ruling out pure biome areas first
        if (input.steepness <= 0.09) {
            // steep: cliff
            textureColour = Tex(cliff.albedo_specular, uv);//float4( 0.0,0.0,0.9,1.0 );//
            textureNormal = Tex(cliff.normal_height, uv);
        }
        else if (input.steepness >= 0.95 && input.snowness >= 0.9) {
            // flat + snowy: snow
            textureColour = Tex(snow.albedo_specular, uv);//float4(0.0, 0.5, 0.9, 1.0);
            textureNormal = Tex(snow.normal_height, uv);

        }
        else if (input.steepness >= 0.95 && input.beachness <= 0.1 && input.noise >= 0.99) {
            // flat + beachy, high noise: sand
            textureColour = Tex(sand.albedo_specular, uv);
            textureNormal = Tex(sand.normal_height, uv);

        }
        else if (input.steepness >= 0.95 && aridness >= 1 && input.temperature >= 1) {
            textureColour = Tex(savan.albedo_specular, uv);
            textureNormal = Tex(savan.normal_height, uv);
        }
        // else if (aridness <= 0.01 && input.noise2 >= 1) {
        //     textureColour = Tex(grass.albedo_specular, uv);
        //     textureNormal = Tex(grass.normal_height, uv);
        // }
         else {
            //  // ((rock or cliff,a,b) or (snow or ((sand or gravel,a,b) or (grass or grass2,a,b),a,b),a,b),a,b) 
            currentMat = heightBlend(heightBlend(heightBlend(heightBlend(stone, sand, input.noise, uv), heightBlend(heightBlend(grass2, grass, input.noise2, uv), heightBlend(rock, savan, 0.86 * (input.temperature + 1), uv), aridness, uv), input.beachness, uv), snow, input.snowness, uv), cliff, 1 - input.steepness, uv);
            //currentMat = heightBlend(savan,grass2 , input.temperature / 22.4, uv);

            textureColour = currentMat.albedo_specular; //float4(snowness, snowness, snowness, 1.0); //
            textureNormal = currentMat.normal_height;
            //textureShine = currentMat.specular;
            //
        }
  
//       MACRO TEXTURE VARIATION - inspired by unreal engine 4
        ////textureColour.xyz = (textureColour.xyz * lerp(float3(0.52, 0.51, 0.50), float3(1.0, 1.0, 1.0), coefficient));

    }
    //  LIGHTING / SUNLIGHT
    LightInfo sunlight = calculateSunlightInfo(input.world_position);

    lightColour = calculateLighting(-sunlight.direction, input.normal + textureNormal.xyz, sunlight.colour);//calculateLighting(-lightDirection, input.normal + textureNormal, diffuseColour); 

    ///-- added biome colour effects --///
    float variety = (tWater.Sample(s0, uv.texY * 0.002).g + 0.5) * (tWater.Sample(s0, uv.texY * 0.05).r + 0.5) * (tWater.Sample(s0, uv.texY * 0.2).b + 0.5); 
        
    
     //add wetness effects at beach
    if (true && altitude <= 2.2)
    {
        float depth = abs(altitude - 2.2);
        textureColour.rgb = pow(textureColour.rgb, min(1 + depth * 0.30, 3.0));//  wetness    
        //textureColour.rgb /= min(1 + depth, 3.0); //darkness. at altitudes > beachline we / by 1 [no change]
        textureColour.a *= min(1 + depth, 2.3);

        depth = pow(min(altitude/scale, 0),3.0);// seawater colouration
        lightColour.r /= 1 - depth * 0.15;//, 4.0;//min();
        lightColour.g /= 1 - depth * 0.005;//, 2.35;//min();
        lightColour.b /= 1 - depth * 0.0002;//, 1.2;//min();
    }
    else if (true || textureColour.g > (textureColour.r + textureColour.b) / 1.35) // if pixel is green
    {// grass dryness
        float greenness = (1.5 * textureColour.g - length(textureColour.rb)) / textureColour.g;//2* textureColour.g - ((textureColour.r + textureColour.b) / 2.35);
        float4 newcol = textureColour;
        newcol.r /= min(0.6 + input.humidity,1.2);
        newcol.g /= max(0.95 + 0.5 + (input.temperature / 64), 0.5);
        newcol.b /= 2 * max(0.95 + 0.5 + (input.temperature / 64), 0.5);
        newcol.rgb *= lerp(float3(0.7, 0.66, 0.6), float3(1.0, 1.0, 1.0), variety);

        textureColour = lerp(textureColour, saturate(newcol), pow(greenness, 2.0)); // //float4(0.7,0.1,0.8,1.0), float4(0.2, 0.8, 0.1, 1.0)
        //textureColour.rgb = greenness;
        //if (greenness > 1.0) textureColour.rg = greenness - 1;
        //if (greenness < 0.0) textureColour.rb = greenness + 1;
    //*/
    }
    if (true && /*textureColour.r > textureColour.b - 0.1 && textureColour.b > textureColour.g - 0.1 && textureColour.g > textureColour.r - 0.1 &&*/ input.snowness < 0.45) // if pixel is grey
    {// rock sandiness and soil moisture
        float rockness = ((max(textureColour.r, max(textureColour.g, textureColour.b))) - (min(textureColour.r, min(textureColour.g, textureColour.b)))) / (1 - abs(2 * length(textureColour.rgb) - 1));
        float3 new_mineral_col = textureColour.rgb;
        //float redness;
        //float darkness;
        variety *= 0.3;
        new_mineral_col.b /= max(variety+ 0.3 + input.humidity * (1 - input.snowness) * 2, 1.0);
        new_mineral_col.g /= max(variety+ 0.2 + input.humidity * (1 - input.snowness) * 1, 1.0);
        textureColour.rgb /= min(max(2.46 + input.humidity * 3.5, 1.0),1.75);
        textureColour.rgb = lerp(textureColour.rgb, new_mineral_col, pow(saturate(1-rockness),2.f));
    }
    ///     Topography shader
    if (false) {
        textureColour.g = pow((0.0008*input.world_position.y+1)*0.5,3);
        textureColour.b = -pow(0.09 * input.world_position.y,3);
        textureColour.r = 1 - pow(0.0008 * input.world_position.y,2) * tan(input.world_position.y);
        if (input.world_position.y >= -1 && input.world_position.y <= 1)
            textureColour.rgb = 1;
    }

            ///     Temperature map shader
    //textureColour.r = input.temperature / 36.0;
    //textureColour.b = -(input.temperature / 36.0);
    //textureColour.g = pow(1 - abs(input.temperature / 36.0),0.5);
    //if (input.world_position.y <= 0)
    //    textureColour.rgb = 0.5;

            ///     Normalmap shader
    //textureColour.rgb = input.normal;

        ///     Humidity Map shader
    if (false) {
        textureColour.r = 2* input.humidity -0.5 ;
        textureColour.b = -2 * input.humidity + 0.5;
        textureColour.g = pow(1 - abs(input.humidity / 2.0),1);
        if (input.world_position.y <= 0)
            textureColour.rgb = 0.5;
    }


    float3 view = normalize(viewpos - input.world_position.xyz);
    
    //textureColour.r = input.humidity; //input.wind.x;//temperature / 36.6; //
    //textureColour.g = cos(input.temperature / 15); //0.0; //snowness; //wind.x;//
    //textureColour.b = input.temperature / -37.0; //wind.y;//
    float4 pixelColour = float4( textureColour.xyz * (lightColour.xyz + sunlight.ambient.xyz) + (sunlight.colour.xyz * textureColour.a * calculateSpecular(-sunlight.direction, normalize(input.normal + textureNormal.xyz), sunlight.colour, view)), 1.0);//+ textureNormal.xyz
    pixelColour.a = 1.0;
    //if (int((input.position.x + (timeOfYear * 20000 * input.windScreenDir))  ) % 67 == 0 && int(input.position.y - (timeOfYear * 50000))% 67 == 0)
    //    pixelColour.xy = 0.0; // snow
    
   //Height Fog
   //    pixelColour = lerp(pixelColour, float4(0.435, 0.671, 0.6931, 1), saturate(log(length(viewpos.xz - input.world_position.xz) / (input.world_position.y))/10));

    if ((input.world_position.z + globalPosition.y) / (planetDiameter * 3.14159265359) > 1.09 || (input.world_position.z + globalPosition.y) / (planetDiameter * 3.14159265359) < -1.09)
        pixelColour.g = 0.0;//   arctic circle
    if ((input.world_position.z + globalPosition.y) / (planetDiameter * 3.14159265359) > 1.6 || (input.world_position.z + globalPosition.y) / (planetDiameter * 3.14159265359) < -1.6)
        pixelColour.rg = 0.0;

        ///     Dithering Shader
    //float thres = 0.5 * (1 + bfm(input.position.xxyy, 3));
    //if (pixelColour.r > thres) pixelColour.r = 1.0;
    //else pixelColour.r = 0.0;
    //if (pixelColour.g > thres) pixelColour.g = 1.0;
    //else pixelColour.g = 0.0;
    //if (pixelColour.b > thres) pixelColour.b = 1.0;
    //else pixelColour.b = 0.0;
    /// monochrome:
    //if (length(pixelColour.rgb) > thres) pixelColour.rgb = 1.0;
    //else pixelColour.rgb = 0.0;

    if (timeOfYear == 0.0) {// humidity view
        HSLcolour hum;
        hum.h = input.humidity;
        hum.s = 1.00;
        hum.l = input.humidity / 1.0;
        pixelColour = float4(hum.toRGB(), 1.0);
    }

    //pixelColour = bfm(input.world_position + bfm(input.world_position + bfm(input.world_position, 14), 14), 14);
   // pixelColour.xyz = input.blendweights;//if (input.blendweights.x == 0) { pixelColour.r = 1; }//
    //return sunlight.colour;
    return pixelColour;
}



///////////////////////--------------------------//////////////////////////
///                          perlin noise!
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
    float seed = 74981.87126386f;//21732.37f;
    //float random = (seed / 100) * sin(x * (seed / 11.0923) + y * (seed / 2.62349) + seed / 32.12379) * cos(x * (seed / 8.812934) * y + y * y * seed + (seed / 23.792173));
    float random = seed * 9.01230387 + sin(x + y * seed) * cos(x * y + seed) * seed + x + x / y + seed * y - sin(seed - y);
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

float perlin(float4 world_position)
{
    float x = abs(world_position.x);
    float y = abs(world_position.z);
    
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
float bfm(float4 coords, int octs)
{
    float val = 0;
    for (int o = 0; o < octs; o++)
    {
        val += perlin(coords * pow(2, o)) / pow(2, o);
    }
    return val;
}
///#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#///