#include "FullScreen.hlsli"

///=============================================================================
// Grayscale BT.709
///=============================================================================


/// ------------------------------------------------------------------
// buffers
/// ------------------------------------------------------------------
Texture2D<float4> gTexture : register(t0); // input texture
SamplerState gSampler : register(s0); // input sampler

/// ------------------------------------------------------------------
// main
/// ------------------------------------------------------------------
PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 color = gTexture.Sample(gSampler, input.texCoords);
    // BT.709 grayscale conversion
    float gray = dot(color.rgb, float3(0.2125f, 0.7154f, 0.0721f));
    output.color = float4(gray, gray, gray, color.a);
    return output;
}
