#include "Sprite.hlsli"

Texture2D<float4> tex : register(t0);
SamplerState smp : register(s0);

float4 main(VSOutput input) : SV_TARGET {
    float4 transformedUV = mul(float4(input.uv,0.0f,1.0f),uvMat);
    return tex.Sample(smp,transformedUV.xy) * color;
}