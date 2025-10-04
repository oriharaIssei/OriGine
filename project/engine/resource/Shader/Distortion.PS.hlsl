#include "FullScreen.hlsli"

///=============================================================================
// Distortion
///=============================================================================
struct DistortionEffectParam
{
    float distortion_Bias; // UV のバイアス
    float distortion_Strength; // UV の強度
};

struct Material
{
    float4x4 uvMat;
};


/// ------------------------------------------------------------------
// buffers
/// ------------------------------------------------------------------
Texture2D<float4> gDistortionTexture : register(t0);
Texture2D<float4> gSceneViewTexture : register(t1);
SamplerState gSampler : register(s0);
ConstantBuffer<DistortionEffectParam> gEffectParam : register(b0);
ConstantBuffer<Material> gMaterial : register(b1);

/// ------------------------------------------------------------------
// main
/// ------------------------------------------------------------------
PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float2 distortedUV = input.texCoords;
    
    // distortion は UV をずらす（Distortion も factor で制御）
    float4 distortionUV = mul(float4(input.texCoords, 0.0f, 1.0f), gMaterial.uvMat);
    float4 distortionTexColor = gDistortionTexture.Sample(gSampler, distortionUV.xy);
    
    if (distortionTexColor.a > 0.1f)
    {
        float2 distortionOffset = (distortionTexColor.rg - gEffectParam.distortion_Bias) * gEffectParam.distortion_Strength;
        distortedUV = input.texCoords + distortionOffset;
    }

    output.color = gSceneViewTexture.Sample(gSampler, distortedUV.xy);

    return output;
}
