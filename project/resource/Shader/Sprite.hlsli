#pragma once

cbuffer cbuff0 : register(b0) {
    float4 color;
    float4x4 mat;
    float4x4 uvMat;
};

struct VSOutput {
    float4 svpos : SV_POSITION0;
    float2 uv : TEXCOORD0;
};
