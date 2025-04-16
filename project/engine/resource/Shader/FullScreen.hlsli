
struct VertexShaderOutput
{
    float4 pos : SV_Position0;
    float2 texCoords : TEXCOORDS0;
};

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};
