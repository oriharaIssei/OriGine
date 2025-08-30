#include "FullScreen.hlsli"

///=============================================================================
// Speedline
///=============================================================================

struct LineEffectParam
{
    float2 screenCenterUV; // 集中線の中心
    float intensity; // 強度
    float density; // 密度（UVスケール）
    float4 color; // 線の色
    float time; // 時間
    float fadeStart; // フェード開始距離 (例: 0.2)
    float fadePow; // フェード強さ (例: 2.0)
};

/// ------------------------------------------------------------------
// buffers
/// ------------------------------------------------------------------

Texture2D<float4> gSceneTexture : register(t0); // シーン
Texture2D<float4> gRadialTexture : register(t1); // ラジアル
SamplerState gSampler : register(s0);

ConstantBuffer<LineEffectParam> gLineEffectParam : register(b0);

/// ------------------------------------------------------------------
// main
/// ------------------------------------------------------------------
PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    // ================================
    // 1. シーンカラー取得
    // ================================
    float4 sceneColor = gSceneTexture.Sample(gSampler, input.texCoords);

    // ================================
    // 2. ラジアル線エフェクト
    // ================================
    // 中心からのベクトル
    float2 dir = input.texCoords - gLineEffectParam.screenCenterUV;
    float dist = length(dir);
    float fadeMask = step(gLineEffectParam.fadeStart, dist);
    float fade = pow(saturate((dist - gLineEffectParam.fadeStart) / (1.0 - gLineEffectParam.fadeStart)), gLineEffectParam.fadePow);

    // (r, θ) をUVに変換してラジアルテクスチャをサンプリング
    float2 radialUV;
    radialUV.x = dist * gLineEffectParam.density;
    radialUV.y = atan2(dir.y, dir.x) / (2.0 * 3.14159265);
    radialUV.y = frac(radialUV.y + 0.5); // [0,1]に正規化

    // 時間で流れる効果
    radialUV.x += gLineEffectParam.time;

    float4 radialSample = gRadialTexture.Sample(gSampler, radialUV);

    // 強度・カラー適用
    float3 radialColor = radialSample.rgb * gLineEffectParam.color.rgb * gLineEffectParam.intensity * fade;
    float radialAlpha = radialSample.a * gLineEffectParam.color.a * gLineEffectParam.intensity * fade;

    // ================================
    // 3. 合成 アルファブレンド
    // ================================
    output.color.rgb = lerp(sceneColor.rgb, radialColor, radialAlpha);
    output.color.a = 1.f;

    return output;
}
