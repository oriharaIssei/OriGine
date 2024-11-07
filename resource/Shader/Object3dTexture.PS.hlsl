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
    float3 direction;
    float intensity;
};

struct PointLight {
    float3 color;
    float3 pos;
    float intensity;
    float radius;
    float decay;
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
/// Light
///========================================

///========================================
/// ConstantBuffer
///========================================
ConstantBuffer<Material> gMaterial : register(b0);

ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

// b2 = viewProjection from hlsli
ConstantBuffer<PointLight> gPointLight : register(b3);
ConstantBuffer<SpotLight> gSpotLight : register(b4);

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

            // Directional Light
                float3 dirLightColor = LambertDiffuse(normal,-gDirectionalLight.direction,gDirectionalLight.color,gDirectionalLight.intensity);
                output.color.rgb = textureColor.rgb * dirLightColor;

            // Point Light
                float3 pointLightDir = normalize(input.worldPos - gPointLight.pos);
                float distance = length(gPointLight.pos - input.worldPos);
                float attenuation = pow(saturate(1.0f - distance / gPointLight.radius),gPointLight.decay);
                float3 pointLightColor = LambertDiffuse(normal,pointLightDir,gPointLight.color,gPointLight.intensity * attenuation);
                output.color.rgb += textureColor.rgb * pointLightColor;

            // Spot Light
                float3 spotLightDir = normalize(input.worldPos - gSpotLight.pos);
                distance = length(gSpotLight.pos - input.worldPos);
                attenuation = pow(saturate(1.0f - distance / gSpotLight.distance),gSpotLight.decay);
                float cosAngle = dot(spotLightDir,normalize(gSpotLight.direction));
                float falloff = saturate(( cosAngle - gSpotLight.cosAngle ) / ( gSpotLight.cosFalloffStart - gSpotLight.cosAngle ));
                float3 spotLightColor = LambertDiffuse(normal,spotLightDir,gSpotLight.color,gSpotLight.intensity * attenuation * falloff);
                output.color.rgb += textureColor.rgb * spotLightColor;

                break;
            }

        case 2:{ // Lambert
                float3 normal = normalize(input.normal);
                float3 viewDir = normalize(gViewProjection.cameraPos - input.worldPos);

            // Directional Light
                float3 dirLightColor = LambertDiffuse(normal,-gDirectionalLight.direction,gDirectionalLight.color,gDirectionalLight.intensity);
                output.color.rgb = textureColor.rgb * dirLightColor;

            // Point Light
                float3 pointLightDir = normalize(input.worldPos - gPointLight.pos);
                float distance = length(gPointLight.pos - input.worldPos);
                float attenuation = pow(saturate(1.0f - distance / gPointLight.radius),gPointLight.decay);
                float3 pointLightColor = LambertDiffuse(normal,pointLightDir,gPointLight.color,gPointLight.intensity * attenuation);
                output.color.rgb += textureColor.rgb * pointLightColor;

            // Spot Light
                float3 spotLightDir = normalize(input.worldPos - gSpotLight.pos);
                distance = length(gSpotLight.pos - input.worldPos);
                attenuation = pow(saturate(1.0f - distance / gSpotLight.distance),gSpotLight.decay);
                float cosAngle = dot(spotLightDir,normalize(gSpotLight.direction));
                float falloff = saturate(( cosAngle - gSpotLight.cosAngle ) / ( gSpotLight.cosFalloffStart - gSpotLight.cosAngle ));
                float3 spotLightColor = LambertDiffuse(normal,spotLightDir,gSpotLight.color,gSpotLight.intensity * attenuation * falloff);
                output.color.rgb += textureColor.rgb * spotLightColor;

                break;
            }

        case 3:{ // Phong
                float3 normal = normalize(input.normal);
                float3 viewDir = normalize(gViewProjection.cameraPos - input.worldPos);

            // Directional Light - Diffuse + Specular
                float3 dirLightDiffuse = LambertDiffuse(normal,-gDirectionalLight.direction,gDirectionalLight.color,gDirectionalLight.intensity);
                float3 dirLightSpecular = PhongSpecular(normal,-gDirectionalLight.direction,viewDir,gMaterial.shininess,gDirectionalLight.color,gDirectionalLight.intensity);
                output.color.rgb = textureColor.rgb * dirLightDiffuse + dirLightSpecular;

            // Point Light - Diffuse + Specular
                float3 pointLightDir = normalize(input.worldPos - gPointLight.pos);
                float distance = length(gPointLight.pos - input.worldPos);
                float attenuation = pow(saturate(1.0f - distance / gPointLight.radius),gPointLight.decay);
                float3 pointLightDiffuse = LambertDiffuse(normal,pointLightDir,gPointLight.color,gPointLight.intensity * attenuation);
                float3 pointLightSpecular = PhongSpecular(normal,pointLightDir,viewDir,gMaterial.shininess,gPointLight.color,gPointLight.intensity * attenuation);
                output.color.rgb += textureColor.rgb * pointLightDiffuse + pointLightSpecular;

                break;
            }

        case 4:{ // Blinn-Phong
                float3 normal = normalize(input.normal);
                float3 viewDir = normalize(gViewProjection.cameraPos - input.worldPos);

            // Directional Light - Diffuse + Blinn-Phong Specular
                float3 dirLightDiffuse = LambertDiffuse(normal,-gDirectionalLight.direction,gDirectionalLight.color,gDirectionalLight.intensity);
                float3 dirLightSpecular = BlinnPhongSpecular(normal,-gDirectionalLight.direction,viewDir,gMaterial.shininess,gDirectionalLight.color,gDirectionalLight.intensity);
                output.color.rgb = textureColor.rgb * dirLightDiffuse + dirLightSpecular;

            // Point Light - Diffuse + Blinn-Phong Specular
                float3 pointLightDir = normalize(input.worldPos - gPointLight.pos);
                float distance = length(gPointLight.pos - input.worldPos);
                float attenuation = pow(saturate(1.0f - distance / gPointLight.radius),gPointLight.decay);
                float3 pointLightDiffuse = LambertDiffuse(normal,pointLightDir,gPointLight.color,gPointLight.intensity * attenuation);
                float3 pointLightSpecular = BlinnPhongSpecular(normal,pointLightDir,viewDir,gMaterial.shininess,gPointLight.color,gPointLight.intensity * attenuation);
                output.color.rgb += textureColor.rgb * pointLightDiffuse + pointLightSpecular;

                break;
            }
    }
    return output;
}