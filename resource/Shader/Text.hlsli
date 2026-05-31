#pragma once

cbuffer TextCB : register(b0) {
    float4x4 vpMat;
};

struct VSInput {
    float4 pos : POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};

struct VSOutput {
    float4 svpos : SV_POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};
