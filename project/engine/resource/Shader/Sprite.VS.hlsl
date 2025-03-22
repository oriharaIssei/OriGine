#include "Sprite.hlsli"

struct VSInput {
    float4 pos : POSITION;
    float2 uv : TEXCOORD;
};

VSOutput main(VSInput input) {
    VSOutput output;
    output.svpos = mul(input.pos,mat);
    output.uv = input.uv;
    return output;
}