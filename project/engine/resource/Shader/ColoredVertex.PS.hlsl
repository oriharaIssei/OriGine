#include "ColoredVertex.hlsli"

float4 main(ColoredVertexData input) : SV_TARGET
{
    if (input.color.a <= 0.01f)
    {
        discard;
    }

    return input.color;
}
