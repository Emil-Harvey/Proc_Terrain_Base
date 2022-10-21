//Landscape heightmap generation.

// 
cbuffer DataBuffer : register(b0)
{
    float2 seed;//
    float scale;
    float time;
    int tessellationFactor;
    float3 manipulationDetails;
    float2 globalPosition;/// 
    float planetDiameter;
    float padding_;
};

Texture2D gInput : register(t0);
RWTexture2D<float4> gOutput : register(u0);

#define N 8 //256
//#define CacheSize (N + 2*gBlurRadius)
//groupshared float4 gCache[CacheSize];

float2 gradient(int x, int y);
float bfm(float2 pos, int octaves);
float perlin(float2 world_position, float speed = 0);
float terragen(float2 coords, int octs);
float flow(float2 i, int octaves) {
    if (octaves <= 0) return bfm(i, 2);
    else
        return bfm(i + bfm(i+bfm(i,octaves/3.0),octaves/1.5),octaves);
}

float hash12(float2 p)// 2 inputs, 1 output
{
    float3 p3 = frac(float3(p.xyx) * .1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return frac((p3.x + p3.y) * p3.z);
}

float2 random_offset(float seed) {// random float2 between 100 & 200
    return float2(100.0 + hash12(float2(seed, 0.0)) * 100.0,
                  100.0 + hash12(float2(seed, 1.0)) * 100.0);
}

float interpolate(float a0, float a1, float w)// cubic interpolation
{
    return pow(w, 2.0) * (3.0 - 2.0 * w) * (a1 - a0) + a0;
} //*/
float avg(float a, float b) { return (a + b) / 2.0; }

float NoiseTexture(float2 coords, float scale,int octs, float roughness,float distortion)
{
    float val = 0;
    //float weight = 1;

    //float xz = coords.x + (0.10);   // domain rotation
    //float s2 = xz * -0.211324865405187;
    //float yy = coords.y * 0.577350269189626;
    //float xr = coords.x + (s2 + yy);
    //float zr = (0.10) + (s2 + yy);
    //float yr = xz * -0.577350269189626 + yy;


    float2 d_coords = coords * scale *scale;//float2(yr,xr)

    if (distortion >= 0.0) {
        d_coords += float2(perlin(d_coords + random_offset(0.0)) * distortion,  //
                           perlin(d_coords + random_offset(1.0)) * distortion); //
    }
    d_coords /= scale;
    
    for (int o = 0; o < octs; o++)
    {
        int freqMultiplier = 4;//1.125;// default = 2
        float h = perlin( scale * d_coords * pow(2, float(o)) ) * pow( saturate(roughness), float(o) );//
        //h *= h;
        //h *= weight;
        //weight = h;
        val += h;
    }
    return val; // ensure output is between 0 & 1.0
}

float invsmoothstep(float y)// broken
{// fast inverse
    float yn = 2.0 * y - 1.0;
    float t = 0.45 * yn;
    t -= (t * (4.0 * t * t - 3.0) + yn) / (12.0 * t * t - 3.0);
    return y;// saturate(t + 0.5);
}



float get_terrain_height(float2 input, float octaves) {
    float ret;
    const float continental_noise = lerp(
                        flow(input / (4900.7 * scale), 5)+(0.2 * bfm(input / (500 * scale), octaves)),
        1 * (bfm(input / (4900.7 * scale), 6) + (0.2 * bfm(input / (500 * scale),octaves))), perlin(input / (919.7 * scale)));
    
    if (true || continental_noise > -0.25) {// save effort on ocean  ... maybe just make 'octaves' lower?
        ret = 100 * scale * terragen((input + bfm(input / (4900.7 * scale), 5)) / (100.0 * scale), octaves / 2);
        ret += abs(100 * scale * bfm((input + 800) / (300.0 * scale), octaves / 2));// abs makes V valleys, nice, but not really any plains... erosion would solve that
    }
    ret += 10 * scale;//   prevents most valleys reaching 0, which wid screw up the next step


    ret = ret *continental_noise;// creates oceans

    if (ret > -5) {
        float Mountain_ness = 3.55 + 3.25 * (
            flow( input / (510.7 * scale),3) + perlin(input / (181.7 * scale)));
        //Mountain_ness = invsmoothstep(Mountain_ness);
        //ret += bfm(input, 2);
        ret *= min(max(ret, 1), Mountain_ness);

        //ret = 10 * pow(bfm(input / scale, octaves / 2), 8) * pow(bfm(input / (6 * scale), octaves / 2), 0.5);
    }

    return ret;
}
float get_alt_terrain_height(float2 input, float octaves) // a revised terrain algorithm after experimentation in blender
{
    float height = 0;
    const float2 alt_input = input + float2(55.4, 35.1);
    const float scale_coeff = scale * 30.0;

    // to start, get some noise that will be used to vary how rough the noises are and much they are distorted.
    const float roughness_noise = saturate(0.4 +0.4 * NoiseTexture(input, 0.16/ scale_coeff, 5, 0.5, 1.5));//bfm(input / scale_coeff,15 );//
    const float distortion_noise = saturate(0.5 + 0.6 * NoiseTexture(alt_input, 0.16 / scale_coeff, 5, 0.5, 0.5));

    const float mountain_noise = 1.0-abs(2.0 * NoiseTexture(input, 3.6 / scale_coeff, 8, roughness_noise, distortion_noise));// 
    const float valley_noise = abs(2.0 * NoiseTexture(alt_input, 3.6 / scale_coeff, 8, roughness_noise, distortion_noise));//0.5+0.5*
    height = mountain_noise * valley_noise;//
    
    const float continental_noise = lerp(
        flow(input / (4900.7 * scale), 5) + (0.2 * bfm(input / (500 * scale), octaves)),
        1 * (bfm(input / (4900.7 * scale), 6) + (0.2 * bfm(input / (500 * scale), octaves))), perlin(input / (919.7 * scale)));

    // the lower this value (0.0), the more 'marbled' the terrain, creating swirly rivers and archipelagos;
    // the higher the value (1.0), the more regular the terrain, far fewer islands or river deltas.
    // low values -> western scotland; high values -> east coast scotland. 
    const float macro_vary_noise = saturate(1.f+NoiseTexture(alt_input, 0.66 / scale_coeff, 7, roughness_noise, distortion_noise)- (1.0*continental_noise));

    height = /*smooth*/max(macro_vary_noise + height, macro_vary_noise * height);

    // APPLY SCALING/SMOOTHING (inv sm. step)...
   
    height = invsmoothstep(-0.25+(1.0* height))-0.10;
    
    float subcontinent_noise = -0.15+NoiseTexture(input, 0.6 / scale_coeff, 8, 0.59589, min(distortion_noise,0.7));

    height *= 59.8 ;
    subcontinent_noise *= 53.4 ;

    height += subcontinent_noise;
    //...
    
    height = ((continental_noise * height) - (3.8f * continental_noise * continental_noise * continental_noise)) * scale * 3.0;
    return max(height,pow(height,3.0)/(10000* scale *scale));//*(clamp(subcontinent_noise / (scale * max(scale,1.0)), 1.0, 2.0)-0.0 );//
}


float3 calculateNormal(float2 pos, int octaves, float h = 1.0 / 50.0f) // inefficient- uses get_terrain_height four times
{

    ///   int octaves = int(lerp(10, 2, saturate(length(viewpos.xz - pos) / 595.0)));

    // float h = 1.0 / 75.0f; // 1/size of plane/quad (arbitrary)

     //calculate the tangent & bitangent using the heightMap    
    float3 tangent;
    float3 bitangent;
    const float2 ux = float2(pos.x - h, pos.y); //neighbour to left
    const float2 vx = float2(pos.x + h, pos.y); //neighbour to right
    float xdy = 1 * get_alt_terrain_height(ux, octaves) - 1 * get_alt_terrain_height(vx, octaves);
    tangent = normalize(float3(2 * h, xdy, 0));
    // same for bitangent but in z dimension
    const float2 uz = float2(pos.x, pos.y - h); //behind
    const float2 vz = float2(pos.x, pos.y + h); //in front
    float zdy = 0.75 * get_alt_terrain_height(uz, octaves) - 0.75 * get_alt_terrain_height(vz, octaves);
    bitangent = normalize(float3(0, zdy, -2 * h));

    return normalize(cross(tangent, bitangent));
}

[numthreads(N, N, 1)]
void main(int3 groupThreadID : SV_GroupThreadID,
	int3 dispatchThreadID : SV_DispatchThreadID)
{
    const double c = 10.0;// scale factor ->: 1500 * 7.68 = 11520 { the world_position/size of the 3x3 terrain meshes }
    const float2 coords = { (dispatchThreadID.x * c) + seed.x + globalPosition.x , (dispatchThreadID.y * c) + seed.y + globalPosition.y };
    const float height = get_alt_terrain_height(coords, 20.0);

    const float macroScale = 3.0;
    const float2 macroCoords = { (((dispatchThreadID.x * c) - 5760) * macroScale) + 5760 + seed.x + globalPosition.x , (((dispatchThreadID.y * c) - 5760) * macroScale) + 5760 + seed.y + globalPosition.y };
    const float macro_height = get_alt_terrain_height(macroCoords, 8.0);

    // CALCULATE NORMAL
    const float3 normal = calculateNormal(coords, 5.0); const float slope = normal.y; const float2 aspect = normal.xz;
    
    // PREVAILING WINDS VECTOR
    float latitude = coords.y - seed.y; 
    latitude = 2 * (latitude / (planetDiameter * 3.14159)); // add 'sphericality'
    const half prevailingWindY = tan(2 * latitude) / (1.74 / abs(latitude * 0.3183)); // or something
    const half prevailingWindX = -tan(abs(1 * latitude) + (3.14159 * 0.5)) / 5.0; //
    float2 offset = gradient(coords.x, coords.y); // sampling jitter
    float2 wind = normalize(float2(prevailingWindX + offset.x, prevailingWindY + offset.y));//
    
    float humidity = 0;// calculate humidity 
    //*
    if (true || height <= 0)
        humidity = 0.5;// underwater, duh
    /*else {
        float h_attenuation = 100.0 / 1.0; // how far humidity is carried from 'water sources' 
        float positionOffset = 50;
        while (h_attenuation > 1) {//       this causes stress on the GPU? <- debug settings were enabled in properties
            
            float a = get_terrain_height(coords + (-wind * positionOffset), 1.0);
            a *= a*a;// cube it
            humidity -= h_attenuation * ((a>0)? (0.2/scale)*a:(15/scale)*a);
            h_attenuation = h_attenuation - 0.5;//pow(h_attenuation, 0.98) // 
            positionOffset = 1.05 * positionOffset + 350;
        }
        humidity = max(min(humidity, 2.0),0.0);

        //          from terrain-ds:
        const half minGlobalTemp = -37.0;//`C
        const half maxGlobalTemp = 36.6;//`C
        //  climactic/static annual average temperature. seasonal and daily weather will alter this
        float temperature = lerp(minGlobalTemp, maxGlobalTemp,
                                    (0.5 + 0.5 * cos(latitude)) // cold @ poles hot @ equator
                                    - (height / (300 * scale))//   cold @ high altitude
                                    + dot(aspect.y, sin(latitude * 0.5)) * 0.8);// cold on polar aspect slopes // 
        // absolute humidity - relative humidity is less meaningful///          <-- fix this algorithm?
        //*
        humidity +=
            ((temperature - minGlobalTemp) * 0.02 // ~
            + (dot(wind, -aspect) *0.0)  ); // rainshadow effect - it is more humid on slopes that face the wind
        //+ perlin(seedc.xz / (scale * 99)) +1
        //)/ altitude ; //
        
        humidity = saturate(sqrt(humidity / 1.0) + min(macro_height, 0) / 1.60);
        
        humidity = pow((humidity - 0.5) * 1.75, 5) + 0.50;// shift most values closer to 0.5 *
        
        

    }
    //*/
    //float3 normal = calculateNormal(coords.xz, 10.0); unnecessary(?) just sample heightmap per vertex instead

	float4 output = float4(0, 0, 1, 1);

	

    output.a = height;//40+sin(degrees(coords.y)) * 40.f;//
    output.r = height/(100.f*scale);//humidity

    output.g = slope; // / manipulationDetails.z;
    output.b = macro_height;//0.01 * height / (1+pow(manipulationDetails.z,3));
    //if (height < 0) { output.b = 1+height; }
    //output.rgb = normal;//?
    /// output.rgb -> humidity & wind?

	gOutput[dispatchThreadID.xy] = output;

    //output = gOutput[(int2)(((dispatchThreadID.xy - 750) * macroScale) + 750)];

    if (false && manipulationDetails.z > 1.0) {
        output.r = gInput[dispatchThreadID.xy + wind.xy].r * 0.95
            + (gInput[dispatchThreadID.xy + (wind.xy * 2)].r * 0.8)
            + (gInput[dispatchThreadID.xy + (wind.xy * 4)].r * 0.6)
            + (gInput[dispatchThreadID.xy + (wind.xy * 7)].r * 0.4)
            + (gInput[dispatchThreadID.xy + (wind.xy * 11)].r * 0.25)
            + (gInput[dispatchThreadID.xy + (wind.xy * 16)].r * 0.1);
        output.r /= 400;
        gOutput[dispatchThreadID.xy] = output;
    }
}

//////////////////////--------------------------//////////////////////////



/* Create random direction vector
 */
float2 gradient(int x, int y)
{
    // makes the random gradients for the given grid coordinate
    float seed = 21732.37f;
    //float random = (seed / 100) * sin(x * (seed / 11.0923) + y * (seed / 2.62349) + seed / 32.12379) * cos(x * (seed / 8.812934) * y + y * y * seed + (seed / 23.792173));
    float random = seed * 9.01230387 + sin(x + y * seed) * cos(x * y + seed) * seed + x + x / (y + seed) * y - sin(seed - y);// ^ was producing artefacts
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
    float x = abs(world_position.x); //+ time * speed;
    float y = abs(world_position.y); //+ 0.3291f * time * speed;

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
        val += perlin(coords * pow(2, o), 0) / pow(2, o);
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