#include "Skybox.hlsli"


struct TransformMatrix
{
    float4x4 wvp;
};

ConstantBuffer<TransformMatrix> gTransformMatrix : register(b0);

struct VertexShaderInput
{
    float4 pos : POSITION0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.pos = mul(input.pos, gTransformMatrix.wvp).xyww;
    output.texcoord = input.pos.xyz;
    return output;
}
