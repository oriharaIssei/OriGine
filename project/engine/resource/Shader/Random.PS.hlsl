#include "FullScreen.hlsli"

#include "Random.hlsli"

///=============================================================================
// Random
///=============================================================================

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
    float randomValue = rand2dTo1d(input.texCoords * gRandomParams.time);
    output.color = float4(randomValue, randomValue, randomValue, 1.0);
    
    return output;
}
