#include "FullScreen.hlsli"

struct DissolveParam
{
    float threshold;
    float edgeWidth;

    float2 pad;
    
    float4 outLineColor;

    float4x4 uvMat;
};

Texture2D<float4> gSceneTexture : register(t0); // input scene texture
Texture2D<float4> gDissolveTexture : register(t1); // dissolve texture
SamplerState gSampler : register(s0); // input sampler
ConstantBuffer<DissolveParam> gDissolveParam : register(b0); // dissolve parameters

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 transformedUV = mul(float4(input.texCoords, 0.0f, 1.0f), gDissolveParam.uvMat);
    float dissolveValue = gDissolveTexture.Sample(gSampler, transformedUV.xy).r;

    if (dissolveValue < gDissolveParam.threshold)
    {
        discard;
    }
    
        // Apply outline effect
    float edgeFactor = smoothstep(gDissolveParam.threshold,
                                      gDissolveParam.threshold + gDissolveParam.edgeWidth,
                                      dissolveValue);
    
    output.color = lerp(gDissolveParam.outLineColor, gSceneTexture.Sample(gSampler, input.texCoords), edgeFactor);

    return output;
}
