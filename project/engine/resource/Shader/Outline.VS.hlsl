#include "Object3dTextureColor.hlsli"

struct WorldTransform
{
    float4x4 world;
};
struct OutlineParam
{
    float outlineWidth;
    float4 outlineColor;
};

ConstantBuffer<WorldTransform> gWorldTransform : register(b0);


ConstantBuffer<OutlineParam> gOutlineParam : register(b1);

struct VertexShaderInput
{
    float4 pos : POSITION0;
    float2 texCoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float4 color : COLOR0;
};

/// Object3dTextureColorのオブジェクトのアウトライン描画用の頂点シェーダー
VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    //mul は 行列の 積
    float4x4 vpvMat = mul(mul(gWorldTransform.world, gViewProjection.view), gViewProjection.projection);
    output.normal = normalize(mul(input.normal, (float3x3) gWorldTransform.world));
    float3 outlinePos = input.pos.xyz + output.normal * gOutlineParam.outlineWidth;
    output.pos = mul(float4(outlinePos, input.pos.w), vpvMat);
    
    output.texCoord = input.texCoord;
    output.worldPos = mul(outlinePos, (float3x3)gWorldTransform.world);
    
    output.color = input.color * gOutlineParam.outlineColor;
    
    return output;
}
