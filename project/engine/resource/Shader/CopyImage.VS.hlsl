#include "CopyImage.hlsli"

static const uint kNumVertex = 4;
static const uint kNumVertexIndex = 6;

static const uint kVertexIndex[kNumVertexIndex] =
{
    0, 1, 2,
    1, 3, 2
};

static const float4 kPosition_[kNumVertex] =
{
    { -1.0f, 1.0f, 0.0f, 1.0f },
    { 1.0f, 1.0f, 0.0f, 1.0f },
    { -1.0f, -1.0f, 0.0f, 1.0f },
    { 1.0f, -1.0f, 0.0f, 1.0f }
};

static float2 kTexCoords[kNumVertex] =
{
    { 0.0f, 0.0f },
    { 1.0f, 0.0f },
    { 0.0f, 1.0f },
    { 1.0f, 1.0f }
};

VertexShaderOutput main(uint vertexId : SV_VertexID)
{
    uint vertexIndex = kVertexIndex[vertexId];
    VertexShaderOutput output;
    output.pos = kPosition_[vertexIndex];
    output.texCoords = kTexCoords[vertexIndex];
    return output;
}
