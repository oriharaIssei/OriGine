#pragma once
#include "component/IComponent.h"

/// stl
#include <memory>
// container
#include <unordered_map>
#include <vector>
// utility
#include <concepts>

/// engine
// assets
#include "model/Model.h"
// directX12Object
#include "directX12/buffer/SimpleConstantBuffer.h"
#include "directX12/mesh/Mesh.h"
#include "directX12/mesh/MeshHandle.h"
#include "directX12/raytracing/RaytracingMeshType.h"
#include "directX12/ShaderManager.h"

// module
#include "asset/AssetSystem.h"

/// ECS
// component
#include "component/material/Material.h"
#include "component/transform/Transform.h"

namespace OriGine {

///< summary>
/// MeshRenderer
/// VertexType は Mesh.h で定義されている VertexType と同じものを指定する
///</summary>
template <typename MeshTenplate, typename VertexDataType>
concept IsDerivedMesh = std::derived_from<MeshTenplate, Mesh<VertexDataType>>;

/// <summary>
/// メッシュレンダラー基底クラス。
/// meshGroup_(shared_ptr&lt;vector&lt;MeshTemplate&gt;&gt;)としてCPU側の頂点/インデックスデータを保持しつつ、
/// 各Meshは内部でGPUリソースをMapしたまま保持している(Mesh側の実装)。
/// つまりCPU側の値を書き換えただけではGPUに反映されず、Mesh::TransferData()を明示的に呼んだ時点で初めて
/// Map済みのGPUメモリへコピーされる。shared_ptrで持つことで、コンストラクタで既存のメッシュ群を
/// そのまま(コピーせず)受け取って共有することもできる
/// </summary>
template <typename MeshTemplate, typename VertexDataType = MeshTemplate::VertexType>
    requires IsDerivedMesh<MeshTemplate, VertexDataType>
class MeshRenderer
    : public IComponent {
public:
    using VertexType = VertexDataType;
    using MeshType   = MeshTemplate;

    MeshRenderer() { meshGroup_ = std::make_shared<std::vector<MeshTemplate>>(); }
    MeshRenderer(const std::vector<MeshTemplate>& _meshGroup) {
        meshGroup_ = std::make_shared<std::vector<MeshTemplate>>(_meshGroup);
    }
    MeshRenderer(const std::shared_ptr<std::vector<MeshTemplate>>& _meshGroup) : meshGroup_(_meshGroup) {}

    virtual ~MeshRenderer() {}

    /// <summary>
    /// 初期化処理(派生クラスでオーバーライドされる想定の共通実装)
    /// </summary>
    /// <param name="_owner">このレンダラーを所有するエンティティ</param>
    virtual void Initialize(Scene* /*_scene*/, const EntityHandle& _owner) {
        hostEntityHandle_ = _owner;
    }

    // 基底クラスでは編集UIを持たないため既定は何もしない(派生クラスが必要に応じてオーバーライドする)
    void Edit(Scene* /*_scene*/, const EntityHandle& /*_owner*/, const std::string& /*_parentLabel*/) override {}

    /// <summary>
    /// 終了処理。保持している全メッシュのGPUリソースを解放してからmeshGroup_自体を手放す
    /// </summary>
    virtual void Finalize() {
        for (auto& mesh : *meshGroup_) {
            mesh.Finalize();
        }
        meshGroup_.reset();
    }

protected:
    EntityHandle hostEntityHandle_ = {}; // このレンダラーを所有するエンティティのハンドル

    BlendMode currentBlend_ = BlendMode::Alpha; // 描画時のブレンドモード

    bool isRender_  = true; // 描画を行うかどうか
    bool isCulling_ = true; // カリングを行うかどうか

    std::shared_ptr<std::vector<MeshTemplate>> meshGroup_; // 描画対象のメッシュ群
    std::vector<RaytracingMeshType> meshRaytracingTypes_; // 各メッシュのレイトレーシング種別

public: // ↓ Accessor
    EntityHandle GetHostEntityHandle() const {
        return hostEntityHandle_;
    }

    //------------------------------ BlendMode ------------------------------//
    BlendMode GetCurrentBlend() const {
        return currentBlend_;
    }
    void SetBlendMode(BlendMode _blend) {
        currentBlend_ = _blend;
    }

    //------------------------------ isRender ------------------------------//
    bool IsRender() const {
        return isRender_;
    }
    void SetIsRender(bool _isRender) {
        isRender_ = _isRender;
    }
    //------------------------------ isCulling ------------------------------//
    bool IsCulling() const {
        return isCulling_;
    }
    void SetIsCulling(bool _isCulling) {
        isCulling_ = _isCulling;
    }

    //------------------------------ MeshGroup ------------------------------//
    int32_t GetMeshGroupSize() const {
        return static_cast<int32_t>(meshGroup_->size());
    }
    const std::shared_ptr<std::vector<MeshTemplate>>& GetMeshGroup() const {
        return meshGroup_;
    }

    RaytracingMeshType GetMeshRaytracingType(int32_t _meshIndex) const {
        if (_meshIndex < 0 || static_cast<size_t>(_meshIndex) >= meshRaytracingTypes_.size()) {
            return RaytracingMeshType::Auto;
        }
        return meshRaytracingTypes_[_meshIndex];
    }

    /// <summary>
    /// 特定のメッシュを識別するためのハンドルを取得。
    /// このコンポーネント自身のComponentHandleとmeshGroup_内のインデックスを組み合わせることで、
    /// 「どのコンポーネントの何番目のメッシュか」を一意に表現する
    /// </summary>
    /// <param name="_meshIndex"></param>
    /// <returns></returns>
    MeshHandle GetMeshHandle(int32_t _meshIndex) const {
        MeshHandle handle{};
        handle.handle    = this->GetHandle();
        handle.meshIndex = static_cast<uint32_t>(_meshIndex);
        return handle;
    }
    /// <summary>
    /// メッシュグループ内のすべてのメッシュを識別するためのハンドルを取得
    /// </summary>
    /// <returns></returns>
    std::vector<MeshHandle> GetAllMeshHandles() const {
        std::vector<MeshHandle> handles;
        for (size_t i = 0; i < meshGroup_->size(); ++i) {
            MeshHandle handle{};
            handle.handle    = this->GetHandle();
            handle.meshIndex = static_cast<uint32_t>(i);
            handles.push_back(handle);
        }
        return handles;
    }

    void SetMeshGroup(const std::vector<MeshTemplate>& _meshGroup) {
        meshGroup_ = std::make_shared<std::vector<MeshTemplate>>(_meshGroup);
    }
    void PushBackMesh(const MeshTemplate& _mesh) {
        meshGroup_->push_back(_mesh);
    }
};

} // namespace OriGine
