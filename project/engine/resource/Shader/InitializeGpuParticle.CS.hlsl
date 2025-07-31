#include "Particle.hlsli"

static const int kMaxParticleSize = 1024;

RWStructuredBuffer<GPUParticleData> gParticles : register(u0);

[numthreads(kMaxParticleSize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint particleIndex = DTid.x;
    if (particleIndex < kMaxParticleSize)
    {
        gParticles[particleIndex] = (GPUParticleData) 0;
        gParticles[particleIndex].scale = float3(1.0f, 1.0f, 1.0f);
        gParticles[particleIndex].color = float4(1.0f, 1.0f, 1.0f, 1.0f);

    }

}
