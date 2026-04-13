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

/// engine
class Scene;

/// <summary>
/// レイトレーシング用メッシュエントリ
/// </summary>
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

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize();

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize();

    /// <summary>
    /// BLAS更新
    /// </summary>
    /// <param name="_device">デバイス</param>
    /// <param name="_commandList">コマンドリスト</param>
    /// <param name="_entries">メッシュエントリ</param>
    void UpdateBlases(
        ID3D12Device8* _device,
        ID3D12GraphicsCommandList6* _commandList,
        const std::vector<RaytracingMeshEntry>& _entries);

    /// <summary>
    /// TLAS更新
    /// </summary>
    /// <param name="_device">デバイス</param>
    /// <param name="_commandList">コマンドリスト</param>
    /// <param name="_instances">インスタンスデータ</param>
    void UpdateTlas(
        ID3D12Device8* _device,
        ID3D12GraphicsCommandList6* _commandList,
        const std::vector<RayTracingInstance>& _instances);

    /// <summary>
    /// シーンが空かどうか
    /// </summary>
    /// <returns>空ならtrue</returns>
    bool IsEmpty() const;

private:
    std::unordered_map<MeshHandle, BottomLevelAccelerationStructure> blasMap_; // BLAS (meshごとに1つ)
    TopLevelAccelerationStructure tlas_; // TLAS (tlasはシーンで1つ)
    bool tlasIsCreated_ = false;

public:
    /// <summary>
    /// TLASリソースの取得
    /// </summary>
    /// <returns>リソース</returns>
    ID3D12Resource* GetTlasResource() {
        return tlas_.GetResource();
    }

    /// <summary>
    /// 指定メッシュのBLAS取得
    /// </summary>
    /// <param name="_meshHandle">メッシュハンドル</param>
    /// <returns>BLAS</returns>
    BottomLevelAccelerationStructure* GetBlas(const MeshHandle& _meshHandle) {
        auto it = blasMap_.find(_meshHandle);
        if (it != blasMap_.end()) {
            return &it->second;
        }
        return nullptr;
    }
};

/// <summary>
/// 指定したエンティティのメッシュが動的かどうかを判定する.
/// </summary>
/// <param name="_entityHandle"></param>
/// <param name="_type"></param>
/// <param name="_isModelMesh"></param>
/// <returns></returns>
bool MeshIsDynamic(Scene* _scene, EntityHandle _entityHandle, RaytracingMeshType _type, bool _isModelMesh = false);

}
