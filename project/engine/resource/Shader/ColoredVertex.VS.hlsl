#include "ColoredVertex.hlsli"

struct WorldTransform
{
    float4x4 worldMat;
};
struct ViewProjection
{
    float4x4 viewMat;
    float4x4 projectionMat;
    
};
ConstantBuffer<WorldTransform> gWorldTransform : register(b0);
ConstantBuffer<ViewProjection> gViewProjection : register(b1);

ColoredVertexData main(ColoredVertexData input)
{
    ColoredVertexData output;
    float4x4 vpvMat = mul(mul(gWorldTransform.worldMat, gViewProjection.viewMat), gViewProjection.projectionMat);
    output.pos = mul(input.pos, vpvMat);
    return output;
}
