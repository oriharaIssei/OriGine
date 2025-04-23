#include "FullScreen.hlsli"

///=============================================================================
// Smoothing / BoxFilter,BoxBlur
///=============================================================================

/// ------------------------------------------------------------------
// buffers
/// ------------------------------------------------------------------
Texture2D<float4> gTexture : register(t0); // input texture
SamplerState gSampler : register(s0); // input sampler

struct FilterBoxSize
{
    float2 size;
};
ConstantBuffer<FilterBoxSize> gFilterBoxSize : register(b0);

float CalculateAverageKernel(float2 boxSize)
{
    float kernel = 1.f / (boxSize.x * boxSize.y);

    return kernel;
}

/// ------------------------------------------------------------------
// main
/// ------------------------------------------------------------------
PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    uint width, height;
    gTexture.GetDimensions(width, height);
    float2 uvStepSize = float2(rcp(width), rcp(height));

    output.color = float4(0, 0, 0, 1.f);
    
    float2 boxCenter = gFilterBoxSize.size * 0.5f;
    float kernel = CalculateAverageKernel(gFilterBoxSize.size);
    
    for (int x = 0; x < gFilterBoxSize.size.x; ++x)
    {
        for (int y = 0; y < gFilterBoxSize.size.y; ++y)
        {
            float2 uvOffset = float2(x - boxCenter.x, y - boxCenter.y) * uvStepSize;
            float2 uv = input.texCoords + uvOffset;
            
            float3 fetchColor = gTexture.Sample(gSampler, uv).rgb;
            output.color.rgb += fetchColor * kernel;
        }
    }

    return output;
}
