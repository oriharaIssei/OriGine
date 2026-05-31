#include "Text.hlsli"

Texture2D<float4> fontAtlas : register(t0);
SamplerState fontSampler : register(s0);

float4 main(VSOutput input) : SV_TARGET {
    float4 texColor = fontAtlas.Sample(fontSampler, input.uv);
    float4 output = texColor * input.color;
    if (output.a < 0.01f) {
        discard;
    }
    return output;
}
