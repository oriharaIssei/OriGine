#include "Particle.hlsli"

#include "Random.hlsli"

static const int kMaxParticleSize = 1024;

struct EmitterShape
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
    float pad2;
};

struct PerFrame
{
    float time;
    float deltaTime;
};

ConstantBuffer<EmitterShape> gEmitteShape : register(b0);
ConstantBuffer<PerFrame> gPerFrame : register(b1);

RWStructuredBuffer<GPUParticleData> gParticles : register(u0);
RWStructuredBuffer<int> gFreeListIndex : register(u1);
RWStructuredBuffer<uint> gFreeList : register(u2);

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    if (gEmitteShape.emit != 0)
    {
        RandomGenerator3d randGen;
        randGen.seed = (DTid + gPerFrame.time) * gPerFrame.time;

        uint emitCount = uint(randGen.Generate1d(float(gEmitteShape.minCount), float(gEmitteShape.maxCount)));

        for (uint i = 0; i < emitCount; ++i)
        {
            int perticleIndex = 0;
            InterlockedAdd(gFreeListIndex[0], -1, perticleIndex);

            if (perticleIndex >= kMaxParticleSize || perticleIndex >= 0)
            {
                InterlockedAdd(gFreeListIndex[0], 1);
                break; // Avoid exceeding the maximum particle size
            }
            
            GPUParticleData particle = (GPUParticleData) 0;
            
            particle.scale = randGen.Generate3d(gEmitteShape.minScale, gEmitteShape.maxScale);
            particle.velocity = randGen.Generate3d(gEmitteShape.minVelocity, gEmitteShape.maxVelocity);
            particle.color = float4(randGen.Generate3d(gEmitteShape.minColor, gEmitteShape.maxColor), 1.0f);
            particle.lifeTime = randGen.Generate1d(gEmitteShape.minLifeTime, gEmitteShape.maxLifeTime);

            float3 halfSize = gEmitteShape.size * 0.5f;
            /// Box
            if (gEmitteShape.isBox != 0)
            { // Edge
                if (gEmitteShape.isEdge != 0)
                {
                    float3 dir = normalize(randGen.Generate3d());

                    float3 halfSize = gEmitteShape.size * 0.5f;
                    float3 center = gEmitteShape.center;

                    // 各軸でAABBの面までのtを計算
                    float3 t;
                    t.x = (dir.x > 0) ? (halfSize.x / abs(dir.x)) : (-halfSize.x / abs(dir.x));
                    t.y = (dir.y > 0) ? (halfSize.y / abs(dir.y)) : (-halfSize.y / abs(dir.y));
                    t.z = (dir.z > 0) ? (halfSize.z / abs(dir.z)) : (-halfSize.z / abs(dir.z));

                    // 0除算を防ぐ
                    float tx = (dir.x != 0) ? (halfSize.x / abs(dir.x)) : 1e20;
                    float ty = (dir.y != 0) ? (halfSize.y / abs(dir.y)) : 1e20;
                    float tz = (dir.z != 0) ? (halfSize.z / abs(dir.z)) : 1e20;

                    // 最小のtがAABBのEdgeまでの距離
                    float tEdge = min(tx, min(ty, tz));

                    // Edge座標
                    particle.translate = center + dir * tEdge;
                }
                else
                {
                    particle.translate = gEmitteShape.center + randGen.Generate3d(-halfSize, halfSize);
                }
            }
            else // Sphere
            {
               
                float3 dir = normalize(randGen.Generate3d());
                float3 radius = float3(halfSize.x, halfSize.y, halfSize.z); // 各軸の半径

                float r = 1.0 / sqrt((dir.x * dir.x) / (radius.x * radius.x)
                    + (dir.y * dir.y) / (radius.y * radius.y)
                    + (dir.z * dir.z) / (radius.z * radius.z)
                    );
                
                // Edge
                if (gEmitteShape.isEdge != 0)
                {
                    particle.translate = gEmitteShape.center + dir * r;
                }
                else
                {
                    particle.translate = gEmitteShape.center + dir * randGen.Generate1d(0.f, r);
                }
            }
            
            gParticles[perticleIndex] = particle;
        }
    }

}
