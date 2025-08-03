
struct VertexShaderOutput {
    float4 pos : SV_Position0;
    float3 normal : NORMAL0;
    float2 texCoord : TEXCOORD0;
    
    float4 color : COLOR0;
};

/// ============================
// For GPU Particle 
/// ============================

struct GPUParticleData
{
    float3 scale;
    float3 translate;
    float lifeTime;
    float3 velocity;
    float currentTime;
    float4 color;
};
struct PerView
{
    float4x4 viewProjection;
    float4x4 billboardMat;
};
