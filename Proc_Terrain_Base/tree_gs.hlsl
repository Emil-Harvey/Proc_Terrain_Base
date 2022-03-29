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
float bfm(float4 coords, int octaves)
{
    float val = 0;
    for (int o = 0; o < octaves; o++)
    {
        val += perlin(coords * pow(2, o)) / pow(2, o);
    }
    return val;
}
///#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#/#///

cbuffer MatrixBuffer : register(b0)
{// float4x4
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};
cbuffer ChunkBuffer : register(b1)
{
    float2 chunkPos;
    float scale;
    float timeOfYear;
    int resolution;//tessellationFactor
    float3 manipulationDetails;
    //
    //float2 globalPosition;/// 
    //float2 float2 padding_;// cameraPosition;//
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
    

    //float steepness : PSIZE0;
    //float temperature : PSIZE6;
    //float snowness : PSIZE1;
    //float beachness : PSIZE2;
    //float noise : PSIZE3;
    //float noise2 : PSIZE4;
    //float humidity : PSIZE5;
    //float2 wind : TEXCOORD1;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float4 world_position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float treeType : PSIZE0; //  (int)
    float4 colourMultiplier : COLOR0;
};

//  pre-define
void generateQuad(float4 wp, OutputType output, float quadSize, bool wind, inout TriangleStream<OutputType> triStream);

matrix inverse(matrix input);

[maxvertexcount(6)]
void main(point InputType input[1], inout TriangleStream<OutputType> triStream)
{
    OutputType output;



   //
   // const float PI180 = 57.29577;
   // const float4 coords = input[0].world_position + float4(400, 0, 324, 0);
    const float altitude = input[0].world_position.y;
    const float latitude = input[0].world_position.z;//abs(input[0].world_position.z);
   // const float longditude = abs(input[0].world_position.x);
   // const float slope = input[0].normal.y;
   // 
    //const int freq = pow(2, min(int(scale),3)); //1.0; //1.298730940; //grid-frequency (scale of the grid) on which trees may be planted 
    
    //// flat to cliff attenuation
    //float steepness = saturate(pow(saturate(slope) + 0.18 + (0.12 * bfm(coords / (12 * scale), 4)), 100));
    //
    //// snowline // altitude - high near the middle (equator)
    //float snowline = (2 + 2.4 * cos(latitude * 0.0035)
    //                 + (cos(timeOfYear / PI180) * -sin(latitude * 0.00175)))
    //                 * scale;
    //// this represents temperature/climate, specifically how cold
    //float snowness = saturate(0.2 * altitude - bfm(coords / (2 * scale), 4) - snowline);
    //                                                      
    ////beachline
    //float beachline = 0.2; //altitude at which beaches meet soil
    //float beachness = 1 - saturate(slope) * saturate(2 * altitude - bfm(coords / (12* scale), 4) - beachline); // ;
    //
    //float noise = pow(saturate(1 - bfm(coords / (20*scale), 5)), 3);
    //float noise2 = pow(saturate(1 - bfm(19 + coords / (10 * scale), 3)), 3);
    ////float offset = abs(noise2 - (int) noise2);
   /////// float gridAlignment;
    
    float quadSize = -1;
    
    output.colourMultiplier = float4(1, 0.8, 1, 1);
    

    if (altitude >= 0)// not on water
    { /*
        if (input[0].steepness > 0.5)
        { //not too steep to grow
            if (input[0].snowness > 0.5)// snow biome
            {
                gridAlignment =gridAlignment = abs(fmod(latitude * freq, 1)) + abs(fmod(longditude * freq, 1)); //fmod(latitude * freq + offset, 2 - noise) + fmod(longditude * freq, 2 - noise);
                if (gridAlignment >= -1.0 / resolution && gridAlignment < 1.0 / resolution && input[0].noise <= 0.3 && input[0].noise2 > 0.32)// if the tree is  within 1 vertex space of the theoretical "tree placement grid"
                {
                
                    output.treeType = 2;// snow pine
                    quadSize = 2.0f + (6 * input[0].snowness);
                }
            }
            else if (input[0].beachness < 0.5)
            { //
                if (input[0].humidity > 0.3)// grass biome
                {
                
                    if (input[0].noise > 0.6 && input[0].snowness < 0.2)// plains
                    {
                        gridAlignment = abs(fmod(latitude * freq, 1)) + abs(fmod(longditude * freq, 1));
                        if (gridAlignment >= -1.0 / resolution && gridAlignment < 1.0 / resolution && input[0].noise2 > 0.595 && input[0].noise2 < 0.6)
                        {
                            output.treeType = 3; // bush
                            quadSize = 2.50f - bfm(coords, 1);
                        }
                    }
                    else // woods
                    {
                        if (input[0].noise > 0.3)// outer woods
                        {
                            gridAlignment = abs(fmod(latitude * freq, 1)) + abs(fmod(longditude * freq, 1));
                            if (gridAlignment > -1.0 / resolution && gridAlignment < 1.0 / resolution && input[0].noise2 > 0.52)
                            {
                                output.treeType = 0; // oak
                                quadSize = 5.50f - (4.5 * bfm(coords, 1));
                            
                                output.colourMultiplier.g = max(pow(1 - input[0].snowness, 0.2), 0.752);
                                output.colourMultiplier.r = max(pow(input[0].snowness, 0.8) * 1.7, 1);
                            }
                        }
                        else // dense woods / high altitude woods
                        { //      * 2        * 2 + offset
                            gridAlignment = abs(fmod(latitude * freq, 1)) + abs(fmod(longditude * freq, 1));
                            if (gridAlignment >= -1.0 / resolution && gridAlignment < 1.0 / resolution && input[0].noise2 > 0.32)
                            {
                                output.treeType = 1; // pine
                                quadSize = 7.50f - (12 * bfm(coords, 1));
                            
                            }
                        }
                    }
                }
                else
                {// savannah/desert biome
                    
                }
                
            }
        }//*/
 

        output.treeType = 3;//int(2+ 2*sin(latitude)); 

        float3 crosp = cross(normalize(float3(viewMatrix[0][2],
            viewMatrix[1][2],
            viewMatrix[2][2])), normalize(input[0].world_position.xyz - camera_pos.xyz));
        bool pos_infront_of_camera = length(crosp) < 0.850; /// abs(a+b) < abs(a) then b goes 'backwards' from a

        if (pos_infront_of_camera && length(input[0].world_position.xz - camera_pos.xz) < 700)
            quadSize = 5 + 1.5 * perlin(input[0].world_position + 0.4115); //+ (6 * input[0].snowness);


    }
    

      if (quadSize > 0)
        generateQuad(input[0].world_position, output, quadSize, true, triStream);
}

//#######################################################################//
//#######################################################################//
//#######################################################################//

/*matrix inverse(matrix A) {
h = 1  Initialization of the pivot row 
k = 1 /* Initialization of the pivot column * /

while (h <= m && k <= n)
    /* Find the k-th pivot: * /
    i_max = argmax(i = h ... m, abs(A[i][k]))
    if A[i_max][k] = 0
        /* No pivot in this column, pass to next column * /
        k = k + 1
    else
        swap rows(h, i_max)
        /* Do for all rows below pivot: * /
        for i = h + 1 ... m:
        f = A[i][k] / A[h][k]
        /* Fill with zeros the lower part of pivot column: * /
        A[i][k]  = 0
        /* Do for all remaining elements in current row: * /
        for j = k + 1 ... n :
            A[i][j] = A[i][j] - A[h][j] * f
    /* Increase pivot row and column * /
    h = h + 1
    k = k + 1
}//*/

//              InputType input[1]
void generateQuad(float4 wp, OutputType output, float quadSize, bool wind, inout TriangleStream<OutputType> triStream)
{

    //     generate quad

   // create billboard vectors     reference: https://www.geeks3d.com/20140815/particle-billboarding-with-the-geometry-shader-glsl/
//float3 rightVector = float3(viewMatrix[0][0],
//                                viewMatrix[1][0],
//                                viewMatrix[2][0]);
    float3 rightVector = normalize(float3(2.83 * perlin(wp + 2.1235), 0, 1.83 * perlin(wp + 6.3211)));// variation
    float3 displacementVector = float3(0, 0, 0);
    if (wind) { displacementVector.x += perlin(wp + (timeOfYear * 600)); }
    float3 upVector = float3(0, 1, 0);

    upVector *= 2; // so the width & height of the quad are 2:1 while not being centered on the pivot
// (the pivot of the billboard is at the bottom, since it is a plant).

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
    output.tex = float2(0.0, 1.0);
    output.normal = normalize(upVector);

    triStream.Append(output);

    // bottom right
    output.world_position = wp;//input[0].world_position;//mul(input.position, worldMatrix);//
    output.world_position.w = 1.0;
    output.world_position.xyz = output.world_position.xyz + ((rightVector - 0) * quadSize);
    output.position = mul(output.world_position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    output.tex = float2(1.0, 1.0);
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
    output.tex = float2(1.0, 0.0);
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
    output.tex = float2(1.0, 1.0);
    output.normal = normalize(upVector);

    triStream.Append(output);

    triStream.RestartStrip();
}