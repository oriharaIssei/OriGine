#include "Random.hlsli"

struct PointPlacementParams
{
    uint seed;

    float collisionRadius;

    float densityThreshold;
    float volumeThreshold;
    float maskThreshold;

    uint placementResolution; // 出力セル数 (R x R)
    uint sampleCount; // Box Sample の 1 辺 (例: 2,4)
    
    float pad;

    float2 worldMinXZ;
    float2 worldMaxXZ;
};

struct Point
{
    float3 pos : POSITION;
    float volume;
};

AppendStructuredBuffer<Point> gOutPoints : register(u0);

ConstantBuffer<PointPlacementParams> gParams : register(b0);

Texture2D<float4> gPointDensityTex : register(t0);
Texture2D<float4> gVolumeMaskTex : register(t1);
Texture2D<float4> gPointMaskTex : register(t2);
SamplerState gSampler : register(s0);

[numthreads(32, 32, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint R = gParams.placementResolution;
    uint2 cellId = DTid.xy;

    if (cellId.x >= R || cellId.y >= R)
    {
        return;
    }

    // --------------------
    // セル UV 範囲
    // --------------------
    float cellSize = 1.0f / R;
    float2 cellUVMin = float2(cellId) * cellSize;

    uint sampleCount = max(1, gParams.sampleCount);

    float densitySum = 0.0f;
    float maskSum = 0.0f;
    float volumeSum = 0.0f;
    
    for (uint y = 0; y < sampleCount; ++y)
    {
        for (uint x = 0; x < sampleCount; ++x)
        {
            uint2 sampleId = uint2(x, y);
            float2 noise = rand3dTo2d(float3(sampleId, float(gParams.seed)));

            float2 offset = (float2(sampleId) + noise) / sampleCount;
            float2 uv = cellUVMin + offset * cellSize;

            densitySum += dot(gPointDensityTex.SampleLevel(gSampler, uv, 0).rgb, float3(0.3333, 0.3333, 0.3333));
            maskSum += dot(gPointMaskTex.SampleLevel(gSampler, uv, 0).rgb, float3(0.3333, 0.3333, 0.3333));
            volumeSum += dot(gVolumeMaskTex.SampleLevel(gSampler, uv, 0).rgb, float3(0.3333, 0.3333, 0.3333));
        }
    }

    float invSampleCount = 1.0f / (sampleCount * sampleCount);

    float densityAvg = densitySum * invSampleCount;
    float maskAvg = maskSum * invSampleCount;
    float volumeAvg = volumeSum * invSampleCount;

    // --------------------
    // 判定
    // --------------------
    if (densityAvg < gParams.densityThreshold)
    {
        return;
    }
    if (maskAvg < gParams.maskThreshold)
    {
        return;
    }
    if (volumeAvg < gParams.volumeThreshold)
    {
        return;
    }

    // --------------------
    // 点の位置生成
    // --------------------
    float2 cellCenterUV = cellUVMin + cellSize * 0.5f;

    // 位置の微小ランダム（セル内）
    float2 possampleCountoise = rand3dTo2d(float3(cellId, float(gParams.seed + 1)));

    float2 uv = cellCenterUV + possampleCountoise * cellSize * 0.4f;

    float2 worldXZ = lerp(
        gParams.worldMinXZ,
        gParams.worldMaxXZ,
        uv
    );

    Point p;
    p.pos = float3(worldXZ.x, 0.f, worldXZ.y);
    p.volume = volumeAvg;

    gOutPoints.Append(p);
}
