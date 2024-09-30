#include "Object3d.hlsli"

///========================================
/// Material
///========================================
struct Material {
    float4 color;
    int enableLighting;
    float4x4 uvTransform;
    float shininess;
    float3 specularColor;
};

///========================================
/// ShaderOutput
///========================================
struct PixelShaderOutput {
    float4 color : SV_TARGET0;
};

///========================================
/// Light
struct DirectionalLight {
    float4 color;
    float3 direction;
    float intensity;
};

struct PointLight {
    float4 color;
    float3 pos;
    float intensity;
    float radius;
    float decay;
};

struct SpotLight {
    float4 color;
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

PixelShaderOutput main(VertexShaderOutput input) {
    PixelShaderOutput output;

    output.color = gMaterial.color;
    
    switch (gMaterial.enableLighting) {
        case 1:{ // half Lambert
                float3 inputNormal = normalize(input.normal);
                ///=============================================
                /// DirectionalLight
                ///=============================================
                float NdotL = dot(inputNormal,-gDirectionalLight.direction);
                float cos = pow(NdotL * 0.5f + 0.5f,2.0f);
                output.color.xyz = gMaterial.color.xyz * gMaterial.color.xyz * gDirectionalLight.color.xyz * cos * gDirectionalLight.intensity;
                
                ///=============================================
                /// PointlLight
                ///=============================================
                float3 pointLightDir = normalize(input.worldPos - gPointLight.pos);
                float distance = length(gPointLight.pos - input.worldPos);
                float distFactor = pow(saturate(-distance / gPointLight.radius + 1.0f),gPointLight.decay);
            
                NdotL = dot(inputNormal,pointLightDir);
                cos = pow(NdotL * 0.5f + 0.5f,2.0f);
                
                output.color.xyz += gMaterial.color.xyz * gMaterial.color.xyz * gPointLight.color.xyz * cos * gPointLight.intensity * distFactor;
                
                ///=============================================
                /// SpotlLight
                ///=============================================
                float3 spotLightDir = normalize(input.worldPos - gSpotLight.pos);
                distance = length(gSpotLight.pos - input.worldPos);
                distFactor = pow(saturate(1.0f - distance / gSpotLight.distance),gSpotLight.decay);
                float cosAngle = dot(spotLightDir,normalize(gSpotLight.direction));
                float falloffFactor = saturate(( cosAngle - gSpotLight.cosAngle ) / ( gSpotLight.cosFalloffStart - gSpotLight.cosAngle ));
                
                NdotL = dot(inputNormal,spotLightDir);
                cos = pow(NdotL * 0.5f + 0.5f,2.0f);
                
                output.color.xyz += gMaterial.color.xyz * gMaterial.color.xyz * gSpotLight.color.xyz * cos * gSpotLight.intensity * falloffFactor * distFactor;
                
            
                break;
            };
        case 2:{// Lambert
                float cos = saturate(dot(normalize(input.normal),-gDirectionalLight.direction));
                output.color.xyz = gMaterial.color.xyz * gMaterial.color.xyz * gDirectionalLight.color.xyz * cos * gDirectionalLight.intensity;
                
                ///=============================================
                /// PointlLight
                ///=============================================
                float3 pointLightDir = normalize(input.worldPos - gPointLight.pos);
                float distance = length(gPointLight.pos - input.worldPos);
                float distFactor = pow(saturate(-distance / gPointLight.radius + 1.0f),gPointLight.decay);
            
                cos = saturate(dot(normalize(input.normal),-pointLightDir));
                output.color.xyz += gMaterial.color.xyz * gMaterial.color.xyz * gPointLight.color.xyz * cos * gDirectionalLight.intensity * distFactor;
                
                ///=============================================
                /// SpotlLight
                ///=============================================
                float3 spotLightDir = normalize(input.worldPos - gSpotLight.pos);
                break;
            };
        case 3:{// Phong
                float3 toEye = normalize(gViewProjection.cameraPos - input.worldPos);

                float cos = saturate(dot(normalize(input.normal),-gDirectionalLight.direction));

                float3 reflectLight = reflect(gDirectionalLight.direction,normalize(input.normal));
                float rDotE = dot(reflectLight,toEye);
                float specularPow = pow(saturate(rDotE),gMaterial.shininess);

                float3 diffuse = gMaterial.color.rgb * gMaterial.color.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
                float3 specular = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * gMaterial.specularColor;
                output.color.rgb = diffuse + specular;
                break;
            }
        case 4:{// bllin Phong
                float3 toEye = normalize(gViewProjection.cameraPos - input.worldPos);
                float3 inputNormal = normalize(input.normal);
                ///=============================================
                /// DirectionalLight
                ///=============================================

                float cos = saturate(dot(inputNormal,-gDirectionalLight.direction));

                float3 halfVec = normalize(-gDirectionalLight.direction + toEye);
                float nDotH = dot(inputNormal,halfVec);
                float specularPow = pow(saturate(nDotH),gMaterial.shininess);

                float3 directionalLightColor = gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
                float3 diffuse = gMaterial.color.rgb * gMaterial.color.rgb * directionalLightColor;
                float3 specular = directionalLightColor * specularPow * gMaterial.specularColor;
                output.color.rgb = diffuse + specular;
            
                ///=============================================
                /// PointlLight
                ///=============================================
                float3 pointLightDir = normalize(input.worldPos - gPointLight.pos);
                float distance = length(gPointLight.pos - input.worldPos);
            
                float factor = pow(saturate(-distance / gPointLight.radius + 1.0f),gPointLight.decay);
            
                cos = saturate(dot(inputNormal,pointLightDir));
                halfVec = normalize(-pointLightDir + toEye);
                nDotH = dot(inputNormal,halfVec);
                specularPow = pow(saturate(nDotH),gMaterial.shininess);
                
                float3 pointLightColor = gPointLight.color.rgb * cos * gPointLight.intensity * factor;
                diffuse = gMaterial.color.rgb * gMaterial.color.rgb * pointLightColor;
                specular = pointLightColor * specularPow * gMaterial.specularColor;
                output.color.rgb += diffuse + specular;
            
                ///=============================================
                /// SpotLight
                ///=============================================
                float3 spotLightDirectionOnSurface = normalize(input.worldPos - gSpotLight.pos);

                float cosAngle = dot(spotLightDirectionOnSurface,normalize(gSpotLight.direction));
                float falloffFactor = saturate(( cosAngle - gSpotLight.cosAngle ) / ( gSpotLight.cosFalloffStart - gSpotLight.cosAngle ));

                distance = length(gSpotLight.pos - input.worldPos);
                float attenuationFactor = pow(saturate(1.0f - distance / gSpotLight.distance),gSpotLight.decay);

                float3 spotLightColor = gSpotLight.color.rgb * gSpotLight.intensity * attenuationFactor * falloffFactor;

                halfVec = normalize(spotLightDirectionOnSurface + toEye);
                nDotH = dot(inputNormal,halfVec);
                specularPow = pow(saturate(nDotH),gMaterial.shininess);

                diffuse = gMaterial.color.rgb * spotLightColor * saturate(dot(inputNormal,spotLightDirectionOnSurface));
                specular = spotLightColor * specularPow * gMaterial.specularColor;

                output.color.rgb += diffuse + specular;

                break;
            }
    }
    
    return output;
}