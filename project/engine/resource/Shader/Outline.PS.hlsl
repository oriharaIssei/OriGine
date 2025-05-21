#include "FullScreen.hlsli"

/// =====================================================
//  Outline
/// =====================================================

struct OutlineParam
{
    float4 color;
    float4x4 projectionInverse;
};

static const float3x3 kPrewttHorizontalKernel =
{
    -1.f / 6.f, 0.f, 1.f / 6.f,
    -1.f / 6.f, 0.f, 1.f / 6.f,
    -1.f / 6.f, 0.f, 1.f / 6.f
};

static const float3x3 kPrewttVerticalKernel =
{
    -1.f / 6.f, -1.f / 6.f, -1.f / 6.f,
    0.f, 0.f, 0.f,
    1.f / 6.f, 1.f / 6.f, 1.f / 6.f
};

static const float2 kIndex3x3[3][3] =
{
    { { -1.0f, -1.0f }, { 0.0f, -1.0f }, { 1.0f, -1.0f } },
    { { -1.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f } },
    { { -1.0f, 1.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f } }
};

float3 Luminance(float3 v)
{
    return dot(v, float3(0.2125f, 0.7154f, 0.0721f));
}


/// ---------------------------------------
// Buffers
/// ---------------------------------------
Texture2D<float4> gInputTexture : register(t0);
SamplerState gInputSampler : register(s0);
Texture2D<float> gDepthTexture : register(t1);
SamplerState gDepthSampler : register(s1);

ConstantBuffer<OutlineParam> gOutlineParam : register(b0);


PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    float2 diff = float2(0.0f, 0.0f);
    
    uint width, height;
    gInputTexture.GetDimensions(width, height);
    float2 uvStepSize = float2(rcp(width), rcp(height));
    
    for (int x = 0; x < 3; ++x)
    {
        for (int y = 0; y < 3; ++y)
        {
            float2 texCoord = input.texCoords + uvStepSize * kIndex3x3[x][y];

            float ndcDepth = gDepthTexture.Sample(gDepthSampler, texCoord);
            float4 viewSpace = mul(float4(0.f, 0.f, ndcDepth, 1.0f), gOutlineParam.projectionInverse);
            float4 viewZ = viewSpace.z * rcp(viewSpace.w);

            diff.x += viewZ * kPrewttHorizontalKernel[x][y];
            diff.y += viewZ * kPrewttVerticalKernel[x][y];
        }
    }

    float weight = length(diff);

    output.color.rgb = (1.f - weight) * gInputTexture.Sample(gInputSampler, input.texCoords).rgb;
    output.color.a = 1.f;
    
    return output;
}
