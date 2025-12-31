
RaytracingAccelerationStructure gRaytracingScene : register(t5);

///========================================================================================================================
/// Shadow Utility
///========================================================================================================================

/// 影ができるかどうかを判定する
// _worldPos: ワールド空間の位置
// _mesh4LighthDirection: ライト方向（正規化済み）
// _rayMin: レイの最小距離
// _rayMax: レイの最大距離
bool TraceShadowRay(float3 _worldPos, float3 _mesh4LighthDirection, float _rayMin, float _rayMax)
{
	// rayDesc
    RayDesc rayDesc;
    rayDesc.Origin = _worldPos;
    rayDesc.Direction = _mesh4LighthDirection;
    rayDesc.TMin = _rayMin;
    rayDesc.TMax = _rayMax;
		
	// createRayQueryObject
    RayQuery < 0 > rayQuery;
	// executeRay
    rayQuery.TraceRayInline(gRaytracingScene, 0, 0xFE, rayDesc);
	
    while (rayQuery.Proceed())
    {
    }
    return rayQuery.CommittedStatus() == COMMITTED_TRIANGLE_HIT;
}

/// 円錐内の方向をサンプリングする
// _axis: 中心方向（正規化済み）
// _xi: [0,1)^2
// _cosThetaMax: cos(最大角)
float3 SampleCone(
    float3 axis, // 中心方向（正規化済み）
    float2 xi, // [0,1)^2
    float cosThetaMax)  // cos(最大角)
{
    float cosTheta = lerp(cosThetaMax, 1.0f, xi.x);
    float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
    float phi = 2.0f * 3.14159265f * xi.y;

    // ローカル空間
    float3 localDir = float3(
        cos(phi) * sinTheta,
        sin(phi) * sinTheta,
        cosTheta
    );

    // axis を Z に合わせた直交基底を作る
    float3 up = abs(axis.z) < 0.999f ? float3(0, 0, 1) : float3(1, 0, 0);
    float3 tangent = normalize(cross(up, axis));
    float3 bitangent = cross(axis, tangent);

    // ワールド方向へ
    return
        tangent * localDir.x +
        bitangent * localDir.y +
        axis * localDir.z;
}

/// Radical Inverse（Van der Corput）
float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16) | (bits >> 16);
    bits = ((bits & 0x55555555u) << 1) | ((bits & 0xAAAAAAAAu) >> 1);
    bits = ((bits & 0x33333333u) << 2) | ((bits & 0xCCCCCCCCu) >> 2);
    bits = ((bits & 0x0F0F0F0Fu) << 4) | ((bits & 0xF0F0F0F0u) >> 4);
    bits = ((bits & 0x00FF00FFu) << 8) | ((bits & 0xFF00FF00u) >> 8);
    return float(bits) * 2.3283064365386963e-10f; // / 2^32
}
/// Hammersley 点列
float2 Hammersley(uint i, uint N)
{
    return float2(float(i) / float(N), RadicalInverse_VdC(i));
}


static const uint SHADOW_SAMPLE_COUNT = 4;
float TraceShadowSoft(
    float3 origin,
    float3 lightDir,
    float maxDist,
    float cosThetaMax)
{
    float shadow = 0.0f;
    
    [unroll]
    for (int i = 0; i < SHADOW_SAMPLE_COUNT; ++i)
    {
        // サンプリング方向を決定
        float2 xi = Hammersley(i, SHADOW_SAMPLE_COUNT);
        // 円錐内の方向をサンプリング
        float3 dir = SampleCone(lightDir, xi, cosThetaMax);
        // 影判定
        shadow += TraceShadowRay(origin, dir, 0.001f, maxDist) ? 0.0f : 1.0f;
    }

    return shadow / SHADOW_SAMPLE_COUNT;
}
