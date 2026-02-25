#include "FullScreen.hlsli"

///=============================================================================
// Grayscale BT.709
///=============================================================================

struct GrayScaleBuff
{
    float grayscaleAmount;
};

/// ------------------------------------------------------------------
// buffers
/// ------------------------------------------------------------------
Texture2D<float4> gTexture : register(t0); // input texture
SamplerState gSampler : register(s0); // input sampler
ConstantBuffer<GrayScaleBuff> gGrayScaleBuff : register(b0); // grayscale parameters

/// ------------------------------------------------------------------
// main
/// ------------------------------------------------------------------
PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    float4 color = gTexture.Sample(gSampler, input.texCoords);

    // BT.709 grayscale conversion
    float gray = dot(color.rgb, float3(0.2125f, 0.7154f, 0.0721f));

    output.color = float4(
    lerp(color.rgb, float3(gray, gray, gray), gGrayScaleBuff.grayscaleAmount),
    color.a);
    
    return output;
}
