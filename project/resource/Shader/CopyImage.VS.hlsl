#include "CopyImage.hlsli"

static const uint kNumVertex = 3;
static const float4 kPosition_[kNumVertex] = {
    { -1.0f,1.0f,0.0f,1.0f },
    { 3.0f,1.0f,0.0f,1.0f },
    { -1.0f,-3.0f,0.0f,1.0f }
};

static float2 kTexCoords[kNumVertex] = {
    { 0.0f,0.0f },
    { 2.0f,0.0f },
    { 0.0f,2.0f }
};

VertexShaderOutput main(uint vertexId : SV_VertexID) {
    VertexShaderOutput output;
    output.pos = kPosition_[vertexId];
    output.texCoords = kTexCoords[vertexId];
    return output;
}