#include "FullScreen.hlsli"

///=============================================================================
// Vignette
///=============================================================================

/// ------------------------------------------------------------------
// buffers
/// ------------------------------------------------------------------
struct VignetteParams
{
    float4 color;
    float scale;
    float pow;
};
ConstantBuffer<VignetteParams> gVignetteParams : register(b0); // Vignette parameters
Texture2D<float4> gTexture : register(t0); // input texture
SamplerState gSampler : register(s0); // input sampler

/// ------------------------------------------------------------------
// main
/// ------------------------------------------------------------------
PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    output.color = gTexture.Sample(gSampler, input.texCoords);
    float2 correct = input.texCoords * (1.0f - input.texCoords.yx);
    
    float vignette = correct.x * correct.y * gVignetteParams.scale;

    vignette = saturate(pow(vignette, gVignetteParams.pow));
    
    output.color.rgb = lerp(gVignetteParams.color.rgb, output.color.rgb, vignette);
    return output;
}
