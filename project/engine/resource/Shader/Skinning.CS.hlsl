
// numthreadsの引数は (x, y, z) と呼ばれている.
// いくつかの制約がある.
// 1.x,y,z が 正の整数(0もだめ) でなければならない.
// 2.それぞれの数字は コンパイル時定数 でなければならない.
// 3.z <= 64 でなければならない.
// 4. x * y * z <= 1024 でなければならない.(総要素数が 1024以下でなければならない)

/// ==========================================
// Structs
// ==========================================
struct Vertex
{
    float4 pos;
    float2 texcoord;
    float3 normal;
};
struct Well
{
    float4x4 skeletonSpaceMat;
    float4x4 skeletonSpaceInverseTransposeMat;
};
struct VertexInfluence
{
    float4 weights;
    int4 indecies;
};
struct SkinningInformation
{
    uint numVertices;
};

/// ==========================================
// Buffers
// ==========================================
RWStructuredBuffer<Vertex> outputVertices : register(u0);
StructuredBuffer<Vertex> gVertices : register(t0);
StructuredBuffer<Well> gMatrixPalette : register(t1);
StructuredBuffer<VertexInfluence> gVertexInfluences : register(t2);
ConstantBuffer<SkinningInformation> gSkinningInformation : register(b0);

// RWStructuredBuffer
/*
Unordered Access View (UAV)
書き込みも出来る Buffer .
*/


[numthreads(1024, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint vertexIndex = DTid.x;
    
    if (vertexIndex < gSkinningInformation.numVertices)
    {
        Vertex input = gVertices[vertexIndex];
        VertexInfluence influence = gVertexInfluences[vertexIndex];

        Vertex skinned;
        skinned.texcoord = input.texcoord;

        // 位置 の 変換
        skinned.pos = mul(input.pos, gMatrixPalette[influence.indecies.x].skeletonSpaceMat) * influence.weights.x;
        skinned.pos += mul(input.pos, gMatrixPalette[influence.indecies.y].skeletonSpaceMat) * influence.weights.y;
        skinned.pos += mul(input.pos, gMatrixPalette[influence.indecies.z].skeletonSpaceMat) * influence.weights.z;
        skinned.pos += mul(input.pos, gMatrixPalette[influence.indecies.w].skeletonSpaceMat) * influence.weights.w;
        skinned.pos.w = 1.f;

        // 法線 の 変換
        skinned.normal = mul(input.normal, (float3x3) gMatrixPalette[influence.indecies.x].skeletonSpaceInverseTransposeMat) * influence.weights.x;
        skinned.normal += mul(input.normal, (float3x3) gMatrixPalette[influence.indecies.y].skeletonSpaceInverseTransposeMat) * influence.weights.y;
        skinned.normal += mul(input.normal, (float3x3) gMatrixPalette[influence.indecies.z].skeletonSpaceInverseTransposeMat) * influence.weights.z;
        skinned.normal += mul(input.normal, (float3x3) gMatrixPalette[influence.indecies.w].skeletonSpaceInverseTransposeMat) * influence.weights.w;
        skinned.normal = normalize(skinned.normal);

        
        outputVertices[vertexIndex] = skinned;
    }
}
