#include "Particle.hlsli"

struct WorldTransform {
    float4x4 world;
    float4 color : COLOR0;
};

struct ViewProjection {
    float3 cameraPos;
    float4x4 view;
    float4x4 viewTranspose;
    float4x4 projection;
};

StructuredBuffer<WorldTransform> gWorldTransform : register(t0);
ConstantBuffer<ViewProjection> gViewProjection : register(b1);

struct VertexShaderInput {
    float4 pos : POSITION0;
    float2 texCoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input,uint instanceID : SV_InstanceID) {
    VertexShaderOutput output;
    //mul は 行列の 積
    float4x4 vpvMat = mul(mul(gWorldTransform[instanceID].world,gViewProjection.view),gViewProjection.projection);
    output.pos = mul(input.pos,vpvMat);
    output.texCoord = input.texCoord;
    output.normal = normalize(mul(input.normal,(float3x3)gWorldTransform[instanceID].world));
    output.color = gWorldTransform[instanceID].color;
    return output;
}