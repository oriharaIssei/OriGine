
struct VertexShaderOutput {
    float4 pos : SV_Position0;
    float3 normal : NORMAL0;
    float2 texCoord : TEXCOORD0;
    float3 worldPos : POSITION0;
};

struct ViewProjection {
    float3 cameraPos;
    float4x4 view;
    float4x4 viewTranspose;
    float4x4 projection;
};

ConstantBuffer<ViewProjection> gViewProjection : register(b2);