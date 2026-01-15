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
#include "texture/TextureManager.h"

/// ECS
// component
#include "component/material/Material.h"
#include "component/transform/Transform.h"

namespace OriGine {

///==================================================================================================================
/// ↓ MeshRenderer
///==================================================================================================================

///< summary>
/// MeshRenderer
/// VertexType は Mesh.h で定義されている VertexType と同じものを指定する
///</summary>
template <typename MeshTenplate, typename VertexDataType>
concept IsDerivedMesh = std::derived_from<MeshTenplate, Mesh<VertexDataType>>;

/// <summary>
/// メッシュレンダラー基底クラス
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

    virtual void Initialize(Scene* /*_scene*/, EntityHandle _owner) {
        hostEntityHandle_ = _owner;
    }

    void Edit(Scene* /*_scene*/, EntityHandle /*_owner*/, const std::string& /*_parentLabel*/) override {}

    virtual void Finalize() {
        for (auto& mesh : *meshGroup_) {
            mesh.Finalize();
        }
        meshGroup_.reset();
    }

protected:
    EntityHandle hostEntityHandle_ = {};

    BlendMode currentBlend_ = BlendMode::Alpha;

    bool isRender_  = true;
    bool isCulling_ = true;

    std::shared_ptr<std::vector<MeshTemplate>> meshGroup_;
    std::vector<RaytracingMeshType> meshRaytracingTypes_;

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
    /// 特定のメッシュを識別するためのハンドルを取得
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

///==================================================================================================================
/// ↓ DefaultMeshRenderer
///==================================================================================================================

//----------------------------------------- ModelMeshRenderer -----------------------------------------//

///< summary>
/// モデルのメッシュ描画コンポーネント
///</summary>
class ModelMeshRenderer
    : public MeshRenderer<TextureColorMesh, TextureColorVertexData> {
    friend void to_json(nlohmann::json& j, const ModelMeshRenderer& r);
    friend void from_json(const nlohmann::json& j, ModelMeshRenderer& r);

public:
    ModelMeshRenderer() {}
    ModelMeshRenderer(const std::vector<TextureColorMesh>& _meshGroup);
    ModelMeshRenderer(const std::shared_ptr<std::vector<TextureColorMesh>>& _meshGroup);

    ~ModelMeshRenderer() {}
    ///< summary>
    /// 初期化
    ///</summary>
    void Initialize(OriGine::Scene* _scene, EntityHandle _owner) override;

    void Edit(Scene* _scene, EntityHandle _entity, const std::string& _parentLabel) override;

    void Finalize() override {
        for (auto& mesh : *meshGroup_) {
            mesh.Finalize();
        }
        meshGroup_.reset();

        for (auto& transformBuff : meshTransformBuff_) {
            transformBuff.Finalize();
        }
    }

    void InitializeTransformBuffer();
    void InitializeMaterialBuffer();
    void InitializeMaterialBufferWithMaterialIndex();
    void ResizeTransformBuffer2MeshGroupSize() {
        meshTransformBuff_.resize(meshGroup_->size());
    }
    void ResizeMaterialBuffer2MeshGroupSize() {
        meshMaterialBuff_.resize(meshGroup_->size());
        meshTextureNumbers_.resize(meshGroup_->size(), 0);
        textureFilePath_.resize(meshGroup_->size(), "");
    }

private:
    std::string directory_ = "";
    std::string fileName_  = "";

    std::vector<IConstantBuffer<Transform>> meshTransformBuff_;
    // first = material handle, second = constantBuffer
    std::vector<std::pair<ComponentHandle, SimpleConstantBuffer<Material>>> meshMaterialBuff_;

    std::vector<std::string> textureFilePath_ = {};
    std::vector<uint32_t> meshTextureNumbers_;

public:
    //------------------------------ File ------------------------------//
    const std::string& GetDirectory() const {
        return directory_;
    }
    const std::string& GetFileName() const {
        return fileName_;
    }

    //------------------------------ Transform ------------------------------//
    const Transform& GetTransform(int32_t _meshIndex = 0) const {
        return meshTransformBuff_[_meshIndex].openData_;
    }
    Transform& GetTransform(int32_t _meshIndex = 0) {
        return meshTransformBuff_[_meshIndex].openData_;
    }

    const std::vector<IConstantBuffer<Transform>>& GetAllTransformBuff() const {
        return meshTransformBuff_;
    }
    std::vector<IConstantBuffer<Transform>>& GetAllTransformBuffRef() {
        return meshTransformBuff_;
    }

    const IConstantBuffer<Transform>& GetTransformBuff(int32_t _meshIndex = 0) const {
        return meshTransformBuff_[_meshIndex];
    }
    IConstantBuffer<Transform>& GetTransformBuff(int32_t _meshIndex = 0) {
        return meshTransformBuff_[_meshIndex];
    }
    void SetTransform(int32_t _meshIndex, const Transform& _transform) {
        meshTransformBuff_[_meshIndex].openData_ = _transform;
    }
    void pushBackTransformBuff() {
        meshTransformBuff_.emplace_back();
    }

    /// <summary>
    /// 親Transformを設定(すべてのメッシュに対して)
    /// </summary>
    /// <param name="_parent"></param>
    void SetParentTransform(Transform* _parent) {
        for (auto& transformBuff : meshTransformBuff_) {
            transformBuff.openData_.parent = _parent;
        }
    }

    //------------------------------ Material ------------------------------//
    const SimpleConstantBuffer<Material>& GetMaterialBuff(int32_t _meshIndex) const {
        return meshMaterialBuff_[_meshIndex].second;
    }
    SimpleConstantBuffer<Material>& GetMaterialBuff(int32_t _meshIndex) {
        return meshMaterialBuff_[_meshIndex].second;
    }
    ComponentHandle GetMaterialHandle(int32_t _meshIndex) const {
        return meshMaterialBuff_[_meshIndex].first;
    }
    void SetMaterialHandle(int32_t _meshIndex, ComponentHandle _materialHandle) {
        meshMaterialBuff_[_meshIndex].first = _materialHandle;
    }

    //------------------------------ TextureNumber ------------------------------//
    uint32_t GetTextureNumber(int32_t _meshIndex) const {
        return meshTextureNumbers_[_meshIndex];
    }
    void SetTexture(int32_t _meshIndex, const std::string& _filename) {
        textureFilePath_[_meshIndex] = _filename;
        if (_filename.empty()) {
            meshTextureNumbers_[_meshIndex] = 0;
            return;
        }
        meshTextureNumbers_[_meshIndex] = TextureManager::LoadTexture(textureFilePath_[_meshIndex]);
    }
};

void CreateModelMeshRenderer(
    ModelMeshRenderer* _renderer,
    EntityHandle _hostEntity,
    const std::string& _directory,
    const std::string& _fileName,
    bool _usingDefaultTexture = true);
void InitializeMaterialFromModelFile(
    ModelMeshRenderer* _renderer,
    Scene* _scene,
    EntityHandle _hostEntity,
    const std::string& _directory,
    const std::string& _fileName);

//----------------------------------------- LineRenderer -----------------------------------------//
///< summary>
/// ライン描画コンポーネント
///</summary>
class LineRenderer
    : public MeshRenderer<Mesh<ColorVertexData>, ColorVertexData> {
    friend void to_json(nlohmann::json& j, const LineRenderer& r);
    friend void from_json(const nlohmann::json& j, LineRenderer& r);

public:
    LineRenderer();
    LineRenderer(const std::vector<Mesh<ColorVertexData>>& _meshGroup);
    LineRenderer(const std::shared_ptr<std::vector<Mesh<ColorVertexData>>>& _meshGroup);
    ~LineRenderer();
    ///< summary>
    /// 初期化
    ///</summary>
    void Initialize(Scene* _scene, EntityHandle _owner) override;

    void Edit(Scene* _scene, EntityHandle _entity, const std::string& _parentLabel) override;
    void Finalize() override;

private:
    IConstantBuffer<Transform> transformBuff_;
    bool lineIsStrip_ = false;

public:
    //------------------------------ Transform ------------------------------//
    const Transform& GetTransform() const {
        return transformBuff_.openData_;
    }
    void SetTransform(const Transform& _transform) {
        transformBuff_.openData_ = _transform;
    }
    /// <summary>
    /// 親Transformを設定
    /// </summary>
    /// <param name="_parent"></param>
    void SetParentTransform(Transform* _parent) {
        transformBuff_.openData_.parent = _parent;
    }
    IConstantBuffer<Transform>& GetTransformBuff() {
        return transformBuff_;
    }

    //------------------------------ LineIsStrip ------------------------------//
    bool isLineStrip() const {
        return lineIsStrip_;
    }
    void SetLineStrip(bool _isStrip) {
        lineIsStrip_ = _isStrip;
    }
};

} // namespace OriGine
