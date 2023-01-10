//Landscape heightmap generation.

// 
cbuffer DataBuffer : register(b0)
{
    float2 seed;//
    float scale;
    float time;
    int tessellationFactor;
    float2 manipulationDetails;
    float amplitude;
    float2 globalPosition;/// 
    float planetDiameter;
    float pixel_vertex_scale; //padding_;
};

Texture2D gInput : register(t0);
RWTexture2D<float4> gOutput : register(u0);

#define N 8 //256
//#define CacheSize (N + 2*gBlurRadius)
//groupshared float4 gCache[CacheSize];

float2 gradient(int x, int y);
float bfm(float2 pos, int octaves);
float bfm(float3 pos, int octaves);
float perlin(float2 world_position);
float openSimplex2_Conventional(float3 pos);
//#define simplex(pos) openSimplex2_Conventional(pos)
float ridges(float2 coords);
float terragen(float2 coords, int octs);
float aquagen(float2 coords, int octs);
float NoiseTexture(float3 coords, float scale, int octs, float roughness, float distortion);
float flow(float2 i, int octaves);
float flow(float3 i, int octaves);
float4 openSimplex2Base(float3 x);
float simplex(float3 pos){ return openSimplex2_Conventional(pos) ; }// /5.0
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
{    return pow(w, 2.0) * (3.0 - 2.0 * w) * (a1 - a0) + a0; } //*/
float avg(float a, float b) { return (a + b) / 2.0; }

float NoiseTexture(float2 coords, float scale, int octs, float roughness, float distortion);

float invsmoothstep(float y)// broken
{// fast inverse
    float yn = 2.0 * y - 1.0;
    float t = 0.45 * yn;
    t -= (t * (4.0 * t * t - 3.0) + yn) / (12.0 * t * t - 3.0);
    return y;// saturate(t + 0.5);
}

float get_terrain_height(float3 input, float octaves) 
{
    float height = 0;
    const float3 alt_input = input + float3(55.4, 72.7, 35.1);
    const float scale_coeff = 30.0 * 7.0;

    // to start, get some noise that will be used to vary how rough the noises are and much they are distorted.
    const float roughness_noise = saturate(0.4 + 0.4 * NoiseTexture(input, 0.16 / scale_coeff, 5, 0.5, 1.5)); //bfm(input / scale_coeff,15 );//
    const float distortion_noise = saturate(0.5 + 0.6 * NoiseTexture(alt_input, 0.16 / scale_coeff, 5, 0.5, 0.5));

    const float mountain_noise = 1.0 - abs(2.0 * NoiseTexture(input, 3.6 / scale_coeff, 8, roughness_noise, distortion_noise)); //          
    //const float mountain_noise2 = aquagen(input / (190.7 * 7.0), 18);
    //return ;
    
    const float valley_noise = abs(2.0 * NoiseTexture(alt_input, 3.6 / scale_coeff, 8, roughness_noise, distortion_noise)); //0.5+0.5*
    height = mountain_noise * valley_noise; //
    const float c_s = 0.25; // continent_scale
    const float continental_noise = 0.5;
    saturate(lerp(
        flow(input / (490.7 * scale_coeff * c_s), 5) + (0.2 * bfm(input / (50 * scale_coeff * c_s), octaves)),
        1 * (bfm(input / (490.7 * scale_coeff * c_s), 6) + (0.2 * NoiseTexture(input, 0.067 / scale_coeff, 8, 0.39589, min(distortion_noise, 0.7)))), bfm(input / (91.97 * scale_coeff * c_s), 4)) ); //* 0.000025

    const float macro_vary_noise = saturate(1.3f + NoiseTexture(alt_input, 0.066 / scale_coeff, 7, roughness_noise, distortion_noise) - (1.0 * continental_noise));

    height = /*smooth*/max(macro_vary_noise + height, macro_vary_noise * height);

   
    height = invsmoothstep(-0.25 + (1.0 * height)) - 0.10;
    
    float subcontinent_noise = -0.15 + NoiseTexture(input, 0.6 / scale_coeff, 8, 0.59589, min(distortion_noise, 0.7));

    height *= 59.8;
    subcontinent_noise *= 53.4;

    height += subcontinent_noise;
    //...
    
    height = ((continental_noise * height) - (3.8f * continental_noise * continental_noise * continental_noise)) * 1.f * 3.0;
    float curved_height = 8.0 * pow(height / (400.f), 2.0f) * 40.f * sign(height);
    //return height;
    //return curved_height;
    //return valley_noise;
    return lerp(height * 16, curved_height, saturate(0.09 * (roughness_noise + subcontinent_noise + (0.2 * valley_noise)))); //(height + clamp(pow(height,3.0),height*2.f, 0));//*(clamp(subcontinent_noise / (scale * max(scale,1.0)), 1.0, 2.0)-0.0 );//

}
float get_alt_terrain_height(float2 input, float octaves) // a revised terrain algorithm after experimentation in blender
{
    float height = 0;
    const float2 alt_input = input + float2(55.4, 35.1);
    const float scale_coeff = 30.0 * 7.0;

    // to start, get some noise that will be used to vary how rough the noises are and much they are distorted.
    const float roughness_noise = saturate(0.4 +0.4 * NoiseTexture(input, 0.16/ scale_coeff, 5, 0.5, 1.5));//bfm(input / scale_coeff,15 );//
    const float distortion_noise = saturate(0.5 + 0.6 * NoiseTexture(alt_input, 0.16 / scale_coeff, 5, 0.5, 0.5));

    const float mountain_noise = 1.0-abs(2.0 * NoiseTexture(input, 3.6 / scale_coeff, 8, roughness_noise, distortion_noise));// 
    //const float mountain_noise2 = 1.0 - abs(2.0 * NoiseTexture(input, 0.1096 / scale_coeff, 8, roughness_noise, distortion_noise)); // 
    const float mountain_noise2 = aquagen(input / (190.7 * 7.0), 18);
    //return ;
    
    const float valley_noise = abs(2.0 * NoiseTexture(alt_input, 3.6 / scale_coeff, 8, roughness_noise, distortion_noise));//0.5+0.5*
    height = mountain_noise * valley_noise;//
    const float c_s = 0.25; // continent_scale
    const float continental_noise = lerp(
        flow(input / (490.7 * scale_coeff * c_s), 5) + (0.2 * bfm(input / (50 * scale_coeff * c_s), octaves)),
        1 * (bfm(input / (490.7 * scale_coeff * c_s), 6) + (0.2 * NoiseTexture(input, 0.067 / scale_coeff, 8, 0.39589, min(distortion_noise, 0.7)))), bfm(input / (91.97 * scale_coeff * c_s),4));

    // the lower this value (0.0), the more 'marbled' the terrain, creating swirly rivers and archipelagos;
    // the higher the value (1.0), the more regular the terrain, far fewer islands or river deltas.
    // low values -> western scotland; high values -> east coast scotland. 
    const float macro_vary_noise = saturate(1.3f+NoiseTexture(alt_input, 0.066 / scale_coeff, 7, roughness_noise, distortion_noise)- (1.0*continental_noise));

    height = /*smooth*/max(macro_vary_noise + height, macro_vary_noise * height);

    // APPLY SCALING/SMOOTHING (inv sm. step)...
   
    height = invsmoothstep(-0.25+(1.0* height))-0.10;
    
    float subcontinent_noise = -0.15+NoiseTexture(input, 0.6 / scale_coeff, 8, 0.59589, min(distortion_noise,0.7));

    height *= 59.8 ;
    subcontinent_noise *= 53.4 ;

    height += subcontinent_noise;
    //...
    
    height = ((continental_noise * height) - (3.8f * continental_noise * continental_noise * continental_noise)) * 10.f * 3.0;
    float curved_height = 8.0*pow(height / (400.f), 2.0f) * 40.f * sign(height);
    //return height;
    //return curved_height;
    //return (3.8f * continental_noise * continental_noise);
    return lerp( height * 16, curved_height, saturate(0.09 * (roughness_noise + subcontinent_noise + (0.2 * valley_noise))) ); //(height + clamp(pow(height,3.0),height*2.f, 0));//*(clamp(subcontinent_noise / (scale * max(scale,1.0)), 1.0, 2.0)-0.0 );//
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
    const int2 offset_pixel = { dispatchThreadID.x - (4096 / 2), dispatchThreadID.y - (4096 / 2) };
    const double c = 10.98 * pixel_vertex_scale; // scale factor ->: how many meters does 1 pixel width represent // 15,000/1500 = 10 // 7.68 = 11520 / 1500 { the world_position/size of the 3x3 terrain meshes }
    const float2 coords = {seed.x + ((offset_pixel.x * c) + globalPosition.x) / scale,
                           seed.y + ((offset_pixel.y * c) + globalPosition.y) / scale };
    const float coords3 = float3(1, coords.x / scale, coords.y / scale);
    const float height = get_alt_terrain_height(coords, 20.0);
    //const float height = bfm(float3(coords/scale, 1.0), 10 ); //simplex(float3(coords / scale, 1.0)); //
    
    
    const float macroScale = 3.0 * scale;
    const float2 macroCoords = { seed.x + ((dispatchThreadID.x * c) + globalPosition.x) / scale,
                                 seed.y + ((dispatchThreadID.y * c) + globalPosition.y) / scale };
    const float macro_height = get_alt_terrain_height(macroCoords, 14.0);

    // CALCULATE NORMAL
    const float3 normal = calculateNormal(coords3, 5.0); const float slope = normal.y; const float2 aspect = normal.xz;
    
    /*/ PREVAILING WINDS VECTOR
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
       
    //*/
    //float3 normal = calculateNormal(coords.xz, 10.0); unnecessary(?) just sample heightmap per vertex instead

	float4 output = float4(0, 0, 1, 1);

    output.a = 0 + height * scale * 0.025;
    output.r = max(height / (amplitude), 0);
   
    output.g = max(height / (2 * scale), 0);
    output.b = max(height / (amplitude), 0);
    //if (height < 0) { output.b = 1+height; }
    /// output.rgb -> humidity & wind?

    /*/ debug: for the pData array
    output.a = 1.0f;
    //output.r = dispatchThreadID.x / 1024.0f;
    output.g = dispatchThreadID.x;
    output.b = dispatchThreadID.y;//*/
    //output.rgb = height / 180.f; output.g = macro_height /(40.f*scale);
	gOutput[dispatchThreadID.xy] = output;

    //output = gOutput[(int2)(((dispatchThreadID.xy - 750) * macroScale) + 750)];
    
    /*if (false && manipulationDetails.z > 1.0) {
        output.r = gInput[dispatchThreadID.xy + wind.xy].r * 0.95
            + (gInput[dispatchThreadID.xy + (wind.xy * 2)].r * 0.8)
            + (gInput[dispatchThreadID.xy + (wind.xy * 4)].r * 0.6)
            + (gInput[dispatchThreadID.xy + (wind.xy * 7)].r * 0.4)
            + (gInput[dispatchThreadID.xy + (wind.xy * 11)].r * 0.25)
            + (gInput[dispatchThreadID.xy + (wind.xy * 16)].r * 0.1);
        output.r /= 400;
        gOutput[dispatchThreadID.xy] = output;
    }*/
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

float perlin(float2 world_position)
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
float ridges(float2 coords)
{
    return (1 - (4 * abs(perlin(coords)))) * perlin(coords.yx);
}

float terragen(float2 coords, int octs)
{
    float val = 0;
    float weight = 1;
    for (int o = 0; o < octs; o++)
    {
        int freqMultiplier = 4;//1.125;// default = 2
        float h = ridges(coords * pow(freqMultiplier, o)) / pow(2, o);//
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
        val += perlin(coords * pow(2, o)) / pow(2, o);
    }
    return val;
}
float aquagen(float2 coords, int octs)
{
    float val = 0;
    for (int o = 0; o < octs; o++)
    {
        float h = ridges(coords * pow(2, o)) / pow(2, o);
        //h *= h;
        val += h;

    }
    return val;
}
float flow(float2 i, int octaves)
{
    if (octaves <= 0)
        return bfm(i, 2);
    else
        return bfm(i + bfm(i + bfm(i, octaves / 3.0), octaves / 1.5), octaves);
}
float flow(float3 i, int octaves)
{
    if (octaves <= 0)
        return bfm(i, 2);
    else
        return bfm(i + bfm(i + bfm(i, octaves / 3.0), octaves / 1.5), octaves);
}
float NoiseTexture(float2 coords, float scale, int octs, float roughness, float distortion)
{
    float val = 0;
    //float weight = 1;

    //float xz = coords.x + (0.10);   // domain rotation
    //float s2 = xz * -0.211324865405187;
    //float yy = coords.y * 0.577350269189626;
    //float xr = coords.x + (s2 + yy);
    //float zr = (0.10) + (s2 + yy);
    //float yr = xz * -0.577350269189626 + yy;


    float2 d_coords = coords * scale * scale; //float2(yr,xr)

    if (distortion >= 0.0)
    {
        d_coords += float2(perlin(d_coords + random_offset(0.0)) * distortion, //
                           perlin(d_coords + random_offset(1.0)) * distortion); //
    }
    d_coords /= scale;
    
    for (int o = 0; o < octs; o++)
    {
        int freqMultiplier = 4; //1.125;// default = 2
        float h = perlin(scale * d_coords * pow(2, float(o))) * pow(saturate(roughness), float(o)); //
        //h *= h;
        //h *= weight;
        //weight = h;
        val += h;
    }
    return val; // ensure output is between 0 & 1.0
}

///#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#///
#ifndef __bcc_noise_4_hlsl_
#define __bcc_noise_4_hlsl_

/////////////// K.jpg's Simplex-like Re-oriented 4-Point BCC Noise ///////////////
//////////////////// Output: float4(dF/dx, dF/dy, dF/dz, value) ////////////////////

// Inspired by Stefan Gustavson's noise
float4 permute(float4 t)
{
    return t * (t * 34.0 + 133.0);
}

// https://stackoverflow.com/questions/7610631/glsl-mod-vs-hlsl-fmod
float mod(float x, float y)
{
    return x - y * floor(x / y);
}

float2 mod(float2 x, float2 y)
{
    return x - y * floor(x / y);
}

float3 mod(float3 x, float3 y)
{
    return x - y * floor(x / y);
}

float4 mod(float4 x, float4 y)
{
    return x - y * floor(x / y);
}

// Gradient set is a normalized expanded rhombic dodecahedron
float3 grad(float hash)
{
	// Random vertex of a cube, +/- 1 each
    float3 cube = mod(floor(hash / float3(1.0, 2.0, 4.0)), 2.0) * 2.0 - 1.0;

	// Random edge of the three edges connected to that vertex
	// Also a cuboctahedral vertex
	// And corresponds to the face of its dual, the rhombic dodecahedron
    float3 cuboct = cube;

    int index = int(hash / 16.0);

    if (index == 0)
        cuboct.x = 0.0;
    else if (index == 1)
        cuboct.y = 0.0;
    else
        cuboct.z = 0.0;

	// In a funky way, pick one of the four points on the rhombic face
    float type = mod(floor(hash / 8.0), 2.0);
    float3 rhomb = (1.0 - type) * cube + type * (cuboct + cross(cube, cuboct));

	// Expand it so that the new edges are the same length
	// as the existing ones
    float3 grad = cuboct * 1.22474487139 + rhomb;

	// To make all gradients the same length, we only need to shorten the
	// second type of vector. We also put in the whole noise scale constant.
	// The compiler should reduce it into the existing floats. I think.
    grad *= (1.0 - 0.042942436724648037 * type) * 32.80201376986577;

    return grad;
}

// BCC lattice split up into 2 cube lattices
float4 openSimplex2Base(float3 X)
{
	// First half-lattice, closest edge
    float3 v1 = round(X);
    float3 d1 = X - v1;
    float3 score1 = abs(d1);
    float3 dir1 = step(max(score1.yzx, score1.zxy), score1);
    float3 v2 = v1 + dir1 * sign(d1);
    float3 d2 = X - v2;

	// Second half-lattice, closest edge
    float3 X2 = X + 144.5;
    float3 v3 = round(X2);
    float3 d3 = X2 - v3;
    float3 score2 = abs(d3);
    float3 dir2 = step(max(score2.yzx, score2.zxy), score2);
    float3 v4 = v3 + dir2 * sign(d3);
    float3 d4 = X2 - v4;

	// Gradient hashes for the four points, two from each half-lattice
    float4 hashes = permute(mod(float4(v1.x, v2.x, v3.x, v4.x), 289.0));
    hashes = permute(mod(hashes + float4(v1.y, v2.y, v3.y, v4.y), 289.0));
    hashes = mod(permute(mod(hashes + float4(v1.z, v2.z, v3.z, v4.z), 289.0)), 48.0);

	// Gradient extrapolations & kernel function
    float4 a = max(0.5 - float4(dot(d1, d1), dot(d2, d2), dot(d3, d3), dot(d4, d4)), 0.0);
    float4 aa = a * a;
    float4 aaaa = aa * aa;
    float3 g1 = grad(hashes.x);
    float3 g2 = grad(hashes.y);
    float3 g3 = grad(hashes.z);
    float3 g4 = grad(hashes.w);
    float4 extrapolations = float4(dot(d1, g1), dot(d2, g2), dot(d3, g3), dot(d4, g4));

    float4x3 derivativeMatrix = { d1, d2, d3, d4 };
    float4x3 gradientMatrix = { g1, g2, g3, g4 };

	// Derivatives of the noise
    float3 derivative = -8.0 * mul(aa * a * extrapolations, derivativeMatrix)
		+ mul(aaaa, gradientMatrix);

	// Return it all as a float4
    return float4(derivative, dot(aaaa, extrapolations));
}

// Use this if you don't want Z to look different from X and Y
float openSimplex2_Conventional(float3 X)
{
	// Rotate around the main diagonal. Not a skew transform.
    float4 result = openSimplex2Base(dot(X, float3(2.0 / 3.0, 2.0 / 3.0, 2.0 / 3.0)) - X);
    //return float4(dot(result.xyz, float3(2.0 / 3.0, 2.0 / 3.0, 2.0 / 3.0)) - result.xyz, result.w);
    return result.w;
}

// Use this if you want to show X and Y in a plane, then use Z for time, vertical, etc.
float4 openSimplex2_ImproveXY(float3 X)
{
	// Rotate so Z points down the main diagonal. Not a skew transform.
    float3x3 orthonormalMap =
    {
        0.788675134594813, -0.211324865405187, -0.577350269189626,
			-0.211324865405187, 0.788675134594813, -0.577350269189626,
			0.577350269189626, 0.577350269189626, 0.577350269189626
    };

    float4 result = openSimplex2Base(mul(X, orthonormalMap));
    return float4(mul(orthonormalMap, result.xyz), result.w);
}

//////////////////////////////// End noise code ////////////////////////////////

#endif

float bfm(float3 coords, int octs)
{
    float val = 0;
    float weight = 1;
    for (int o = 0; o < octs; o++)
    {
        val += simplex(coords * pow(2, o)) / pow(2, o);
    }
    return val;
}
float3 random_offset3(float seed)
{ // random float3 between 100 & 200
    return float3(100.0 + hash12(float2(seed, 0.0)) * 100.0,
                  100.0 + hash12(float2(seed, 1.0)) * 100.0,
                  100.0 + hash12(float2(seed, -8.7)) * 100.0);
}
float NoiseTexture(float3 coords, float scale, int octs, float roughness, float distortion)
{
    float val = 0;


    float3 d_coords = coords * scale * scale; //float2(yr,xr)

    if (distortion >= 0.0)
    {
        d_coords += float3(simplex(d_coords + random_offset3(0.0)) * distortion, //
                           simplex(d_coords + random_offset3(1.0)) * distortion, //
                           simplex(d_coords + random_offset3(-2.0)) * distortion); //
    }
    d_coords /= scale;
    
    for (int o = 0; o < octs; o++)
    {
        int freqMultiplier = 4; //1.125;// default = 2
        float h = simplex(scale * d_coords * pow(2, float(o))) * pow(saturate(roughness), float(o)); //

        val += h;
    }
    return val; // ensure output is between 0 & 1.0
}