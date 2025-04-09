#include "ColoredVertex.hlsli"

struct ColoredVertexData
{
    float4 pos : POSITION0;
    float4 color : COLOR0;
};

struct WorldTransform
{
    float4x4 worldMat;
};
struct ViewProjection
{
    float3 cameraPos;
    float4x4 view;
    float4x4 viewTranspose;
    float4x4 projection;
};
ConstantBuffer<WorldTransform> gWorldTransform : register(b0);
ConstantBuffer<ViewProjection> gViewProjection : register(b1);

VsOutput main(ColoredVertexData input)
{
    VsOutput output;
    float4x4 vpvMat = mul(mul(gWorldTransform.worldMat, gViewProjection.view), gViewProjection.projection);
    output.pos = mul(input.pos, vpvMat);
    output.color = input.color;
    return output;
}
