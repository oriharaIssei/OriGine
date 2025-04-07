#include "ColoredVertex.hlsli"

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(ColoredVertexData input)
{
    PixelShaderOutput output;

    if (input.color.a <= 0.01f)
    {
        discard;
    }
    
    output.color = input.color;

    return output;
}
