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

static const float PI = 3.14159265;
float gauss(float x, float y, float sigma)
{
    float exponent = -(x * x + y * y) / (2 * sigma * sigma);
    float denominator = 2.f * PI * sigma * sigma;
    return exp(exponent) * rcp(denominator);
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
    
    float kernelSum = 0.f;
    
    for (int x = 0; x < (int) gFilterBoxSize.size.x; ++x)
    {
        for (int y = 0; y < (int) gFilterBoxSize.size.y; ++y)
        {
            float2 uvOffset = float2(x - gFilterBoxSize.size.x * 0.5f, y - gFilterBoxSize.size.y * 0.5f) * uvStepSize;
            float weight = gauss(uvOffset.x, uvOffset.y, 2.f);
            kernelSum += weight;

            float2 uv = input.texCoords + uvOffset;
            float3 fetchColor = gTexture.Sample(gSampler, uv).rgb;
            output.color.rgb += fetchColor * weight;
        }
    }
    output.color.rgb /= kernelSum;

    return output;
}
