#include "FullScreen.hlsli"

///=============================================================================
// Vignette
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
    
    output.color = gTexture.Sample(gSampler, input.texCoords);
    
    float2 correct = input.texCoords * (-1.0f - input.texCoords.yx);
    
    float vignette = correct.x * correct.y * 16.0f;

    vignette = saturate(pow(vignette, 0.8f));

    output.color.rgb *= vignette;
    return output;
}
