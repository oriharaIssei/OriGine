#include "Particle.hlsli"

static const int kMaxParticleSize = 1024;
RWStructuredBuffer<GPUParticleData> gParticles : register(u0);
RWStructuredBuffer<int> gFreeListIndex : register(u1);
RWStructuredBuffer<uint> gFreeList : register(u2);

[numthreads(kMaxParticleSize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint particleIndex = DTid.x;
    if (particleIndex < kMaxParticleSize)
    {
        gParticles[particleIndex] = (GPUParticleData) 0;
        gFreeList[particleIndex] = particleIndex;

        if (particleIndex == 0)
        {
            gFreeListIndex[0] = kMaxParticleSize - 1; // Initialize the free list index to the last particle
        }
    }

}
