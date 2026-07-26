#include "RaytracingScene.h"

/// engine
#include "scene/Scene.h"

/// ECS
// component
#include "component/animation/SkinningAnimationComponent.h"
#include "component/renderer/ModelMeshRenderer.h"

using namespace OriGine;

/// <summary>
/// 初期化。BLASマップを空にし、TLASを初期化する。
/// </summary>
void RaytracingScene::Initialize() {
    if (!blasMap_.empty()) {
        blasMap_.clear();
    }
    tlas_.Initialize();
}

/// <summary>
/// 終了処理。全メッシュ分のBLASとTLASを解放する。
/// </summary>
void RaytracingScene::Finalize() {
    if (!blasMap_.empty()) {
        for (auto& [meshHandle, blas] : blasMap_) {
            blas.Finalize();
        }
        blasMap_.clear();
    }
    tlas_.Finalize();
}

/// <summary>
/// BLAS (Bottom Level Acceleration Structure) を更新する。
/// </summary>
/// <remarks>
/// BLASはメッシュ単体（頂点・インデックスバッファ）が持つ三角形群の空間構造で、メッシュ1つにつき1つ持つ。
/// 同じメッシュを使い回すインスタンスが複数あっても、頂点データそのものが変化しない限りBLASは共有できるため、
/// メッシュハンドル単位でキャッシュ(blasMap_)しておき、初回のみ構築し以降は更新のみを行う。
/// スキニングアニメーション等で頂点位置が変化するメッシュ(isDynamic)は毎フレームUpdateが必要になる。
/// </remarks>
void RaytracingScene::UpdateBlases(
    ID3D12Device8* _device,
    ID3D12GraphicsCommandList6* _commandList,
    const std::vector<RaytracingMeshEntry>& _entries) {
    for (const auto& entry : _entries) {
        auto it = blasMap_.find(entry.meshHandle);
        if (it == blasMap_.end()) {
            // BLAS未作成なら作成
            BottomLevelAccelerationStructure blas;
            blas.CreateResource(
                _device,
                _commandList,
                entry.mesh->GetVertexBuffer().GetResource()->GetGPUVirtualAddress(),
                entry.mesh->GetVertexSize(),
                entry.mesh->GetIndexBuffer().GetResource()->GetGPUVirtualAddress(),
                entry.mesh->GetIndexSize(),
                entry.isDynamic);
            blasMap_.emplace(entry.meshHandle, std::move(blas));
        } else {
            // BLAS更新
            it->second.Update(_commandList);
        }
    }
}

/// <summary>
/// TLAS (Top Level Acceleration Structure) を更新する。
/// </summary>
/// <remarks>
/// TLASは「どのBLAS(メッシュ)を、どのワールド変換行列で、シーン上のどこに配置するか」をまとめた
/// シーン全体で1つの構造体で、各BLASへの参照とインスタンスごとの変換行列を保持する。
/// レイトレーシングはこのTLASを起点にBLAS階層をたどって交差判定を行うため、
/// オブジェクトの位置やBLASの中身が変わるたびに再構築・更新する必要がある。
/// 初回のみCreateResource(フル構築)し、以降は差分更新(Update)で済ませることでコストを抑える。
/// </remarks>
void RaytracingScene::UpdateTlas(ID3D12Device8* _device, ID3D12GraphicsCommandList6* _commandList, const std::vector<RayTracingInstance>& _instances) {
    if (_instances.empty()) {
        return;
    }

    if (!tlasIsCreated_) {
        tlas_.CreateResource(_device, _commandList, _instances, true);
        tlasIsCreated_ = true;
    } else {
        tlas_.Update(_device, _commandList, _instances);
    }
}

/// <summary>
/// TLASが未構築、またはBLASが1つも無ければ「シーンが空」とみなす。
/// </summary>
bool OriGine::RaytracingScene::IsEmpty() const {
    return !tlasIsCreated_ || blasMap_.empty();
}

/// <summary>
/// 指定したエンティティのメッシュをBLASの動的更新対象として扱うべきか判定する。
/// </summary>
bool OriGine::MeshIsDynamic(Scene* _scene, const EntityHandle& _entityHandle, RaytracingMeshType _type, bool _isModelMesh) {
    if (_type == RaytracingMeshType::Dynamic) {
        return true;
    }
    if (_type == RaytracingMeshType::Static) {
        return false;
    }
    // Auto 判定
    if (_isModelMesh) {
        auto* modelComp = _scene->GetComponent<ModelMeshRenderer>(_entityHandle);
        if (modelComp) {
            // スキニングアニメーションを持つモデルは毎フレーム頂点位置が変化するため動的として扱う
            auto* skinningComp = _scene->GetComponent<SkinningAnimationComponent>(_entityHandle);
            if (skinningComp) {
                return true;
            }
        }
    }

    return false;
}
