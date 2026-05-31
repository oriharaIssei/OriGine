#include "Line2D.hlsli"

VSOutput main(VSInput input) {
    VSOutput output;
    output.svpos = mul(input.pos, vpMat);
    output.color = input.color;
    return output;
}
