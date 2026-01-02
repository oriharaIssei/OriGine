#include "Object3dTextureColor.hlsli"
#include "ShadowUtility.hlsli"

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
/// Light
struct DirectionalLight
{
    float3 color;
    float intensity;
    float3 direction;
    float angularRadius;
};

struct PointLight
{
    float3 color;
    float intensity;
    float3 pos;
    float radius;
    float decay;
    float angularRadius;
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
    float angularRadius;
    float padding[1];
};

struct LightCounts
{
    int directionalLightCount;
    int pointLightCount;
    int spotLightCount;
};

///========================================
/// ConstantBuffer
///========================================
ConstantBuffer<Material> gMaterial : register(b0);

Texture2D<float4> gTexture : register(t0);
TextureCube<float4> gEnvironmentTexture : register(t1);
SamplerState gSampler : register(s0);

// b2 = viewProjection from hlsli
StructuredBuffer<DirectionalLight> gDirectionalLight : register(t2);
StructuredBuffer<PointLight> gPointLight : register(t3);
StructuredBuffer<SpotLight> gSpotLight : register(t4);

ConstantBuffer<LightCounts> gLightCounts : register(b5);

///=============================================================
/// Lighting Functions
///=============================================================
float3 LambertDiffuse(float3 normal, float3 lightDir, float3 lightColor, float lightIntensity)
{
    float NdotL = saturate(dot(normal, lightDir));
    return lightColor * lightIntensity * NdotL;
}

float3 PhongSpecular(
    float3 normal,
    float3 lightDir,
    float3 viewDir,
    float shininess,
    float3 lightColor,
    float lightIntensity
)
{
    float3 reflectDir = reflect(-lightDir, normal);
    float RdotV = saturate(dot(reflectDir, viewDir));
    float specularFactor = pow(RdotV, shininess);
    return lightColor * lightIntensity * specularFactor;
}

float3 BlinnPhongSpecular(
    float3 normal,
    float3 lightDir,
    float3 viewDir,
    float shininess,
    float3 lightColor,
    float lightIntensity
)
{
    float3 halfVec = normalize(lightDir + viewDir);
    float NdotH = saturate(dot(normal, halfVec));
    float specularFactor = pow(NdotH, shininess);
    return lightColor * lightIntensity * specularFactor;
}

static const float SHADOW_RAY_EPSILON = 0.01f;

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
    
    float3 objectColor = textureColor.rgb * input.color.rbg;
    
    // ToDo
    // if の削除
    if (gMaterial.enableLighting == 0)
    {
        output.color = float4(objectColor, textureColor.a * input.color.a);
        return output; // skip
    }
    
    float3 normal = normalize(input.normal);
    float3 viewDir = normalize(gViewProjection.cameraPos - input.worldPos);
  
    float3 lightResult = float3(0.0f, 0.0f, 0.0f);
    float cosThetaMax = 0.f;
        
    // Directional Light
    for (uint directionalLightIndex = 0;
     directionalLightIndex < gLightCounts.directionalLightCount;
     ++directionalLightIndex)
    {
        float3 lightDir = -gDirectionalLight[directionalLightIndex].direction;
        
        float NdotL = saturate(dot(normal, lightDir));
        float cosAngle = pow(NdotL * 0.5f + 0.5f, 2.0f);

        float3 diffuse =
            objectColor *
            gDirectionalLight[directionalLightIndex].color.rgb *
            cosAngle *
            gDirectionalLight[directionalLightIndex].intensity;

        float3 halfVector = normalize(lightDir + viewDir);
        float NdotH = dot(normal, halfVector);
        float specularPow = pow(saturate(NdotH), gMaterial.shininess);

        float3 specular =
            gDirectionalLight[directionalLightIndex].color.rgb *
            gDirectionalLight[directionalLightIndex].intensity *
            specularPow *
            gMaterial.specularColor;

        float3 direcResult = diffuse + specular;

        cosThetaMax = cos(gDirectionalLight[directionalLightIndex].angularRadius);
        float shadow = TraceShadowSoft(
            input.worldPos + normal * SHADOW_RAY_EPSILON,
            lightDir,
            1e5f,
            cosThetaMax,
            4);
        
        
        lightResult += direcResult * shadow;
    }

    // Point Light
    for (uint pointLightIndex = 0;
     pointLightIndex < gLightCounts.pointLightCount;
     ++pointLightIndex)
    {
        float3 lightVec = gPointLight[pointLightIndex].pos - input.worldPos;
        float distance = length(lightVec);
        float3 lightDir = lightVec / distance;

        float factor = pow(saturate(1.0f - distance / gPointLight[pointLightIndex].radius), gPointLight[pointLightIndex].decay);
        
        float3 lightColor = gPointLight[pointLightIndex].color.rgb * (gPointLight[pointLightIndex].intensity * factor);
        
        float NdotL = saturate(dot(normal, lightDir));
        float cosAngle = pow(NdotL * 0.5f + 0.5f, 2.0f);
        
        float3 diffuse = objectColor.rgb * lightColor * cosAngle;
        
        float3 halfVector = normalize(-lightDir + viewDir);
        float NdotH = dot(normal, halfVector);
        float specularPow = pow(saturate(NdotH), gMaterial.shininess);
        
        float3 specular = lightColor * specularPow * gMaterial.specularColor;
        
        float3 direcResult = diffuse + specular;
        
        cosThetaMax = cos(gPointLight[pointLightIndex].angularRadius);
        
        float shadow = TraceShadowSoft(
        input.worldPos + normal * SHADOW_RAY_EPSILON,
        lightDir,
        distance,
        cosThetaMax,
        2);

        direcResult *= shadow;
        
        lightResult += direcResult;
    }

    // Spot Light
    for (uint spotLightIndex = 0;
     spotLightIndex < gLightCounts.spotLightCount;
     ++spotLightIndex)
    {
        float3 lightDir = normalize(gSpotLight[spotLightIndex].pos - input.worldPos);
        float distance = length(gSpotLight[spotLightIndex].pos - input.worldPos);

        float cosAngle = dot(lightDir, gSpotLight[spotLightIndex].direction);

        float falloff = saturate((cosAngle - gSpotLight[spotLightIndex].cosAngle) / (gSpotLight[spotLightIndex].cosFalloffStart - gSpotLight[spotLightIndex].cosAngle));
        float attenuationFactor = pow(saturate(1.0f - distance / gSpotLight[spotLightIndex].distance), gSpotLight[spotLightIndex].decay);
        float falloffFactor = saturate((cosAngle - gSpotLight[spotLightIndex].cosAngle) / (gSpotLight[spotLightIndex].cosFalloffStart - gSpotLight[spotLightIndex].cosAngle));
        float3 lightColor = gSpotLight[spotLightIndex].color.rgb * gSpotLight[spotLightIndex].intensity * attenuationFactor * falloffFactor;
        
        float NdotL = saturate(dot(normal, lightDir));
        float cosN = pow(NdotL * 0.5f + 0.5f, 2.0f);
        
        float3 diffuse = objectColor.rgb * lightColor * cosN;
        
        float3 halfVector = normalize(-lightDir + viewDir);
        float NdotH = dot(normal, halfVector);
        float specularPow = pow(saturate(NdotH), gMaterial.shininess);
        
        float3 specular = lightColor * specularPow * gMaterial.specularColor;
        
        float3 direcResult = diffuse + specular;
        
        cosThetaMax = cos(gSpotLight[spotLightIndex].angularRadius);
        
        float shadow = TraceShadowSoft(
        input.worldPos + normal * SHADOW_RAY_EPSILON,
        lightDir,
        distance,
        cosThetaMax,
        2);

        direcResult *= shadow;

        lightResult += direcResult;
    }
    
    float3 camera2Pos = normalize(input.worldPos - gViewProjection.cameraPos);
    float3 reflectedVec = reflect(camera2Pos, normal);
    float4 environmentColor = gEnvironmentTexture.Sample(gSampler, reflectedVec);

    output.color.rgb = lightResult;
    output.color.rgb += environmentColor.rgb * gMaterial.environmentCoefficient;
    output.color.a = textureColor.a * input.color.a;
    
    return output;
}
