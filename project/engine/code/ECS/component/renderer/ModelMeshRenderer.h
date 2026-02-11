#pragma once

#include "MeshRenderer.h"

/// engine
// asset
#include "asset/TextureAsset.h"

namespace OriGine {

///< summary>
/// モデルのメッシュ描画コンポーネント
///</summary>
class ModelMeshRenderer
    : public MeshRenderer<TextureColorMesh, TextureColorVertexData> {
    friend void to_json(nlohmann::json& _j, const ModelMeshRenderer& _comp);
    friend void from_json(const nlohmann::json& _j, ModelMeshRenderer& _comp);

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
    std::vector<size_t> meshTextureNumbers_;

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

    //------------------------------ textureIndex------------------------------//
    size_t GetTextureIndex(int32_t _meshIndex) const {
        return meshTextureNumbers_[_meshIndex];
    }
    void SetTexture(int32_t _meshIndex, const std::string& _filename) {
        textureFilePath_[_meshIndex] = _filename;
        if (_filename.empty()) {
            meshTextureNumbers_[_meshIndex] = 0;
            return;
        }
        meshTextureNumbers_[_meshIndex] = AssetSystem::GetInstance()->GetManager<TextureAsset>()->LoadAsset(textureFilePath_[_meshIndex]);
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

} // namespace OriGine
