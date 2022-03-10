
#include "Noise.h"

Noise::Noise()
{
}

Noise::~Noise()
{
}
///* Function to interpolate between a and ab
// Weight w should be in the range (0.0, 1.0)

float Noise::interpolate(float a, float b, float w) {
    return pow(w, 2.0) * (3.0 - 2.0 * w) * (b - a) + a;
}//*/   w^2 x (3-2w)(b-a) + a



/* Create random direction vector
 */
Noise::vector2 Noise::gradient(int x, int y) {
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


