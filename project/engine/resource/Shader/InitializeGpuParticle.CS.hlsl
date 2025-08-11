#include "Particle.hlsli"

RWStructuredBuffer<GpuParticleData> gParticles : register(u0);
RWStructuredBuffer<int> gFreeListIndex : register(u1);
RWStructuredBuffer<uint> gFreeList : register(u2);

ConstantBuffer<GpuEmitterShape> gEmitteShape : register(b0);

[numthreads(1024, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint particleIndex = DTid.x;
    if (particleIndex < gEmitteShape.particleSize)
    {
        gParticles[particleIndex] = (GpuParticleData) 0;
        gParticles[particleIndex].color.a = 0.f;
        gFreeList[particleIndex] = particleIndex;

        if (particleIndex == 0)
        {
            gFreeListIndex[0] = gEmitteShape.particleSize - 1; // Initialize the free list index to the last particle
        }
    }

}
