#include "Particle.hlsli"

#include "Random.hlsli"

static const int kMaxParticleSize = 1024;

struct PerFrame
{
    float time;
    float deltaTime;
};

ConstantBuffer<PerFrame> gPerFrame : register(b0);

RWStructuredBuffer<GPUParticleData> gParticles : register(u0);
RWStructuredBuffer<int> gFreeListIndex : register(u1);
RWStructuredBuffer<uint> gFreeList : register(u2);

[numthreads(1024, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint perticleIndex = DTid.x;
    if (perticleIndex >= kMaxParticleSize)
    {
        return; // Avoid exceeding the maximum particle size
    }

    if (gParticles[perticleIndex].color.a == 0.0f)
    {
        int freeIndex = 0;
        InterlockedAdd(gFreeListIndex[0], 1, freeIndex);

        if ((freeIndex + 1) < kMaxParticleSize)
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

    gParticles[perticleIndex].color.a = 1.f - (gParticles[perticleIndex].currentTime / gParticles[perticleIndex].lifeTime);
    
}
