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
    float padding[2];
};

struct SpotLight {
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

// b2 = viewProjection from hlsli
StructuredBuffer<DirectionalLight> gDirectionalLight : register(t1);
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
    
    // ToDo
    // if の削除
    if (gMaterial.enableLighting == 0) {
        output.color = textureColor;
        return output; // skip
    }
    
    float3 normal = normalize(input.normal);
    float3 viewDir = normalize(gViewProjection.cameraPos - input.worldPos);
    // Half Lambert & phong
    output.color.rgb = float3(0.0f,0.0f,0.0f);
        
        // Directional Light
    for (uint directionalLightIndex = 0 ; directionalLightIndex < gLightCounts.directionalLightCount ; ++directionalLightIndex) {
        float NdotL = saturate(dot(normal,-gDirectionalLight[directionalLightIndex].direction));
        float cos = pow(NdotL * 0.5f + 0.5f,2.0f);
        
        float3 diffuse = textureColor.rgb * gDirectionalLight[directionalLightIndex].color.rgb * cos * gDirectionalLight[directionalLightIndex].intensity;
        
        //float reflectLight = reflect(gDirectionalLight[directionalLightIndex].direction,normal);
        //float RdotE = dot(reflectLight,viewDir);
        //float specularPow = pow(saturate(RdotE),gMaterial.shininess);
        
        float3 halfVector = normalize(-gDirectionalLight[directionalLightIndex].direction + viewDir);
        float NdotH = dot(normal,halfVector);
        float specularPow = pow(saturate(NdotH),gMaterial.shininess);
        
        float3 specular = gDirectionalLight[directionalLightIndex].color.rgb * gDirectionalLight[directionalLightIndex].intensity * specularPow * gMaterial.specularColor;
        
        output.color.rgb += diffuse + specular;
    }

        // Point Light
    for (uint pointLightIndex = 0 ; pointLightIndex < gLightCounts.pointLightCount ; ++pointLightIndex) {
        float3 lightDir = normalize(input.worldPos - gPointLight[pointLightIndex].pos);
        
        float distance = length(gPointLight[pointLightIndex].pos - input.worldPos);
        float factElement = saturate(distance / gPointLight[pointLightIndex].radius + 1.0f);
        float factor = pow(factElement,
                         gPointLight[pointLightIndex].decay);
        
        float3 lightColor = gPointLight[pointLightIndex].color.rgb * ( gPointLight[pointLightIndex].intensity * factor );
        
        float NdotL = saturate(dot(normal,lightDir));
        float cos = pow(NdotL * 0.5f + 0.5f,2.0f);
        
        float3 diffuse = textureColor.rgb * lightColor * cos;
        
        float3 halfVector = normalize(lightDir + viewDir);
        float NdotH = dot(normal,halfVector);
        float specularPow = pow(saturate(NdotH),gMaterial.shininess);
        
        float3 specular = lightColor * specularPow * gMaterial.specularColor;
        
        output.color.rgb += diffuse + specular;
    }

        // Spot Light
    for (uint spotLightIndex = 0 ; spotLightIndex < gLightCounts.spotLightCount ; ++spotLightIndex) {
        float3 lightDir = normalize(input.worldPos - gSpotLight[spotLightIndex].pos);
        float distance = length(gSpotLight[spotLightIndex].pos - input.worldPos);
        float attenuationFactor = pow(saturate(1.0f - distance / gSpotLight[spotLightIndex].distance),gSpotLight[spotLightIndex].decay);
        float cosAngle = dot(lightDir,gSpotLight[spotLightIndex].direction);
        float falloffFactor = saturate(( cosAngle - gSpotLight[spotLightIndex].cosAngle ) / ( gSpotLight[spotLightIndex].cosFalloffStart - gSpotLight[spotLightIndex].cosAngle ));
        float3 lightColor = gSpotLight[spotLightIndex].color.rgb * gSpotLight[spotLightIndex].intensity * attenuationFactor * falloffFactor;
        
        float NdotL = saturate(dot(normal,lightDir));
        float cos = pow(NdotL * 0.5f + 0.5f,2.0f);
        
        float3 diffuse = textureColor.rgb * lightColor * cos;
        
        float3 halfVector = normalize(lightDir + viewDir);
        float NdotH = dot(normal,halfVector);
        float specularPow = pow(saturate(NdotH),gMaterial.shininess);
        
        float3 specular = lightColor * specularPow * gMaterial.specularColor;
        
        output.color.rgb += diffuse + specular;
    }
    
    // Lambert
    //float3 normal = normalize(input.normal);
    //float3 viewDir = normalize(gViewProjection.cameraPos - input.worldPos);
    //output.color.rgb = float3(0.0f,0.0f,0.0f);

    //    // Directional Light
    //for (uint directionalLightIndex = 0 ; directionalLightIndex < gLightCounts.directionalLightCount ; ++directionalLightIndex) {
    //    float3 dirLightColor = LambertDiffuse(normal,-gDirectionalLight[directionalLightIndex].direction,gDirectionalLight[directionalLightIndex].color,gDirectionalLight[directionalLightIndex].intensity);
    //    output.color.rgb += textureColor.rgb * dirLightColor;
    //}

    //    // Point Light
    //for (uint pointLightIndex = 0 ; pointLightIndex < gLightCounts.pointLightCount ; ++pointLightIndex) {
    //    float3 lightDir = normalize(input.worldPos - gPointLight[pointLightIndex].pos);
    //    float distance = length(gPointLight[pointLightIndex].pos - input.worldPos);
    //    float attenuation = pow(saturate(1.0f - distance / gPointLight[pointLightIndex].radius),gPointLight[pointLightIndex].decay);
    //    float3 lightColor = LambertDiffuse(normal,lightDir,gPointLight[pointLightIndex].color,gPointLight[pointLightIndex].intensity * attenuation);
    //    output.color.rgb += textureColor.rgb * lightColor;
    //}

    //    // Spot Light
    //for (uint spotLightIndex = 0 ; spotLightIndex < gLightCounts.spotLightCount ; ++spotLightIndex) {
    //    float3 lightDir = normalize(input.worldPos - gSpotLight[spotLightIndex].pos);
    //    float distance = length(gSpotLight[spotLightIndex].pos - input.worldPos);
    //    float attenuation = pow(saturate(1.0f - distance / gSpotLight[spotLightIndex].distance),gSpotLight[spotLightIndex].decay);
    //    float cosAngle = dot(lightDir,normalize(gSpotLight[spotLightIndex].direction));
    //    float falloff = saturate(( cosAngle - gSpotLight[spotLightIndex].cosAngle ) / ( gSpotLight[spotLightIndex].cosFalloffStart - gSpotLight[spotLightIndex].cosAngle ));
    //    float3 lightColor = LambertDiffuse(normal,lightDir,gSpotLight[spotLightIndex].color,gSpotLight[spotLightIndex].intensity * attenuation * falloff);
    //    output.color.rgb += textureColor.rgb * lightColor;
    //}
    
    return output;
}