#include "ColoredVertex.hlsli"

float4 main(VsOutput input) : SV_TARGET
{
    if (input.color.a <= 0.01f)
    {
        discard;
    }

    return input.color;
}
