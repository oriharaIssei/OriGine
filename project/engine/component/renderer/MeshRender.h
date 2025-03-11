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
// conmponent
#include "../material/Material.h"
#include "../transform/Transform.h"
// directX12Object
#include "directX12/Mesh.h"
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

template <typename MeshTemplate, typename VertexDataType>
    requires IsDerivedMesh<MeshTemplate, VertexDataType>
class MeshRenderer
    : public IComponent {
public:
    using VertexType = VertexDataType;

    MeshRenderer(GameEntity* _hostEntity) : IComponent(_hostEntity) { meshGroup_ = std::make_shared<std::vector<MeshTemplate>>(); }
    MeshRenderer(GameEntity* _hostEntity, const std::vector<MeshTemplate>& _meshGroup) : IComponent(_hostEntity) {
        meshGroup_ = std::make_shared<std::vector<MeshTemplate>>(_meshGroup);
    }
    MeshRenderer(GameEntity* _hostEntity, const std::shared_ptr<std::vector<MeshTemplate>>& _meshGroup) : IComponent(_hostEntity), meshGroup_(_meshGroup) {}

    virtual ~MeshRenderer() {}

    virtual void Init() = 0;

    bool Edit() override { return false; }
    void Save(BinaryWriter& _writer) override {
        _writer.Write<int32_t>(static_cast<int32_t>(currentBlend_));

        _writer.Write<bool>(isRender_);
    }
    void Load(BinaryReader& _reader) override {
        int32_t blend = 0;
        _reader.Read<int32_t>(blend);
        currentBlend_ = static_cast<BlendMode>(blend);

        _reader.Read<bool>(isRender_);
    }

    virtual void Finalize() {
        for (auto& mesh : *meshGroup_) {
            mesh.Finalize();
        }
        meshGroup_.reset();
    }

protected:
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
public:
    ModelMeshRenderer(GameEntity* _hostEntity) : MeshRenderer<TextureMesh, TextureVertexData>(_hostEntity) {}
    ModelMeshRenderer(GameEntity* _hostEntity, const std::vector<TextureMesh>& _meshGroup);
    ModelMeshRenderer(GameEntity* _hostEntity, const std::shared_ptr<std::vector<TextureMesh>>& _meshGroup);

    ~ModelMeshRenderer() {}
    ///< summary>
    /// 初期化
    ///</summary>
    void Init() override;

    bool Edit() override;

    void Save(BinaryWriter& _writer) override;
    void Load(BinaryReader& _reader) override;

    void Finalize() override {
        for (auto& mesh : *meshGroup_) {
            mesh.Finalize();
        }
        meshGroup_.reset();

        for (auto& transformBuff : meshTransformBuff_) {
            transformBuff.Finalize();
        }
    }

private:
    std::string directory_;
    std::string fileName_;

    std::vector<IConstantBuffer<Transform>> meshTransformBuff_;
    std::vector<IConstantBuffer<Material>*> meshMaterialBuff_;
    std::vector<uint32_t> meshTextureNumber_;

public:
    int32_t getMeshSize() const {
        return static_cast<int32_t>(meshGroup_->size());
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
    void setMaterialBuff(int32_t _meshIndex, IConstantBuffer<Material>* _buff) {
        meshMaterialBuff_[_meshIndex] = _buff;
    }
    IConstantBuffer<Material>* getMaterialBuff(int32_t _meshIndex) const {
        return meshMaterialBuff_[_meshIndex];
    }

    //------------------------------ TextureNumber ------------------------------//
    uint32_t getTextureNumber(int32_t _meshIndex) const {
        return meshTextureNumber_[_meshIndex];
    }
    void setTextureNumber(int32_t _meshIndex, uint32_t _textureNumber) {
        meshTextureNumber_[_meshIndex] = _textureNumber;
    }
    void setTexture(int32_t _meshIndex, const std::string& _filename) {
        meshTextureNumber_[_meshIndex] = TextureManager::LoadTexture(_filename);
    }
};

void CreateModelMeshRenderer(ModelMeshRenderer* _renderer, GameEntity* _hostEntity, const std::string& _directory, const std::string& _filenName);

//----------------------------------------- PrimitiveMeshRenderer -----------------------------------------//
class PrimitiveMeshRenderer
    : public MeshRenderer<PrimitiveMesh, PrimitiveVertexData> {
public:
    PrimitiveMeshRenderer(GameEntity* _hostEntity);
    PrimitiveMeshRenderer(GameEntity* _hostEntity, const std::vector<PrimitiveMesh>& _meshGroup);
    PrimitiveMeshRenderer(GameEntity* _hostEntity, const std::shared_ptr<std::vector<PrimitiveMesh>>& _meshGroup);

    ~PrimitiveMeshRenderer();

    ///< summary>
    /// 初期化
    ///</summary>
    void Init() override;

private:
    std::vector<IConstantBuffer<Transform>> meshTransformBuff_;
    std::vector<IConstantBuffer<Material>*> meshMaterialBuff_;

public:
    //------------------------------ Transform ------------------------------//
    const Transform& getTransform(int32_t _meshIndex) const {
        return meshTransformBuff_[_meshIndex].openData_;
    }
    void setTransform(int32_t _meshIndex, const Transform& _transform) {
        meshTransformBuff_[_meshIndex].openData_ = _transform;
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
    void setMaterialBuff(int32_t _meshIndex, IConstantBuffer<Material>* _buff) {
        meshMaterialBuff_[_meshIndex] = _buff;
    }
    IConstantBuffer<Material>* getMaterialBuff(int32_t _meshIndex) const {
        return meshMaterialBuff_[_meshIndex];
    }
};
