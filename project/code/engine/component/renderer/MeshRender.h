#pragma once
#include "IRendererComponent.h"

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
#include "transform/Transform.h"
// directX12Object
#include "directX12/Mesh.h"
// module
#include "material/texture/TextureManager.h"

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
    : public IRendererComponent {
public:
    using VertexType = VertexDataType;

    MeshRenderer() {}
    MeshRenderer(const std::vector<MeshTemplate>& _meshGroup) {
        meshGroup_ = std::make_shared<std::vector<MeshTemplate>>(_meshGroup);
    }
    MeshRenderer(const std::shared_ptr<std::vector<MeshTemplate>>& _meshGroup) : meshGroup_(_meshGroup) {}

    virtual ~MeshRenderer() {}

    virtual void Init()   = 0;
    virtual void Update() = 0;
    virtual void Render() = 0;
    virtual void Finalize() {
        meshGroup_.reset();
    }

protected:
    BlendMode currentBlend_ = BlendMode::Alpha;

    std::shared_ptr<std::vector<MeshTemplate>> meshGroup_;

public: // ↓ Accessor
    //------------------------------ BlendMode ------------------------------//
    BlendMode getCurrentBlend() const {
        return currentBlend_;
    }
    void setBlendMode(BlendMode _blend) {
        currentBlend_ = _blend;
    }

    //------------------------------ MeshGroup ------------------------------//
    const std::shared_ptr<std::vector<MeshTemplate>>& getMeshGroup() const {
        return meshGroup_;
    }

    void setMeshGroup(const std::vector<MeshTemplate>& _meshGroup) {
        meshGroup_ = std::make_shared<std::vector<MeshTemplate>>(_meshGroup);
    }
};

///==================================================================================================================
/// ↓ DefaultMeshRenderer
///==================================================================================================================

//----------------------------------------- TextureMeshRenderer -----------------------------------------//
class TextureMeshRenderer
    : public MeshRenderer<TextureMesh, TextureVertexData> {
public:
    TextureMeshRenderer() {}
    TextureMeshRenderer(const std::vector<TextureMesh>& _meshGroup) : MeshRenderer<TextureMesh, TextureVertexData>(_meshGroup) {}
    TextureMeshRenderer(const std::shared_ptr<std::vector<TextureMesh>>& _meshGroup) : MeshRenderer<TextureMesh, TextureVertexData>(_meshGroup) {}

    ~TextureMeshRenderer() {}
    ///< summary>
    /// 初期化
    ///</summary>
    void Init() override;
    /// <summary>
    /// 更新
    /// </summary>
    void Update() override;
    /// <summary>
    /// 描画
    /// </summary>
    void Render() override;

private:
    std::vector<IConstantBuffer<Transform>> meshTransformBuff_;
    std::vector<IConstantBuffer<Material>*> meshMaterialBuff_;
    std::vector<uint32_t> meshTextureNumber_;

public:
    //------------------------------ Transform ------------------------------//
    const Transform& getTransform(int32_t _meshIndex) const {
        return meshTransformBuff_[_meshIndex].openData_;
    }
    void setTransform(int32_t _meshIndex, const Transform& _transform) {
        meshTransformBuff_[_meshIndex].openData_ = _transform;
    }
    const IConstantBuffer<Transform>& getTransformBuff(int32_t _meshIndex) const {
        return meshTransformBuff_[_meshIndex];
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

//----------------------------------------- PrimitiveMeshRenderer -----------------------------------------//
class PrimitiveMeshRenderer
    : public MeshRenderer<PrimitiveMesh, PrimitiveVertexData> {
public:
    PrimitiveMeshRenderer();
    PrimitiveMeshRenderer(const std::vector<PrimitiveMesh>& _meshGroup) : MeshRenderer<PrimitiveMesh, PrimitiveVertexData>(_meshGroup) {}
    PrimitiveMeshRenderer(const std::shared_ptr<std::vector<PrimitiveMesh>>& _meshGroup) : MeshRenderer<PrimitiveMesh, PrimitiveVertexData>(_meshGroup) {}

    ~PrimitiveMeshRenderer();

    ///< summary>
    /// 初期化
    ///</summary>
    void Init() override;
    /// <summary>
    /// 更新
    /// </summary>
    void Update() override;
    /// <summary>
    /// 描画
    /// </summary>
    void Render() override;

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

///==================================================================================================================
/// ↓ DefaultMeshRenderControllers
///==================================================================================================================

//----------------------------------------------------------------------------------------------------------
// ↓ PrimitiveMesh
//----------------------------------------------------------------------------------------------------------
class PrimitiveMeshRendererController
    : public RendererComponentController<PrimitiveMeshRenderer> {
public:
    void StartRender() override;

    void EndRender() override {}

    void CreatePso() override;
};

//----------------------------------------------------------------------------------------------------------
// ↓ Texture付きMesh
//----------------------------------------------------------------------------------------------------------

class TextureMeshRendererController
    : public RendererComponentController<TextureMeshRenderer> {
public:
    void StartRender() override;

    void EndRender() override {}

    void CreatePso() override;

public:
};
