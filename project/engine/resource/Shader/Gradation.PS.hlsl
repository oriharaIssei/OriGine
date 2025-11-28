#include "FullScreen.hlsli"

struct GradationParam
{
    float2 centerUv; // グラデ中心点
    float2 direction; // Directional のときだけ使用
    float scale; // 変化スピード
    float pow; // ベキ乗
    int colorChannel; // 書き込むチャンネル
    int gradType; // 0 = Directional、1 = Radial
};

///========================================
/// material
struct Material
{
    float4 color;
    float4x4 uvMat;
};

Texture2D<float4> gSceneTexture : register(t0); // input scene texture
SamplerState gSampler : register(s0); // input sampler
ConstantBuffer<GradationParam> gGradationParam : register(b0); // gradation parameters
ConstantBuffer<Material> gMaterial : register(b1); // material parameters

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    float4 transformedUV = mul(float4(input.texCoords, 0.0f, 1.0f), gMaterial.uvMat);

    float2 uv = transformedUV.xy;

    // 基準点を中心に移動
    float2 centeredUV = uv - gGradationParam.centerUv;

    float t = 0.0f;

    // ============================
    //   0: Directional Gradation
    // ============================
    if (gGradationParam.gradType == 0)
    {
        // 方向ベクトル正規化
        float len = length(gGradationParam.direction);
        float2 dirN = (len > 1e-6) ? (gGradationParam.direction / len) : float2(0.0, 1.0);

        // 方向へ射影 → スカラー化
        t = dot(centeredUV, dirN);

        // スケールを適用
        t = t * gGradationParam.scale + 0.5;
    }

    // ============================
    //   1: Radial Gradation
    // ============================
    else if (gGradationParam.gradType == 1)
    {
        // 中心からの距離
        float dist = length(centeredUV);

        // スケールの逆数で変化速度調整（scale が大きいほど変化が速くなる）
        t = dist * gGradationParam.scale;
    }

    // 0..1 に収める
    t = saturate(t);

    // pow 調整
    t = pow(t, max(gGradationParam.pow, 0.00001));

    // 既存カラー
    float4 inputColor = gSceneTexture.Sample(gSampler, uv) * gMaterial.color;

    // 指定チャンネルにグラデ値を書き込み
    inputColor[gGradationParam.colorChannel] = t;

    output.color = inputColor;
    return output;
}
