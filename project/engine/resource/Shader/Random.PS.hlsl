#include "FullScreen.hlsli"

///=============================================================================
// Random
///=============================================================================

float random(float3 _input)
{
    float random = dot(_input, float3(12.9898, 78.233, 37.719));
    random = frac(sin(random) * 143758.5453);
    return random;
}

/// ------------------------------------------------------------------
// buffers
/// ------------------------------------------------------------------
struct RandomParam
{
    float time;
};
ConstantBuffer<RandomParam> gRandomParams : register(b0); // Random parameters

/// ------------------------------------------------------------------
// main
/// ------------------------------------------------------------------
PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float randomValue = random(float3(input.texCoords, 0.0) * gRandomParams.time);
    output.color = float4(randomValue, randomValue, randomValue, 1.0);
    
    return output;
}
