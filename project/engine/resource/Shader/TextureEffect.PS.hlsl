#include "Object3dTexture.hlsli"

/// =====================================================
// Effect of Using Textures (Combination to Object3dTexture.VS.hlsl)
/// =====================================================

struct PS_Output
{
    float4 color : SV_TARGET;
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
/// Light
struct DirectionalLight
{
    float3 color;
    float intensity;
    float3 direction;
};

struct PointLight
{
    float3 color;
    float intensity;
    float3 pos;
    float radius;
    float decay;
    float padding[2];
};

struct SpotLight
{
    float3 color;
    float intensity;
    float3 pos;
    float distance;
    float3 direction;
    float decay;
    float cosAngle;
    float cosFalloffStart;
    float padding[2];
};

struct LightCounts
{
    int directionalLightCount;
    int pointLightCount;
    int spotLightCount;
};

struct effectParam
{
    float4x4 mask_uv;
    float4x4 dissolve_uv;
    float4x4 distortion_uv;
    float dissolve_Threshold;
    float distortion_Strength;
    
    int effect_Flags;
    
    int padding;
};

// effectFlags 
static const uint EFFECT_NONE = 0;
static const uint EFFECT_DISSOLVE = (1 << 0);
static const uint EFFECT_DISTORTION = (1 << 1);
static const uint EFFECT_MASK = (1 << 2);

/// =====================================================
// Buffers
/// =====================================================

/// Textures
Texture2D<float4> gMainTex : register(t0);
Texture2D<float4> gDissolveTex : register(t1);
Texture2D<float4> gMaskTex : register(t2);
Texture2D<float4> gDistortionTex : register(t3);

SamplerState gSampler : register(s0);

/// Constant Buffers
ConstantBuffer<Material> gMaterial : register(b0);

// b2 = viewProjection from hlsli
StructuredBuffer<DirectionalLight> gDirectionalLight : register(t4);
StructuredBuffer<PointLight> gPointLight : register(t5);
StructuredBuffer<SpotLight> gSpotLight : register(t6);
ConstantBuffer<LightCounts> gLightCounts : register(b7);

ConstantBuffer<effectParam> gEffectParam : register(b8);

PS_Output main(VertexShaderOutput input)
{
    PS_Output output;

    float2 mainUV = mul(float4(input.texCoord, 0.0f, 1.0f), gMaterial.uvTransform).xy;
    
    /// ========================================
    // Effect
    /// ========================================

    float4 dissolveUV = mul(float4(input.texCoord, 0.0f, 1.0f), gEffectParam.dissolve_uv);
    
    float dissolveFactor = gDissolveTex.Sample(gSampler, dissolveUV.xy).r;
    float dissolveMask = step(gEffectParam.dissolve_Threshold, dissolveFactor); // 0 or 1

    float4 maskUV = mul(float4(input.texCoord, 0.0f, 1.0f), gEffectParam.mask_uv);
    float maskValue = gMaskTex.Sample(gSampler, maskUV.xy).r;
    float maskFactor = step(0.5, maskValue); // 0 or 1

// distortion は UV をずらす（Distortion も factor で制御）
    float4 distortionUV = mul(float4(input.texCoord, 0.0f, 1.0f), gEffectParam.distortion_uv);
    float2 distortionOffset = (gDistortionTex.Sample(gSampler, distortionUV.xy).rg - 0.5f) * gEffectParam.distortion_Strength;
    float2 distortedUV = mainUV + distortionOffset;

// 機能マスク（CPU から設定）
    uint effectFlags = gEffectParam.effect_Flags;

// 条件分岐ではなく、重みを掛けることで切り替え
    bool useDissolve = (effectFlags & EFFECT_DISSOLVE) != 0;
    bool useMask = (effectFlags & EFFECT_MASK) != 0;
    bool useDistortion = (effectFlags & EFFECT_DISTORTION) != 0;

    float dissolveEnabled = useDissolve ? 1.0f : 0.0f;
    float maskEnabled = useMask ? 1.0f : 0.0f;
    float distortEnabled = useDistortion ? 1.0f : 0.0f;

// UV
    float2 finalUV = lerp(mainUV, distortedUV, distortEnabled);

// Color Sample
    output.color = gMainTex.Sample(gSampler, finalUV) * gMaterial.color;
    
    // まずはdissolveだけでclip
    clip(dissolveEnabled > 0.5f ? dissolveMask - 0.01f : 1.0f);
    // maskだけ
    clip(maskEnabled > 0.5f ? maskFactor - 0.01f : 1.0f);

    if (!gMaterial.enableLighting)
    {
        return output;
    }
    
    float3 lightingResult = float3(0.f, 0.f, 0.f);
    
    float3 normal = normalize(input.normal);
    float3 viewDir = normalize(gViewProjection.cameraPos - input.worldPos);
    
// Directional Light
    for (int directionalLightIndex = 0; directionalLightIndex < gLightCounts.directionalLightCount; ++directionalLightIndex)
    {
        float NdotL = saturate(dot(normal, -gDirectionalLight[directionalLightIndex].direction));
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
    
        float3 diffuse = output.color.rgb * gDirectionalLight[directionalLightIndex].color.rgb * cos * gDirectionalLight[directionalLightIndex].intensity;
    
        float3 halfVector = normalize(-gDirectionalLight[directionalLightIndex].direction + viewDir);
        float NdotH = dot(normal, halfVector);
        float specularPow = pow(saturate(NdotH), gMaterial.shininess);
    
        float3 specular = gDirectionalLight[directionalLightIndex].color.rgb * gDirectionalLight[directionalLightIndex].intensity * specularPow * gMaterial.specularColor;
    
        lightingResult += diffuse + specular;
    }

// Point Light
    for (int pointLightIndex = 0; pointLightIndex < gLightCounts.pointLightCount; ++pointLightIndex)
    {
        float3 lightDir = normalize(input.worldPos - gPointLight[pointLightIndex].pos);
    
        float distance = length(gPointLight[pointLightIndex].pos - input.worldPos);
        float factElement = saturate(distance / gPointLight[pointLightIndex].radius + 1.0f);
        float factor = pow(factElement, gPointLight[pointLightIndex].decay);
    
        float3 lightColor = gPointLight[pointLightIndex].color.rgb * (gPointLight[pointLightIndex].intensity * factor);
    
        float NdotL = saturate(dot(normal, lightDir));
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
    
        float3 diffuse = output.color.rgb * lightColor * cos;
    
        float3 halfVector = normalize(lightDir + viewDir);
        float NdotH = dot(normal, halfVector);
        float specularPow = pow(saturate(NdotH), gMaterial.shininess);
    
        float3 specular = lightColor * specularPow * gMaterial.specularColor;
    
        lightingResult += diffuse + specular;
    }

// Spot Light
    for (int spotLightIndex = 0; spotLightIndex < gLightCounts.spotLightCount; ++spotLightIndex)
    {
        float3 lightDir = normalize(input.worldPos - gSpotLight[spotLightIndex].pos);
        float distance = length(gSpotLight[spotLightIndex].pos - input.worldPos);
        float attenuationFactor = pow(saturate(1.0f - distance / gSpotLight[spotLightIndex].distance), gSpotLight[spotLightIndex].decay);
        float cosAngle = dot(lightDir, gSpotLight[spotLightIndex].direction);
        float falloffFactor = saturate((cosAngle - gSpotLight[spotLightIndex].cosAngle) / (gSpotLight[spotLightIndex].cosFalloffStart - gSpotLight[spotLightIndex].cosAngle));
        float3 lightColor = gSpotLight[spotLightIndex].color.rgb * gSpotLight[spotLightIndex].intensity * attenuationFactor * falloffFactor;
    
        float NdotL = saturate(dot(normal, lightDir));
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
    
        float3 diffuse = output.color.rgb * lightColor * cos;
    
        float3 halfVector = normalize(lightDir + viewDir);
        float NdotH = dot(normal, halfVector);
        float specularPow = pow(saturate(NdotH), gMaterial.shininess);
    
        float3 specular = lightColor * specularPow * gMaterial.specularColor;
    
        lightingResult += diffuse + specular;
    }

// enableLighting が 0 の場合は textureColor のまま、1 ならライティング結果を使う
    output.color.rgb += lightingResult;
    
    return output;
}
