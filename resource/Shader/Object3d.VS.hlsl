#include "Object3d.hlsli"

struct WorldTransform {
    float4x4 world;
};

ConstantBuffer<WorldTransform> gWorldTransform : register(b0);

struct VertexShaderInput {
    float4 pos : POSITION0;
    float3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input) {
    VertexShaderOutput output;
    //mul は 行列の 積
    float4x4 vpvMat = mul(mul(gWorldTransform.world,gViewProjection.view),gViewProjection.projection);
    
    output.pos = mul(input.pos,vpvMat);
    output.normal = normalize(mul(input.normal,(float3x3)gViewProjection.viewTranspose));
    output.worldPos = mul(input.pos,gWorldTransform.world).xyz;
    return output;
}