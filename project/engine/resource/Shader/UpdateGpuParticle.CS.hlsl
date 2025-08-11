#include "Particle.hlsli"

#include "Random.hlsli"

struct PerFrame
{
    float time;
    float deltaTime;
};

RWStructuredBuffer<GpuParticleData> gParticles : register(u0);
RWStructuredBuffer<int> gFreeListIndex : register(u1);
RWStructuredBuffer<uint> gFreeList : register(u2);

ConstantBuffer<GpuEmitterShape> gEmitteShape : register(b0);
ConstantBuffer<PerFrame> gPerFrame : register(b1);

[numthreads(1024, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint perticleIndex = DTid.x;
    if (perticleIndex >= gEmitteShape.particleSize)
    {
        return; // Avoid exceeding the maximum particle size
    }

    if (gParticles[perticleIndex].color.a == 0.0f)
    {
        int freeIndex = 0;
        InterlockedAdd(gFreeListIndex[0], 1, freeIndex);

        if ((freeIndex + 1) < gEmitteShape.particleSize)
        {
            gFreeList[freeIndex] = perticleIndex; // Update the free list
        }
        else
        {
            InterlockedAdd(gFreeListIndex[0], -1); // Restore the index if we exceed the limit
        }
        
        return; // Skip particles that are not alive
    }
    
    gParticles[perticleIndex].lifeTime -= gPerFrame.deltaTime;

    gParticles[perticleIndex].translate += gParticles[perticleIndex].velocity * gPerFrame.deltaTime;

    gParticles[perticleIndex].color.a = gParticles[perticleIndex].lifeTime / gParticles[perticleIndex].maxLifeTime;
    
}
