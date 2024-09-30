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

struct DirectionalLight {
    float4 color;
    float3 direction;
    float intensity;
};
///========================================
/// ConstantBufferの定義
///========================================

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

Texture2D<float4> gTexture : register(t0); // SRVの registerは t
SamplerState gSampler : register(s0); // textureを読むためのもの. texture のサンプリングを担当

PixelShaderOutput main(VertexShaderOutput input) {
    PixelShaderOutput output;

    // texcoord を z=0 の (3+1)次元 として考える
    float4 transformedUV = mul(float4(input.texCoord,0.0f,1.0f),gMaterial.uvTransform);
    float4 textureColor = gMaterial.color * gTexture.Sample(gSampler,transformedUV.xy);
    if (textureColor.a <= 0.1f) {
        discard;
    }

    output.color = gMaterial.color * textureColor;
                          
    switch (gMaterial.enableLighting) {
        case 1:{
                float NdotL = dot(normalize(input.normal),-gDirectionalLight.direction);
                float cos = pow(NdotL * 0.5f + 0.5f,2.0f);
                output.color.rgb = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
            };
        case 2:{
                float cos = saturate(dot(normalize(input.normal),-gDirectionalLight.direction));
                output.color.rgb = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
            };
        case 3:{
                float3 toEye = normalize(gViewProjection.cameraPos - input.worldPos);

                float cos = saturate(dot(normalize(input.normal),-gDirectionalLight.direction));

                float3 reflectLight = reflect(-gDirectionalLight.direction,normalize(input.normal));
                float rDotE = dot(reflectLight,toEye);
                float specularPow = pow(saturate(rDotE),gMaterial.shininess);

                float3 diffuse = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
                float3 specular = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * gMaterial.specularColor;

                output.color.rgb = diffuse + specular;
            }
        case 4:{
                float3 toEye = normalize(gViewProjection.cameraPos - input.worldPos);

                float cos = saturate(dot(normalize(input.normal),-gDirectionalLight.direction));

                float3 halfVec = normalize(-gDirectionalLight.direction + toEye);
                float nDotH = dot(normalize(input.normal),halfVec);
                float specularPow = pow(saturate(nDotH),gMaterial.shininess);

                float3 diffuse = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
                float3 specular = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * gMaterial.specularColor;
            }
    }
    
    return output;
}