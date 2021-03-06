cbuffer timeBuffer: register(b0) {
    float timeP;
    float timeOfYear;// number between 0-360 ish
    int day;
    float padding;
}
cbuffer CameraBuffer : register(b1)
{
    //float3 viewpos;
    //float pudding;
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};
cbuffer posBuffer : register(b2)
{
    float4 camera_pos;
}


struct InputType
{
    float4 position : SV_POSITION;
    float4 world_position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float4 world_position : POSITION;
};

float hash(float x, float y)// random value betwn -1 & 1
{
    float seed = 21732.37f;
    float random = -y+(seed / 100) * sin(x * (seed / 11.0923) + y * (seed / 2.62349) + seed / 32.12379) * cos(x * (seed / 8.812934) * y + y * y * seed + (seed / 23.792173));
    //float random = seed * 9.01230387 + sin(x + y * seed) * cos(x * y + seed) * seed + x + x / y + seed * y - sin(seed - y);// ^ was producing artefacts

    return (float)cos(random);// ret;
}

//pre-define func
void generateQuad(float4 wp, OutputType output, float quadSize, bool wind, inout TriangleStream<OutputType> triStream);
float perlin(float4 world_position);

[maxvertexcount(6)]
void main(point InputType input[1], inout TriangleStream<OutputType> triStream)
{
    OutputType output;

    /// preliminary definitions
    const float noise = hash(input[0].world_position.x, input[0].world_position.y+0993.32621);
    
    const float time_taken_to_reach_ground = 50;// time in seconds roughly that the coefficient takes to reset to 0. i.e., the period of the rain
    const float time_interpolator_coefficient = (noise + timeOfYear * 500 / time_taken_to_reach_ground) % 1; // a number between 0 and 1, that resets roughly every 10 seconds [on 1.0x timescale]
    const float particle_age = time_interpolator_coefficient * time_taken_to_reach_ground; // age in seconds since spawn

    //const float gravity = -9.8; // acceleration add air resistance?
    const float4 particle_speed = { 0, -2000, 0, 0 };//{ 0, 0 + gravity * particle_age, 0, 0};
    const float4 particle_displacement = particle_speed * particle_age * 0.5;
    /*const*/ float4 particle_position = input[0].world_position + particle_displacement; //+ float4(camera_pos.x,0, camera_pos.z, 0);
    //if (particle_position.y < -100)
    //    particle_position.y += 400;// test floor
    particle_position.y = 100 + particle_position.y % 500;

    float quadSize = 200;
    //
    generateQuad(particle_position, output, quadSize, true, triStream);
}


//              InputType input[1]
void generateQuad(float4 wp, OutputType output, float quadSize, bool wind, inout TriangleStream<OutputType> triStream)
{

    //     generate quad

   // create billboard vectors     reference: https://www.geeks3d.com/20140815/particle-billboarding-with-the-geometry-shader-glsl/
    float3 upVector = float3(0, 1, 0);
    float3 fwdVector = (camera_pos - wp).xyz; fwdVector.y = 0;
    float3 rightVector = cross(normalize(fwdVector), upVector);  //float3(viewMatrix[0][0], viewMatrix[1][0],viewMatrix[2][0]);
    //float3 rightVector = normalize(float3(2.83 * perlin(wp + 2.1235), 0, 1.83 * perlin(wp + 6.3211)));// variation
    float3 displacementVector = float3(0, 0, 0); // top verts
    //if (wind) { displacementVector.x += perlin(wp + (timeOfYear * 600)); }
    

    upVector *= 8; // so the width & height of the quad are 2:2 while not being centered on the pivot
// (the pivot of the billboard is at the bottom, since the first raindrops to poke out from the cloud are there).

    float alignment = 5.0;

    // Move the vertex away from the point position
    // top left
    output.world_position = wp;//input[0].world_position;//mul(input.position, worldMatrix);//
    output.world_position.w = 1.0;
    output.world_position.xyz = output.world_position.xyz - ((rightVector - upVector + displacementVector) * quadSize);
    output.position = mul(output.world_position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    output.tex = float2(0.0, 0.0);

    output.normal = normalize(upVector);

    triStream.Append(output);


    // bottom left
    output.world_position = wp;//input[0].world_position;//mul(input.position, worldMatrix);//
    output.world_position.w = 1.0;
    output.world_position.xyz = output.world_position.xyz - ((rightVector + 0) * quadSize);
    output.position = mul(output.world_position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    output.tex = float2(0.0, 10.0);
    output.normal = normalize(upVector);

    triStream.Append(output);

    // bottom right
    output.world_position = wp;//input[0].world_position;//mul(input.position, worldMatrix);//
    output.world_position.w = 1.0;
    output.world_position.xyz = output.world_position.xyz + ((rightVector - 0) * quadSize);
    output.position = mul(output.world_position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    output.tex = float2(10.0, 10.0);
    output.normal = normalize(upVector);


    triStream.Append(output);

    // end of the first triangle

    triStream.RestartStrip();// commenting out does nothing?

// top right
    output.world_position = wp;//input[0].world_position;//mul(input.position, worldMatrix);//
    output.world_position.w = 1.0;
    output.world_position.xyz = output.world_position.xyz + ((rightVector + upVector + displacementVector) * quadSize);
    output.position = mul(output.world_position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    output.tex = float2(10.0, 0.0);
    output.normal = normalize(upVector);

    triStream.Append(output);

    //top left
    output.world_position = wp;//input[0].world_position;//mul(input.position, worldMatrix);//
    output.world_position.w = 1.0;
    output.world_position.xyz = output.world_position.xyz - ((rightVector - upVector + displacementVector) * quadSize);

    output.position = mul(output.world_position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    output.tex = float2(0.0, 0.0);
    output.normal = normalize(upVector);

    triStream.Append(output);

    // bottom right
    output.world_position = wp;//input[0].world_position;//mul(input.position, worldMatrix);//
    output.world_position.w = 1.0;

    output.world_position.xyz = output.world_position.xyz + (rightVector - 0) * quadSize;

    output.position = mul(output.world_position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    output.tex = float2(10.0, 10.0);
    output.normal = normalize(upVector);

    triStream.Append(output);

    triStream.RestartStrip();
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
    float seed = 21732.37f;
    float random = (seed / 100) * sin(x * (seed / 11.0923) + y * (seed / 2.62349) + seed / 32.12379) * cos(x * (seed / 8.812934) * y + y * y * seed + (seed / 23.792173));
    //float random = sin(x + y) * cos(x * y);
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
float bfm(float4 coords, int octaves)
{
    float val = 0;
    for (int o = 0; o < octaves; o++)
    {
        val += perlin(coords * pow(2, o)) / pow(2, o);
    }
    return val;
}