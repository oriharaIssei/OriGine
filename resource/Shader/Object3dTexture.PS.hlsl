#include "Object3dTexture.hlsli"

struct Material {
    float4 color;
    int enableLighting;
    float4x4 uvTransform;
    float shininess;
    float3 specularColor;
};

struct PixelShaderOutput {
    float4 color : SV_TARGET0;
};

///========================================
/// Light
struct DirectionalLight {
    float3 color;
    float intensity;
    float3 direction;
};

struct PointLight {
    float3 color;
    float intensity;
    float3 pos;
    float radius;
    float decay;
    float3 padding;
};

struct SpotLight {
    float3 color;
    float3 pos;
    float intensity;
    float3 direction;
    float distance;
    float decay;
    float cosAngle;
    float cosFalloffStart;
};

struct LightCounts {
    int directionalLightCount;
    int pointLightCount;
    int spotLightCount;
};

/// Light
///========================================

///========================================
/// ConstantBuffer
///========================================
ConstantBuffer<Material> gMaterial : register(b0);

StructuredBuffer<DirectionalLight> gDirectionalLight : register(t1);
// b2 = viewProjection from hlsli
StructuredBuffer<PointLight> gPointLight : register(t3);
StructuredBuffer<SpotLight> gSpotLight : register(t4);

ConstantBuffer<LightCounts> gLightCounts : register(b5);

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

///=============================================================
/// Lighting Functions
///=============================================================
float3 LambertDiffuse(float3 normal,float3 lightDir,float3 lightColor,float lightIntensity) {
    float NdotL = saturate(dot(normal,lightDir));
    return lightColor * lightIntensity * NdotL;
}

float3 PhongSpecular(
    float3 normal,
    float3 lightDir,
    float3 viewDir,
    float shininess,
    float3 lightColor,
    float lightIntensity
) {
    float3 reflectDir = reflect(-lightDir,normal);
    float RdotV = saturate(dot(reflectDir,viewDir));
    float specularFactor = pow(RdotV,shininess);
    return lightColor * lightIntensity * specularFactor;
}

float3 BlinnPhongSpecular(
    float3 normal,
    float3 lightDir,
    float3 viewDir,
    float shininess,
    float3 lightColor,
    float lightIntensity
) {
    float3 halfVec = normalize(lightDir + viewDir);
    float NdotH = saturate(dot(normal,halfVec));
    float specularFactor = pow(NdotH,shininess);
    return lightColor * lightIntensity * specularFactor;
}

PixelShaderOutput main(VertexShaderOutput input) {
    PixelShaderOutput output;

    // texcoord を z=0 の (3+1)次元 として考える
    float4 transformedUV = mul(float4(input.texCoord,0.0f,1.0f),gMaterial.uvTransform);
    float4 textureColor = gMaterial.color * gTexture.Sample(gSampler,transformedUV.xy);
    
    
    if (textureColor.a <= 0.1f) {
        discard;
    }
    
    output.color = textureColor;
    switch (gMaterial.enableLighting) {
        case 1:{ // Half Lambert
                float3 normal = normalize(input.normal);
                float3 viewDir = normalize(gViewProjection.cameraPos - input.worldPos);
                output.color.rgb = float3(0.0f,0.0f,0.0f);
        
        // Directional Light
                for (uint directionalLightIndex = 0 ; directionalLightIndex < gLightCounts.directionalLightCount ; ++directionalLightIndex) {
                    float3 dirLightColor = LambertDiffuse(normal,-gDirectionalLight[directionalLightIndex].direction,gDirectionalLight[directionalLightIndex].color,gDirectionalLight[directionalLightIndex].intensity);
                    output.color.rgb += textureColor.rgb * dirLightColor;
                }

        // Point Light
                for (uint pointLightIndex = 0 ; pointLightIndex < gLightCounts.pointLightCount ; ++pointLightIndex) {
                    float3 lightDir = normalize(gPointLight[pointLightIndex].pos - input.worldPos);
                    float distance = length(gPointLight[pointLightIndex].pos - input.worldPos);
                    float attenuation = pow(saturate(-distance / gPointLight[pointLightIndex].radius + 1.0f),gPointLight[pointLightIndex].decay);
                    float3 lightColor = LambertDiffuse(normal,lightDir,gPointLight[pointLightIndex].color,gPointLight[pointLightIndex].intensity * attenuation);
                    output.color.rgb += textureColor.rgb * lightColor;
                }

        // Spot Light
                for (uint spotLightIndex = 0 ; spotLightIndex < gLightCounts.spotLightCount ; ++spotLightIndex) {
                    float3 lightDir = normalize(input.worldPos - gSpotLight[spotLightIndex].pos);
                    float distance = length(gSpotLight[spotLightIndex].pos - input.worldPos);
                    float attenuation = pow(saturate(1.0f - distance / gSpotLight[spotLightIndex].distance),gSpotLight[spotLightIndex].decay);
                    float cosAngle = dot(lightDir,normalize(gSpotLight[spotLightIndex].direction));
                    float falloff = saturate(( cosAngle - gSpotLight[spotLightIndex].cosAngle ) / ( gSpotLight[spotLightIndex].cosFalloffStart - gSpotLight[spotLightIndex].cosAngle ));
                    float3 lightColor = LambertDiffuse(normal,lightDir,gSpotLight[spotLightIndex].color,gSpotLight[spotLightIndex].intensity * attenuation * falloff);
                    output.color.rgb += textureColor.rgb * lightColor;
                }
                break;
            }

        case 2:{ // Lambert
                float3 normal = normalize(input.normal);
                float3 viewDir = normalize(gViewProjection.cameraPos - input.worldPos);
                output.color.rgb = float3(0.0f,0.0f,0.0f);

        // Directional Light
                for (uint directionalLightIndex = 0 ; directionalLightIndex < gLightCounts.directionalLightCount ; ++directionalLightIndex) {
                    float3 dirLightColor = LambertDiffuse(normal,-gDirectionalLight[directionalLightIndex].direction,gDirectionalLight[directionalLightIndex].color,gDirectionalLight[directionalLightIndex].intensity);
                    output.color.rgb += textureColor.rgb * dirLightColor;
                }

        // Point Light
                for (uint pointLightIndex = 0 ; pointLightIndex < gLightCounts.pointLightCount ; ++pointLightIndex) {
                    float3 lightDir = normalize(gPointLight[pointLightIndex].pos - input.worldPos);
                    float distance = length(gPointLight[pointLightIndex].pos - input.worldPos);
                    float attenuation = pow(saturate(1.0f - distance / gPointLight[pointLightIndex].radius),gPointLight[pointLightIndex].decay);
                    float3 lightColor = LambertDiffuse(normal,lightDir,gPointLight[pointLightIndex].color,gPointLight[pointLightIndex].intensity * attenuation);
                    output.color.rgb += textureColor.rgb * lightColor;
                }

        // Spot Light
                for (uint spotLightIndex = 0 ; spotLightIndex < gLightCounts.spotLightCount ; ++spotLightIndex) {
                    float3 lightDir = normalize(input.worldPos - gSpotLight[spotLightIndex].pos);
                    float distance = length(gSpotLight[spotLightIndex].pos - input.worldPos);
                    float attenuation = pow(saturate(-distance / gPointLight[pointLightIndex].radius + 1.0f),gPointLight[pointLightIndex].decay);
                    float cosAngle = dot(lightDir,normalize(gSpotLight[spotLightIndex].direction));
                    float falloff = saturate(( cosAngle - gSpotLight[spotLightIndex].cosAngle ) / ( gSpotLight[spotLightIndex].cosFalloffStart - gSpotLight[spotLightIndex].cosAngle ));
                    float3 lightColor = LambertDiffuse(normal,lightDir,gSpotLight[spotLightIndex].color,gSpotLight[spotLightIndex].intensity * attenuation * falloff);
                    output.color.rgb += textureColor.rgb * lightColor;
                }
                break;
            }

        case 3:{ // Phong
                float3 normal = normalize(input.normal);
                float3 viewDir = normalize(gViewProjection.cameraPos - input.worldPos);
                output.color.rgb = float3(0.0f,0.0f,0.0f);

        // Directional Light
                for (uint directionalLightIndex = 0 ; directionalLightIndex < gLightCounts.directionalLightCount ; ++directionalLightIndex) {
                    float3 diffuse = LambertDiffuse(normal,-gDirectionalLight[directionalLightIndex].direction,gDirectionalLight[directionalLightIndex].color,gDirectionalLight[directionalLightIndex].intensity);
                    float3 specular = PhongSpecular(normal,-gDirectionalLight[directionalLightIndex].direction,viewDir,gMaterial.shininess,gDirectionalLight[directionalLightIndex].color,gDirectionalLight[directionalLightIndex].intensity);
                    output.color.rgb += textureColor.rgb * ( diffuse + specular * ( 1 - diffuse ) );
                }

        // Point Light
                for (uint pointLightIndex = 0 ; pointLightIndex < gLightCounts.pointLightCount ; ++pointLightIndex) {
                    float3 lightDir = normalize(gPointLight[pointLightIndex].pos - input.worldPos);
                    float distance = length(gPointLight[pointLightIndex].pos - input.worldPos);
                    float attenuation = pow(saturate(-distance / gPointLight[pointLightIndex].radius + 1.0f),gPointLight[pointLightIndex].decay);
                    float3 diffuse = LambertDiffuse(normal,lightDir,gPointLight[pointLightIndex].color,gPointLight[pointLightIndex].intensity * attenuation);
                    float3 specular = PhongSpecular(normal,lightDir,viewDir,gMaterial.shininess,gPointLight[pointLightIndex].color,gPointLight[pointLightIndex].intensity * attenuation);
                    output.color.rgb += textureColor.rgb * ( diffuse + specular * ( 1 - diffuse ) );
                }
            // Spot Light
                for (uint spotLightIndex = 0 ; spotLightIndex < gLightCounts.spotLightCount ; ++spotLightIndex) {
                    float3 lightDir = normalize(input.worldPos - gSpotLight[spotLightIndex].pos);
                    float distance = length(gSpotLight[spotLightIndex].pos - input.worldPos);
                    float attenuation = pow(saturate(1.0f - distance / gSpotLight[spotLightIndex].distance),gSpotLight[spotLightIndex].decay);
                    float cosAngle = dot(lightDir,normalize(gSpotLight[spotLightIndex].direction));
                    float falloff = saturate(( cosAngle - gSpotLight[spotLightIndex].cosAngle ) / ( gSpotLight[spotLightIndex].cosFalloffStart - gSpotLight[spotLightIndex].cosAngle ));
                    float3 diffuse = LambertDiffuse(normal,lightDir,gSpotLight[spotLightIndex].color,gSpotLight[spotLightIndex].intensity * attenuation * falloff);
                    float3 specular = PhongSpecular(normal,lightDir,viewDir,gMaterial.shininess,gSpotLight[spotLightIndex].color,gSpotLight[spotLightIndex].intensity * attenuation * falloff);
                    output.color.rgb += textureColor.rgb * ( diffuse + specular * ( 1 - diffuse ) );
                }
                break;
            }

        case 4:{ // Blinn-Phong
                float3 normal = normalize(input.normal);
                float3 viewDir = normalize(gViewProjection.cameraPos - input.worldPos);
                output.color.rgb = float3(0.0f,0.0f,0.0f);

        // Directional Light
                for (uint directionalLightIndex = 0 ; directionalLightIndex < gLightCounts.directionalLightCount ; ++directionalLightIndex) {
                    float3 diffuse = LambertDiffuse(normal,-gDirectionalLight[directionalLightIndex].direction,gDirectionalLight[directionalLightIndex].color,gDirectionalLight[directionalLightIndex].intensity);
                    float3 specular = BlinnPhongSpecular(normal,-gDirectionalLight[directionalLightIndex].direction,viewDir,gMaterial.shininess,gDirectionalLight[directionalLightIndex].color,gDirectionalLight[directionalLightIndex].intensity);
                    output.color.rgb += textureColor.rgb * ( diffuse + specular * ( 1 - diffuse ) );
                }

        // Point Light
                for (uint pointLightIndex = 0 ; pointLightIndex < gLightCounts.pointLightCount ; ++pointLightIndex) {
                    float3 lightDir = normalize(gPointLight[pointLightIndex].pos - input.worldPos);
                    float distance = length(gPointLight[pointLightIndex].pos - input.worldPos);
                    float attenuation = pow(saturate(-distance / gPointLight[pointLightIndex].radius + 1.0f),gPointLight[pointLightIndex].decay);
                    float3 diffuse = LambertDiffuse(normal,lightDir,gPointLight[pointLightIndex].color,gPointLight[pointLightIndex].intensity * attenuation);
                    float3 specular = BlinnPhongSpecular(normal,lightDir,viewDir,gMaterial.shininess,gPointLight[pointLightIndex].color,gPointLight[pointLightIndex].intensity * attenuation);
                    output.color.rgb += textureColor.rgb * ( diffuse + specular * ( 1 - diffuse ) );
                }
            
            // Spot Light
                for (uint spotLightIndex = 0 ; spotLightIndex < gLightCounts.spotLightCount ; ++spotLightIndex) {
                    float3 lightDir = normalize(input.worldPos - gSpotLight[spotLightIndex].pos);
                    float distance = length(gSpotLight[spotLightIndex].pos - input.worldPos);
                    float attenuation = pow(saturate(1.0f - distance / gSpotLight[spotLightIndex].distance),gSpotLight[spotLightIndex].decay);
                    float cosAngle = dot(lightDir,normalize(gSpotLight[spotLightIndex].direction));
                    float falloff = saturate(( cosAngle - gSpotLight[spotLightIndex].cosAngle ) / ( gSpotLight[spotLightIndex].cosFalloffStart - gSpotLight[spotLightIndex].cosAngle ));
                    float3 diffuse = LambertDiffuse(normal,lightDir,gSpotLight[spotLightIndex].color,gSpotLight[spotLightIndex].intensity * attenuation * falloff);
                    float3 specular = BlinnPhongSpecular(normal,lightDir,viewDir,gMaterial.shininess,gSpotLight[spotLightIndex].color,gSpotLight[spotLightIndex].intensity * attenuation * falloff);
                    output.color.rgb += textureColor.rgb * ( diffuse + specular * ( 1 - diffuse ) );
                }
                break;
            }
    }

    return output;
}