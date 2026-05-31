#include "Text.hlsli"

VSOutput main(VSInput input) {
    VSOutput output;
    output.svpos = mul(input.pos, vpMat);
    output.uv = input.uv;
    output.color = input.color;
    return output;
}
