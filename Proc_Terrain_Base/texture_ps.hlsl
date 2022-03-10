// Texture pixel / fragment shader
// Basic fragment shader for rendering textured geometry

// Texture and sampler registers
Texture2D texture0 : register(t0);
SamplerState Sampler0 : register(s0);

struct InputType
{
    float4 position : POSITION;
    //float4 worldPosition : POSITION1;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;

};

float bfm(float4 coords, int octs);

float noise(float2 xy) {
    // makes the random gradients for the given grid coordinate
    float seed = 74981.87126386f;//21732.37f;
    float random = seed * 9.01230387 + sin(xy.x + xy.y * seed) * cos(xy.x * xy.y + seed) * seed + xy.x + xy.x / xy.y + seed * xy.y - sin(seed - xy.y);
    //return fmod(random, 1);
    return 0.5*sin(random)+0.5;
}

float4 main(InputType input) : SV_TARGET
{
	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
	float4 textureColour = texture0.Sample(Sampler0, input.tex);

   // textureColor.rgb *= 0.8;
   // textureColor.rgb += 0.19;
   // textureColor.rgb *= textureColor.rgb;
    //textureColor.b = max(textureColor.b, 0.05);

   
        ///     Dithering Shader                                             
    //float thres = 0.5 + 0.5*noise(float2(input.tex.x*14,input.tex.y)); //0.50+ 0.25*( sin(input.tex.x*3201.32) + sin(input.tex.y*1801.18) );//0.5 * (0 + bfm(input.tex.xxyy*998.3, 13));//
    // int rX = 1368;int rY = 743;     int x = ((input.tex.x * rX) % 2); int y = ((input.tex.y * rY) % 2);
    //float thres = 0.23 + (abs(3*x - 2*y) / 9.0);// 0.50 + 0.5 * 
    //float bits = 2.0;
    //textureColour.rgb = float3(int3((textureColour.rgb +thres) * bits)) / bits;
    //if (textureColour.r > thres) textureColour.r = 0.70;
    //else textureColour.r = 0.1850;
    //if (textureColour.g > thres) textureColour.g = 0.90;
    //else textureColour.g = 0.2150;
    //if (textureColour.b > thres) textureColour.b = 0.890;
    //else textureColour.b = 0.150;
        /// monochrome:
    //if (length(textureColour.rgb)/*0*/ > thres) textureColour.rgb = float3(0.3, 0.7, 0.4);// 1.0;// 
    //else textureColour.rgb = float3(0.1,0.06,0.09);// 0.0;// 
    //textureColour.rgb = thres;

    //textureColour.a = length(textureColour.rgb);
    clip(textureColour.a < 0.00623 ? -1 : 1);

	return textureColour;
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
    float seed = 74981.87126386f;//21732.37f;
    //float random = (seed / 100) * sin(x * (seed / 11.0923) + y * (seed / 2.62349) + seed / 32.12379) * cos(x * (seed / 8.812934) * y + y * y * seed + (seed / 23.792173));
    float random = seed * 9.01230387 + sin(x + y * seed) * cos(x * y + seed) * seed + x + x / y + seed * y - sin(seed - y);
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

float perlin(float4 world_position)
{
    float x = abs(world_position.x);
    float y = abs(world_position.z);

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