#include "Object3dTextureColor.hlsli"

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

///========================================
/// material
struct Material
{
    float4 color;
    int enableLighting;
    float4x4 uvTransform;
    float shininess;
    float3 specularColor;
    float environmentCoefficient;
};
///========================================

///========================================
/// ConstantBuffer
///========================================
ConstantBuffer<Material> gMaterial : register(b0);

Texture2D<float4> gTexture : register(t0);
TextureCube<float4> gEnvironmentTexture : register(t1);
SamplerState gSampler : register(s0);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    // texcoord を z=0 の (3+1)次元 として考える
    float4 transformedUV = mul(float4(input.texCoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gMaterial.color * gTexture.Sample(gSampler, transformedUV.xy);
    
    if (textureColor.a <= 0.0f)
    {
        discard;
    }
    
    output.color = textureColor * input.color;
    
    return output;
}
