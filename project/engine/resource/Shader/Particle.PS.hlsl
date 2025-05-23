#include "Particle.hlsli"

struct Material
{
    float4 color;
    int enableLighting;
    float4x4 uvTransform;
};

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

///========================================
/// ConstantBufferの定義
///========================================

ConstantBuffer<Material> gMaterial : register(b0);

Texture2D<float4> gTexture : register(t0); // SRVの registerは t
SamplerState gSampler : register(s0); // textureを読むためのもの. texture のサンプリングを担当

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    // texcoord を z=0 の (3+1)次元 として考える
    output.color = input.color * gTexture.Sample(gSampler, input.texCoord.xy);
   
    if (output.color.a <= 0.1f)
    {
        discard;
    }

    return output;
}
