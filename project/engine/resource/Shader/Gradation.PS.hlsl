#include "FullScreen.hlsli"

struct GradationParam
{
    int inputColorChannel; // 0b0:red, 0b1:green, 0b2:blue, 0b3:alpha
    int outputColorChannel; // 0b0:red, 0b1:green, 0b2:blue, 0b3:alpha
};

///========================================
/// material
struct Material
{
    float4 color;
    float4x4 uvMat;
};

Texture2D<float4> gInputGradation : register(t0); // input gradation texture
Texture2D<float4> gOutputTexture : register(t1); // output texture
SamplerState gSampler : register(s0); // input sampler
ConstantBuffer<GradationParam> gGradationParam : register(b0); // gradation parameters
ConstantBuffer<Material> gMaterial : register(b1); // material parameters

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    float4 transformedUV = mul(float4(input.texCoords, 0.0f, 1.0f), gMaterial.uvMat);

    float4 inputColor = gInputGradation.Sample(gSampler, transformedUV.xy) * gMaterial.color;
    float4 outputColor = gOutputTexture.Sample(gSampler, input.texCoords);
   
    // 入力チャンネル値を取得
    float inputValue = inputColor[gGradationParam.inputColorChannel];

    // 出力チャンネルを書き換え
    outputColor[gGradationParam.outputColorChannel] = inputValue;
    
    output.color = outputColor;
    
    return output;
}
