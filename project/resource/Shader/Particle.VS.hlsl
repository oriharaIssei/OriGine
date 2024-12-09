#include "Particle.hlsli"

struct ParticleData {
    float4x4 worldMat;
    float4x4 uvMat;
    float4 color : COLOR0;
};

struct ViewProjection {
    float3 cameraPos;
    float4x4 view;
    float4x4 viewTranspose;
    float4x4 projection;
};

StructuredBuffer<ParticleData> gParticleData : register(t0);
ConstantBuffer<ViewProjection> gViewProjection : register(b1);

struct VertexShaderInput {
    float4 pos : POSITION0;
    float2 texCoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input,uint instanceID : SV_InstanceID) {
    VertexShaderOutput output;
    //mul は 行列の 積
    float4x4 vpvMat = mul(mul(gParticleData[instanceID].worldMat,gViewProjection.view),gViewProjection.projection);
    output.pos = mul(input.pos,vpvMat);
    output.texCoord = mul(float4(input.texCoord,0.0f,1.0f),gParticleData[instanceID].uvMat).xy;
    output.normal = normalize(mul(input.normal,(float3x3)gParticleData[instanceID].worldMat));
    output.color = gParticleData[instanceID].color;
    return output;
}