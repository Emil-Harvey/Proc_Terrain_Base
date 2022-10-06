// Tessellation domain shader
// After tessellation the domain shader processes the all the vertices
// once per vertex

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};
cbuffer DataBuffer : register(b1)
{
    float2 chunkPosOffset;//
    float scale;
    float time;
    int tessellationFactor;
    float3 manipulationDetails;
    float2 globalPosition;/// 
    float planetDiameter;
    float padding_;
};
cbuffer CameraBuffer : register(b2)
{
    float3 viewpos;
    float padding;
};  // used to calculate normals

Texture2D HEIGHT : register(t0);// heightmap
SamplerState s0 : register(s0);

struct ConstantOutputType
{
    float edges[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
};

struct InputType
{
    float4 position : POSITION;

    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;

    //float amplitude : PSIZE0;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float4 world_position : POSITION;
    noperspective float2 texX : TEXCOORD0;
    noperspective float2 texY : TEXCOORD1;
    noperspective float2 texZ : TEXCOORD2;
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
float bfm(float2 pos, int octaves);
float perlin(float2 world_position, float speed = 0);
float terragen(float2 coords, int octs);

float invsmoothstep(float y)
{// fast inverse
    float yn = 2.0 * y - 1.0;
    float t = 0.45 * yn;
    t -= (t * (4.0 * t * t - 3.0) + yn) / (12.0 * t * t - 3.0);
    return t + 0.5;
}
float get_terrain_height(float2 input, float octaves) { // old noise height function
    float ret;
    float continental_noise = 1 * ( bfm((input + 800) / (4900.7 * scale), 4)+(0.2*perlin(input/(500 * scale))) );
    if (continental_noise > -0.25) {// save effort on ocean  ... maybe just make 'octaves' lower?
        ret = 100 * scale * terragen((input + 800) / (100.0 * scale), octaves / 2);
        ret += abs(100 * scale * bfm((input + 800) / (300.0 * scale), octaves / 2));// abs makes V valleys, nice, but not really any plains... erosion would solve that
    }
    ret += 10 * scale;//   prevents most valleys reaching 0, which wid screw up the next step


    ret *= continental_noise;// creates oceans

    if (ret > -5) {
        float Mountain_ness = 3.55 + 3.25 * (perlin(input / (510.7 * scale)) + perlin(input / (181.7 * scale)));
        Mountain_ness = invsmoothstep(Mountain_ness);
        //ret += bfm(input, 2);
        ret *= min(max(ret, 1), Mountain_ness);
    }

    return ret;
}

float3 calculateNormal(float2 pos, float h = 5.0 / 5.0f)
{
   
    ///   int octaves = int(lerp(10, 2, saturate(length(viewpos.xz - pos) / 595.0)));

    // float h = 1.0 / 75.0f; // 1/size of plane/quad (arbitrary)

     //calculate the tangent & bitangent using the heightMap    
    float3 tangent;
    float3 bitangent;
    const float2 ux = float2(pos.x - h, pos.y); //neighbour to left
    const float2 vx = float2(pos.x + h, pos.y); //neighbour to right
    float xdy = 1 * HEIGHT.SampleLevel(s0, (ux / 11520.0) + 0.5, 1).a - 1 * HEIGHT.SampleLevel(s0, (vx / 11520.0) + 0.5, 1).a;
    tangent = normalize(float3(2 * h, xdy, 0));
    // same for bitangent but in z dimension
    const float2 uz = float2(pos.x, pos.y - h); //behind
    const float2 vz = float2(pos.x, pos.y + h); //in front
    float zdy = 1 * HEIGHT.SampleLevel(s0, (uz / 11520.0) + 0.5, 1).a - 1 * HEIGHT.SampleLevel(s0, (vz / 11520.0) + 0.5, 1).a;
    bitangent = normalize(float3(0, zdy, -2 * h));

    return normalize(cross(tangent, bitangent));
}

[domain("quad")]
OutputType main(ConstantOutputType input, float2 uvwCoord : SV_DomainLocation, const OutputPatch<InputType, 4> patch)// patch should be 3 for tri, 4 for quad etc
{
    float3 vertexPosition;
    OutputType output;

    // Determine the position of the new vertex.
    // bilinear interpolation: 
    vertexPosition = lerp( // lerp the "horizontal" positions by the "vertical" one
        lerp(patch[0].position, patch[1].position, uvwCoord.y),
        lerp(patch[3].position, patch[2].position, uvwCoord.y),
        uvwCoord.x);
    output.position = float4(vertexPosition, 1.0f);

   //going to try make 3D tex coordinates/ output.tex = 64 *uvwCoord / scale;//vertexPosition.xz / 30; //

   //// calculate un-displaced normals
   //output.normal = lerp( // bilinear interpolation
   //    lerp(patch[0].normal, patch[1].normal, uvwCoord.y),
   //    lerp(patch[3].normal, patch[2].normal, uvwCoord.y),
   //    uvwCoord.x);

    //output.amplitude = patch[0].amplitude;
    const half sealevel = 0.02f;
    ///*
    // Calculate the position of the new vertex against the world, view, and projection matrices. or do this in geo shader
    output.world_position = mul(float4(vertexPosition, 1.0f), worldMatrix); //float4(vertexPosition, 1.0f);  

    

    // apply seed offset and/or position offset - to be used for noise. 
    /*const*/ float4 coords = output.world_position + float4(chunkPosOffset.x + globalPosition.x, 0, chunkPosOffset.y + globalPosition.y, 0);//
    while (coords.x - chunkPosOffset.x > planetDiameter)// make planet loop cylindrically
        coords.x += -planetDiameter - planetDiameter;
    while (coords.x - chunkPosOffset.x < -planetDiameter)
        coords.x += planetDiameter + planetDiameter;
    /// will probably need to add distinct 'seed' offset so that seed doesn't interfere with Eqtr position etc. // what? // i think this has been done

    ///     DISPLACE VERTICES
    output.world_position.y = 1;

    int MaxOctaves = 12;// * sqrt(scale);//      scale the Detail & therefore efficiency of noise, over distance
    int MinOctaves = 1;
    int octaves = int(lerp(MaxOctaves, MinOctaves, saturate(log10(length(viewpos - output.world_position) / (2000.0 * scale))+1)));

    //***/output.world_position.y = get_terrain_height(coords.xz, octaves);//  
    float4 heightmapSampled = HEIGHT.SampleLevel(s0, (output.world_position.xz / 11520.0) + 0.5, 0);// vertexPosition.xz//
    output.world_position.y = heightmapSampled.a;



    //       CALCULATE NORMALS              Send the normal, light into the ps
    if (octaves > 6)//
    {
        ////output.normal.xyz = heightmapSampled.rgb;
        output.normal = calculateNormal(output.world_position.xz);
  //        smooth normal by averaging neighbours

       // output.normal.r = -output.normal.r;//*/
    }
    else // this mesh is not being manipulated, so keep the normals as they were
    {
        // calculate un-displaced normals
        output.normal = lerp( // bilinear interpolation
            lerp(patch[0].normal, patch[1].normal, uvwCoord.y),
            lerp(patch[3].normal, patch[2].normal, uvwCoord.y),
            uvwCoord.x);
    }
    //*
    ///     CALCULATE 'GEOGRAPHIC VARIABLES'
    const float4 seedc = coords;
    coords = output.world_position + float4(globalPosition.x, 0, globalPosition.y, 0);// ???
    const half PI180 = 57.29577;
    const half PI = 3.14159265359;
// not needed    const float2 uv = output.tex;
    const half altitude = coords.y;
    const half latitude = 2 * (coords.z / (planetDiameter * PI));// i think this is a val btwn 0 & 2pi [may be -pi & pi or something]
    const half longditude = coords.x/ (planetDiameter * PI);
    
    const half slope = output.normal.y;
    const float2 aspect = output.normal.xz;// compass direction of slope

        // flat to cliff attenuation
    output.steepness = saturate(pow(saturate(slope) + 0.18 + (0.12 * bfm(scale * seedc.xz / (scale * 12), 4)), 10));

    if (output.steepness < 0.016 && octaves > 6){// add real roughness to steep slopes
        output.world_position.xyz += (1- output.steepness) * 4.8 * log(scale-1.0) * output.normal * perlin(seedc.xz / (scale * 800.1171));
        if (octaves > 9) {
            output.world_position.xyz += 0.592 * scale * output.normal * perlin(seedc.xz / (scale * 1.1171));
        }
    }

    // beachline
    const half beachline = 0.83; //altitude at which beaches meet soil
    output.beachness = saturate(slope) * saturate(2 * altitude - (4*bfm(seedc.xz / (scale * 12), 4)) - beachline);

    if (output.world_position.y > -5) {

    // PREVAILING WINDS VECTOR
    const half prevailingWindY = tan(2 * latitude) / (1.74 / abs(latitude * 0.3183)); // or something
    const half prevailingWindX = -tan(abs(1 * latitude) + (3.14159 * 0.5)) / 5.0; //
    ///*const float2 prevailingWind///
    output.wind = normalize(float2(prevailingWindX, prevailingWindY)); //
    //output.world_position.y = prevailingWindX * 30; debug
    

    const half minGlobalTemp = -37.0;//`C
    const half maxGlobalTemp = 36.6;//`C
    //  climactic/static annual average temperature. seasonal and daily weather will alter this
    output.temperature = lerp(minGlobalTemp, maxGlobalTemp, 
        (0.5 + 0.5 * cos(latitude)) // cold @ poles hot @ equator
        - (altitude / (300 * scale))//   cold @ high altitude
        + dot(aspect.y, sin(latitude * 0.5)) * 0.8);// cold on polar aspect slopes

// snowline //height value - high near the middle (equator)     ::{ 70'- 550m; 45'- 3300m; Eqtr- 6000m }::
    const half snowline = (13.0 + 13.3 * ( cos(latitude)
        + 3.25*(cos(time / PI180) * -sin(latitude * 0.5))) )//      <----- needs work
        * scale;
    // this represents temperature/climate, specifically how cold
    output.snowness = pow(saturate(0.5 / scale * (0.2 * altitude - 7.1181*bfm(seedc.xz / (scale * 20), 3) - snowline)), 3.0 );
    //  apply deep snow effect on flat ground (raise vertices)
    output.world_position.y += max(output.snowness * output.steepness - 0.5, 0) * scale;
    /*DEBUG: */output.snowness = HEIGHT.SampleLevel(s0, (output.world_position.zx / 11520.0) + 0.5, 0).b;//heightmapSampled.b;
    // absolute humidity - relative humidity is less meaningful///          <-- fix this algorithm?
    /*output.humidity =
        ((output.temperature - minGlobalTemp) * 0.02// rep 1* w abs
            + (dot(output.wind, -aspect) *0.0)  ); // rainshadow effect - it is more humid on slopes that face the wind
        //+ perlin(seedc.xz / (scale * 99)) +1    
        //)/ altitude ; //
output.humidity = pow((output.humidity - 0.5) * 1.75, 5) + 0.5;// shift most values closer to 0.5 */

output.humidity = heightmapSampled.r;
    }
    else {
        output.snowness = 0.0; // avoid snow underwater lol
    }
//      noise to determine between plains and woodland, slightly higher chance of the former
    output.noise2 =  saturate(output.humidity * pow(1 - bfm(seedc.xz / (scale * 29), 4), 3));

    // similar noise for beach pebbles, but offset by 99 so the noise does not line up
    output.noise = saturate(pow(1 - bfm(seedc.xz / (scale * 60), 3), 3)+perlin(seedc.xz / (scale * 211.1171))+ 0.3);//pow(saturate(1 - bfm(99 + coords / (scale * 12), 4)), 3);
    //output.var = cos(output.world_position.x);



    ///
    /*- triplanar mapping -*/
    output.blendweights = abs(output.normal);
    const float plateauSize = 0.2;
    const float transitionSpeed = 51.0;
    const float texScale = 0.0625;
    const float nLength = 0.5;//?
    output.blendweights = output.blendweights - plateauSize;
    output.blendweights = pow(saturate(output.blendweights), transitionSpeed);
    const float divisor = output.blendweights.x + output.blendweights.y + output.blendweights.z;
    output.blendweights /= divisor;
    output.texX = (output.world_position.yz + nLength) * texScale;
    output.texY = (output.world_position.zx + nLength) * texScale;
    output.texZ = (output.world_position.xy + nLength) * texScale;
   // output.tex = output.world_position.xz/16.0;
    
    ////fish-eye effect?
    //output.world_position.y -= length(viewpos.xz - output.world_position.xz);

    // convert to screen coordinates, after snow height etc is applied
    output.position = mul(output.world_position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    //*/
    return output;
}


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
    float random = seed * 9.01230387 + sin(x + y * seed) * cos(x * y + seed) * seed + x + x / y + seed * y - sin(seed - y);// ^ was producing artefacts
    float2 ret;
    ret.x = (float)cos(random);
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
    float x = abs(world_position.x) + time * speed;
    float y = abs(world_position.y) + 0.3291f * time * speed;

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
float ridges(float2 coords, float s)
{
    return (1 - (4 * abs(perlin(coords, s)))) * perlin(coords.yx, s);
}

float terragen(float2 coords, int octs)
{
    float val = 0;
    float weight = 1;
    for (int o = 0; o < octs; o++)
    {
        int freqMultiplier = 4;//1.125;// default = 2
        float h = ridges(coords * pow(freqMultiplier, o), 0) / pow(2, o);//
        h *= h;
        h *= weight;
        weight = h;
        val += h;

    }
    return val;
}
float bfm(float2 coords, int octs)
{
    float val = 0;
    float weight = 1;
    for (int o = 0; o < octs; o++)
    {
        val +=  perlin(coords * pow(2, o), 0) / pow(2, o);
    }
    return val;
}
float aquagen(float2 coords, int octs, float s)
{
    float val = 0;
    for (int o = 0; o < octs; o++)
    {
        float h = ridges(coords * pow(2, o), s) / pow(2, o);
        h *= h;
        val += h;

    }
    return val;
}
///#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#///