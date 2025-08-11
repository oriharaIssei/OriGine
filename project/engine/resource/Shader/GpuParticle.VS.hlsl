#include "Particle.hlsli"

StructuredBuffer<GpuParticleData> gParticleData : register(t0);
ConstantBuffer<PerView> gPerView : register(b0);

struct VertexShaderInput
{
    float4 pos : POSITION0;
    float2 texCoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input, uint instanceID : SV_InstanceID)
{
    VertexShaderOutput output;
    GpuParticleData particle = gParticleData[instanceID];

    float4x4 worldMat = gPerView.billboardMat;
    worldMat[0] *= particle.scale.x;
    worldMat[1] *= particle.scale.y;
    worldMat[2] *= particle.scale.z;
    worldMat[3].xyz += particle.translate;

    output.pos = mul(input.pos, mul(worldMat, gPerView.viewProjection));
    output.texCoord = input.texCoord;
    output.color = particle.color;
    
    return output;
}
