#pragma once
#include "component/IComponent.h"

#include "module/IModule.h"

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
// component
#include "component/material/Material.h"
#include "component/transform/Transform.h"
// directX12Object
#include "directX12/Mesh.h"
#include "directX12/ShaderManager.h"
// module
#include "texture/TextureManager.h"

///==================================================================================================================
/// ↓ MeshRenderer
///==================================================================================================================

///< summary>
/// MeshRenderer
/// VertexType は Mesh.h で定義されている VertexType と同じものを指定する
///</summary>
template <typename MeshTenplate, typename VertexDataType>
concept IsDerivedMesh = std::derived_from<MeshTenplate, Mesh<VertexDataType>>;

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

    virtual void Initialize(GameEntity* _hostEntity) {
        hostEntity_ = _hostEntity;
    }

    bool Edit() override { return false; }

    virtual void Finalize() {
        for (auto& mesh : *meshGroup_) {
            mesh.Finalize();
        }
        meshGroup_.reset();
    }

protected:
    GameEntity* hostEntity_ = nullptr;

    BlendMode currentBlend_ = BlendMode::Alpha;

    bool isRender_ = true;

    std::shared_ptr<std::vector<MeshTemplate>> meshGroup_;

public: // ↓ Accessor
    //------------------------------ BlendMode ------------------------------//
    BlendMode getCurrentBlend() const {
        return currentBlend_;
    }
    void setBlendMode(BlendMode _blend) {
        currentBlend_ = _blend;
    }

    //------------------------------ isRender ------------------------------//
    bool isRender() const {
        return isRender_;
    }
    void setIsRender(bool _isRender) {
        isRender_ = _isRender;
    }
    //------------------------------ MeshGroup ------------------------------//
    int32_t getMeshGroupSize() const {
        return static_cast<int32_t>(meshGroup_->size());
    }
    const std::shared_ptr<std::vector<MeshTemplate>>& getMeshGroup() const {
        return meshGroup_;
    }

    void setMeshGroup(const std::vector<MeshTemplate>& _meshGroup) {
        meshGroup_ = std::make_shared<std::vector<MeshTemplate>>(_meshGroup);
    }
    void pushBackMesh(const MeshTemplate& _mesh) {
        meshGroup_->push_back(_mesh);
    }
};

///==================================================================================================================
/// ↓ DefaultMeshRenderer
///==================================================================================================================

//----------------------------------------- ModelMeshRenderer -----------------------------------------//

class ModelMeshRenderer
    : public MeshRenderer<TextureMesh, TextureVertexData> {
    friend void to_json(nlohmann::json& j, const ModelMeshRenderer& r);
    friend void from_json(const nlohmann::json& j, ModelMeshRenderer& r);

public:
    ModelMeshRenderer() {}
    ModelMeshRenderer(const std::vector<TextureMesh>& _meshGroup);
    ModelMeshRenderer(const std::shared_ptr<std::vector<TextureMesh>>& _meshGroup);

    ~ModelMeshRenderer() {}
    ///< summary>
    /// 初期化
    ///</summary>
    void Initialize(GameEntity* _hostEntity) override;

    bool Edit() override;

    void Finalize() override {
        for (auto& mesh : *meshGroup_) {
            mesh.Finalize();
        }
        meshGroup_.reset();

        for (auto& transformBuff : meshTransformBuff_) {
            transformBuff.Finalize();
        }
    }

    void InitializeTransformBuffer(GameEntity* _hostEntity);
    void InitializeMaterialBuffer(GameEntity* _hostEntity);
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
    std::vector<IConstantBuffer<Material>> meshMaterialBuff_;

    std::vector<std::string> textureFilePath_ = {};
    std::vector<uint32_t> meshTextureNumbers_;

public:
    //------------------------------ File ------------------------------//
    const std::string& getDirectory() const {
        return directory_;
    }
    const std::string& getFileName() const {
        return fileName_;
    }

    //------------------------------ Transform ------------------------------//
    const Transform& getTransform(int32_t _meshIndex = 0) const {
        return meshTransformBuff_[_meshIndex].openData_;
    }
    Transform& getTransform(int32_t _meshIndex = 0) {
        return meshTransformBuff_[_meshIndex].openData_;
    }
    const IConstantBuffer<Transform>& getTransformBuff(int32_t _meshIndex = 0) const {
        return meshTransformBuff_[_meshIndex];
    }
    IConstantBuffer<Transform>& getTransformBuff(int32_t _meshIndex = 0) {
        return meshTransformBuff_[_meshIndex];
    }
    void setTransform(int32_t _meshIndex, const Transform& _transform) {
        meshTransformBuff_[_meshIndex].openData_ = _transform;
    }
    void pushBackTransformBuff() {
        meshTransformBuff_.emplace_back();
    }

    /// <summary>
    /// 親Transformを設定(すべてのメッシュに対して)
    /// </summary>
    /// <param name="_parent"></param>
    void setParentTransform(Transform* _parent) {
        for (auto& transformBuff : meshTransformBuff_) {
            transformBuff.openData_.parent = _parent;
        }
    }

    //------------------------------ Material ------------------------------//
    void setMaterialBuff(int32_t _meshIndex, Material _data) {
        meshMaterialBuff_[_meshIndex].openData_ = _data;
    }
    const IConstantBuffer<Material>& getMaterialBuff(int32_t _meshIndex) const {
        return meshMaterialBuff_[_meshIndex];
    }
    IConstantBuffer<Material>& getMaterialBuff(int32_t _meshIndex) {
        return meshMaterialBuff_[_meshIndex];
    }

    //------------------------------ TextureNumber ------------------------------//
    uint32_t getTextureNumber(int32_t _meshIndex) const {
        return meshTextureNumbers_[_meshIndex];
    }
    void setTexture(int32_t _meshIndex, const std::string& _filename) {
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
    GameEntity* _hostEntity,
    const std::string& _directory,
    const std::string& _filenName,
    bool _usingDefaultMaterial = true,
    bool _usingDefaultTexture  = true);

//----------------------------------------- LineRenderer -----------------------------------------//
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
    void Initialize(GameEntity* _hostEntity) override;

    bool Edit() override;
    void Finalize() override;

private:
    IConstantBuffer<Transform> transformBuff_;
    bool lineIsStrip_ = false;

public:
    //------------------------------ Transform ------------------------------//
    const Transform& getTransform() const {
        return transformBuff_.openData_;
    }
    void setTransform(const Transform& _transform) {
        transformBuff_.openData_ = _transform;
    }
    /// <summary>
    /// 親Transformを設定
    /// </summary>
    /// <param name="_parent"></param>
    void setParentTransform(Transform* _parent) {
        transformBuff_.openData_.parent = _parent;
    }
    IConstantBuffer<Transform>& getTransformBuff() {
        return transformBuff_;
    }

    //------------------------------ LineIsStrip ------------------------------//
    bool isLineStrip() const {
        return lineIsStrip_;
    }
    void setLineStrip(bool _isStrip) {
        lineIsStrip_ = _isStrip;
    }
};
