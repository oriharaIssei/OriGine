#pragma once

cbuffer Line2DCB : register(b0) {
    float4x4 vpMat;
};

struct VSInput {
    float4 pos : POSITION;
    float4 color : COLOR;
};

struct VSOutput {
    float4 svpos : SV_POSITION;
    float4 color : COLOR0;
};
