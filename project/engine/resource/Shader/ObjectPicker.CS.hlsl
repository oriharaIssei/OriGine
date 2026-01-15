// -------------------------------------------------------------------------
// ScreenSpaceRayPicker.hlsl
// 画面全体(または矩形領域)に対して並列でRayを飛ばし、IDバッファを生成する
// -------------------------------------------------------------------------

// --- 定義 ---
// 無効なID
#define INVALID_INDEX -1

// --- リソースバインディング ---

// シーンの加速構造体 (TLAS)
RaytracingAccelerationStructure gAccelStruct : register(t0);

struct OutputObjectId
{
    int outputObjectId;
    int3 pad;
    
};
// 出力先: IDを格納する2Dテクスチャ (R32_SINT などを想定)
RWStructuredBuffer<OutputObjectId> gOutputID : register(u0);

// 定数バッファ
struct SceneData
{
    float3 rayOrigin;
    float cameraNear;
    float3 rayDirection;
    float cameraFar;
};
ConstantBuffer<SceneData> gSceneData : register(b0);

[numthreads(1, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    // デフォルト値書き込み
    gOutputID[1].outputObjectId = INVALID_INDEX;
    
    // 5. RayQuery の設定
    RayDesc ray;
    ray.Origin = gSceneData.rayOrigin;
    ray.Direction = gSceneData.rayDirection;
    ray.TMin = gSceneData.cameraNear;
    ray.TMax = gSceneData.cameraFar;
    
    // Inline Ray Tracing
    RayQuery < RAY_FLAG_NONE > q;
    q.TraceRayInline(
        gAccelStruct,
        RAY_FLAG_NONE,
        0xFF,
        ray
    );
    
    // ヒット判定ループ
    while (q.Proceed())
    {
    }


    // 6. 結果の書き込み
    if (q.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
    {
        // ヒットした箇所のテクスチャピクセルにIDを書き込む
        gOutputID[0].outputObjectId = q.CommittedInstanceID();
    }
}
