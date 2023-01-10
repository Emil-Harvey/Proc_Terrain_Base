
#include "Noise.h"


///* Function to interpolate between a and ab
// Weight w should be in the range (0.0, 1.0)

float Noise::interpolate(float a, float b, float w) {
    return pow(w, 2.0) * (3.0 - 2.0 * w) * (b - a) + a;
}//*/   w^2 x (3-2w)(b-a) + a
float Noise::lerp(float a, float b, float w) {
    return pow(w, 2.0) * (3.0 - 2.0 * w) * (b - a) + a;
}
float Noise::smooth_min(const float& a, const float& b, const float fac = 1.0f)
{
    if (fac != 0.0) {
        float h = max(fac - abs(a - b), 0.0) / fac;
        return min(a, b) - h * h * h * fac * (1.0 / 6.0);
    }
    else {
        return min(a, b);
    }
}

/*float hash12(vect2 p)// 2 inputs, 1 output (SLOW)
{
    vect3 p3 = fract(vect3(p.x,p.y,p.x) * .1031389675);
    p3 = p3+ dot(p3, vect3(p3.y, p3.z, p3.x) + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}*/
float hash12(const vect2& p) {
    return(1.0 - fract(p.x * 10.f) * fract(tan(p.y)));
}

vect3 random_offset3(float seed)
{ // random float3 between 100 & 200
    return vect3(100.0 + hash12(vect2(seed, 0.0)) * 100.0,
        100.0 + hash12(vect2(seed, 1.0)) * 100.0,
        100.0 + hash12(vect2(seed, -8.7)) * 100.0);
}

/* Create random direction vector
 */
Noise::vector2 Noise::gradient(const int& x,const int& y) {
    // makes the pseudorandom gradients for the given grid coordinate
    const float seed = 217832.f;
    float random = (seed /100) *sin(x * (seed /10) + y * (seed /2) + seed /30)* cos(x * (seed /10) * y * seed + (seed / 20));
    //float random = sin(x + y) * cos(x * y);
    vector2 ret;
    ret.x = (float)cos(random);
    ret.y = (float)sin(random);
    return ret;
}

// Computes the dot product of the distance and gradient vectors.
float Noise::dotGradient(int ix, int iy, float x, float y) {
    // make pseudorandom gradient from integer coordinates
    vector2 grad = gradient(ix, iy);

    //distance to the given grid point 
    float xDistance = x - (float)ix;
    float yDistance = y - (float)iy;

    //dot-product of the distance & gradient vectors
    return (xDistance * grad.x + yDistance * grad.y);
}

// Compute Perlin noise at coordinates x, y (return -0.5 to ~0.5)
float Noise::perlin(float x, float y) {
    // find grid cell coordinates
    int x0 = (int)x;// west side
    int x1 = x0 + 1;// east side
    int y0 = (int)y;// south side
    int y1 = y0 + 1;// north side

    // get interpolation weights; AKA distance to the southeast grid coordinate (local coordinates)
    float xWeight = x - (float)x0;
    float yWeight = y - (float)y0;

    // bilinearly interpolate between grid point gradients at the 4 corners of the grid cell
    float n0, n1, i0, i1, value;

    n0 = dotGradient(x0, y0, x, y);// southeast cell corner
    n1 = dotGradient(x1, y0, x, y);// southwest cell corner
    i0 = interpolate(n0, n1, xWeight);// southern gradient weighting

    n0 = dotGradient(x0, y1, x, y);// northeast cell corner
    n1 = dotGradient(x1, y1, x, y);// northwest cell corner
    i1 = interpolate(n0, n1, xWeight);// northern gradient weighting

    value = interpolate(i0, i1, yWeight);// actual height
    return value;
}
// make ridge-like noise using absolute values
float Noise::ridges(float x, float y)
{
    return (1 - (4 * abs(perlin(x, y)))) * perlin(y, x);
}
// noise where x & y are offset using noise
float Noise::liquid(float x, float y,float offsetAmount)
{
    return perlin(x+(offsetAmount * perlin(x,y)), y + (offsetAmount * perlin(y,x)));
}
float Noise::flow(vect3 i, int octaves)
{
    if (octaves <= 0)
        return Simplex::fBm(i, 2);
    else
        return Simplex::fBm(i + Simplex::fBm(i + Simplex::fBm(i, octaves / 3.0), octaves / 1.5), octaves);
}
float Noise::NoiseTexture(vect3 coords, float scale, int octs, float roughness, float distortion)
{

    float val = 0;
    //float weight = 1;

    //float xz = coords.x + (0.10);   // domain rotation
    //float s2 = xz * -0.211324865405187;
    //float yy = coords.y * 0.577350269189626;
    //float xr = coords.x + (s2 + yy);
    //float zr = (0.10) + (s2 + yy);
    //float yr = xz * -0.577350269189626 + yy;


    vect3 d_coords = coords * scale * scale; //float2(yr,xr)

    if (distortion >= 0.0)
    {
        d_coords += vect3(
            Simplex::noise(d_coords + random_offset3(0.0)) * distortion, //
            Simplex::noise(d_coords + random_offset3(1.0)) * distortion, //
            Simplex::noise(d_coords + random_offset3(1.0)) * distortion ); //
    }
    d_coords = d_coords / scale;

    for (int o = 0; o < octs; o++)
    {
        int freqMultiplier = 4; //1.125;// default = 2
        float h = Simplex::noise(d_coords * scale *  pow(2, float(o))) * pow(saturate(roughness), float(o)); //
        //h *= h;
        //h *= weight;
        //weight = h;
        val += h;
    }
    return val; // ensure output is between 0 & 1.0

}

float Noise::terrain_height(float x, float y, float z, int octaves)
{
    
    float height = 0;
    const vect3 input = { x,y,z };
    const vect3 alt_input = input + vect3(55.4, -72.7, 35.1);
    const float scale_coeff = 30.f * 7.f;
    const int min_octs = max(4 * (octaves / 10), 1);

    // to start, get some noise that will be used to vary how rough the noises are and much they are distorted.
    const float roughness_noise = saturate(0.4 + 0.4 * NoiseTexture(input, 1.6 / scale_coeff, min_octs + 1, 0.5, 1.5)); //bfm(input / scale_coeff,15 );//
    const float distortion_noise = saturate(0.5 + 0.6 * NoiseTexture(alt_input, 1.6 / scale_coeff, min_octs + 1, 0.5, 0.5));

    const float hill_noise = 1.0 - abs(2.0 * NoiseTexture(input, 3.6 / scale_coeff, min_octs + 4, roughness_noise, distortion_noise)); //          
    //const float mountain_noise2 = aquagen(input / (190.7 * 7.0), 18);
    //return ;

    const float valley_noise = abs(2.f * NoiseTexture(alt_input, 3.6 / scale_coeff, min_octs + 4, roughness_noise, distortion_noise)); //0.5+0.5*
    height = hill_noise * valley_noise; //
    height = 1;
    const float c_s = 2.50; // continent_scale
     float continental_noise = 3*
    (lerp(
        flow(input / (490.7 * scale_coeff * c_s), min_octs + 1) + (0.2 * Simplex::fBm(input / (50 * scale_coeff * c_s), octaves)),
        1 * (Simplex::fBm(input / (490.7 * scale_coeff * c_s), min_octs + 2) + (0.2 * NoiseTexture(input, 0.067 / scale_coeff, 8, 0.39589, min(distortion_noise, 0.7)))), Simplex::fBm(input / (91.97 * scale_coeff * c_s), min_octs))); //* 0.000025
    //return continental_noise;
    const float macro_vary_noise = saturate(1.3f + NoiseTexture(alt_input, 0.066 / scale_coeff, min_octs + 3, roughness_noise, distortion_noise) - (1.0 * continental_noise));

    height = /*smooth*/max(macro_vary_noise + height, macro_vary_noise * height);


    //height = interpolate(0,1, -0.25 + (1.0 * height)) - 0.10;

    float subcontinent_noise =  -0.12 + NoiseTexture(input, 1.071 / scale_coeff, min_octs *2, 0.59589, min(distortion_noise, 0.7))*0.5;

    height *= 59.8;
    subcontinent_noise *= 53.4;

    height += subcontinent_noise;
    //...

    height += ((continental_noise * height) - (3.8f * continental_noise * continental_noise * continental_noise)) * 3.0;
    float curved_height = 8.0 * pow(height / (300.f), 2.0f) * 40.f * sign(height);
    curved_height = min(curved_height, height + 400.f );//smooth_min(curved_height, height + 400.f, 100.f);
    //return height;
    //return curved_height;
    //return valley_noise;
    const float curve_variance = saturate(0.4 + 0.4 * NoiseTexture(input + alt_input, 0.731 / scale_coeff, min_octs + 1, 0.402, 0.31)); //bfm(input / scale_coeff,15 );//
    //return curve_variance * 8100;
    return -100.f + lerp(height * 1.f, 4*curved_height, saturate(0.09 * (curve_variance + subcontinent_noise + (0.2 * valley_noise)))); //(height + clamp(pow(height,3.0),height*2.f, 0));//*(clamp(subcontinent_noise / (scale * max(scale,1.0)), 1.0, 2.0)-0.0 );//


}


