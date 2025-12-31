#pragma once

/// stl
#include <unordered_map>

/// engine
// directX12
#include "directX12/raytracing/BottomLevelAccelerationStructure.h"
#include "directX12/raytracing/TopLevelAccelerationStructure.h"

/// ECS
// component
#include "component/ComponentHandle.h"
#include "component/renderer/MeshRenderer.h"

namespace OriGine {

struct RaytracingMeshEntry {
    MeshHandle meshHandle;
    TextureColorMesh* mesh;
    bool isDynamic     = false;
    Matrix4x4 worldMat = MakeMatrix4x4::Identity();
};

/// <summary>
/// レイトレーシングで使用するシーン情報(BLAS,TLASなど)の管理クラス
/// </summary>
class RaytracingScene {
public:
    RaytracingScene()  = default;
    ~RaytracingScene() = default;

    void Initialize();
    void Finalize();

    /// <summary>
    /// BLAS更新
    /// </summary>
    /// <param name="_device"></param>
    /// <param name="_commandList"></param>
    /// <param name="_meshes">TexturedMeshRenderSystemから取得(Dispatch時に選別する)</param>
    void UpdateBlases(
        ID3D12Device8* _device,
        ID3D12GraphicsCommandList6* _commandList,
        const std::vector<RaytracingMeshEntry>& _entries);

    void UpdateTlas(
        ID3D12Device8* _device,
        ID3D12GraphicsCommandList6* _commandList,
        const std::vector<RayTracingInstance>& _instances);

    /// <summary>
    ///
    /// </summary>
    /// <returns></returns>
    bool IsEmpty() const;

private:
    std::unordered_map<MeshHandle, BottomLevelAccelerationStructure> blasMap_; // BLAS (meshごとに1つ)
    TopLevelAccelerationStructure tlas_; // TLAS (tlasはシーンで1つ)
    bool tlasIsCreated_ = false;

public:
    ID3D12Resource* GetTlasResource() {
        return tlas_.GetResource();
    }
    BottomLevelAccelerationStructure* GetBlas(const MeshHandle& _meshHandle) {
        auto it = blasMap_.find(_meshHandle);
        if (it != blasMap_.end()) {
            return &it->second;
        }
        return nullptr;
    }
};

}
