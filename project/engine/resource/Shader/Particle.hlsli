
struct VertexShaderOutput {
    float4 pos : SV_Position0;
    float3 normal : NORMAL0;
    float2 texCoord : TEXCOORD0;
    
    float4 color : COLOR0;
};

/// ============================
// For GPU Particle 
/// ============================

struct GpuParticleData
{
    float3 scale;
    float3 translate;
    float lifeTime;
    float3 velocity;
    float maxLifeTime;
    float4 color;
};
struct GpuEmitterShape
{
    float3 minColor;
    float minLifeTime;
    float3 maxColor;
    float maxLifeTime;
    
    float3 center;
    uint minCount;

    float3 size;
    uint maxCount;

    float3 minVelocity;
    uint isBox;

    float3 maxVelocity;
    uint emit;

    float3 minScale;
    uint isEdge;
    float3 maxScale;
    uint particleSize;
};
struct PerView
{
    float4x4 viewProjection;
    float4x4 billboardMat;
};
