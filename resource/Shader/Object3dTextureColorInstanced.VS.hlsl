#include "Object3dTextureColorInstanced.hlsli"

struct WorldTransform
{
    float4x4 world;
};

StructuredBuffer<WorldTransform> gInstanceTransforms : register(t6);

cbuffer InstanceOffset : register(b6)
{
    uint gInstanceOffset;
};

struct VertexShaderInput
{
    float4 pos      : POSITION0;
    float2 texCoord : TEXCOORD0;
    float3 normal   : NORMAL0;
    float4 color    : COLOR0;
    uint instanceId : SV_InstanceID;
};

InstancedVertexShaderOutput main(VertexShaderInput input)
{
    InstancedVertexShaderOutput output;
    uint adjustedId = input.instanceId + gInstanceOffset;
    float4x4 world = gInstanceTransforms[adjustedId].world;
    float4x4 vpvMat = mul(mul(world, gViewProjection.view), gViewProjection.projection);
    output.pos        = mul(input.pos, vpvMat);
    output.texCoord   = input.texCoord;
    output.normal     = normalize(mul(input.normal, (float3x3) world));
    output.worldPos   = mul(input.pos, world).xyz;
    output.color      = input.color;
    output.instanceId = adjustedId;
    return output;
}
