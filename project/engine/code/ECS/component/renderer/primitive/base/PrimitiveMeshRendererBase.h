#pragma once

/// engine
// directX12
#include "component/renderer/MeshRenderer.h"

/// <summary>
/// PrimitiveRendererをポリモーフィズムで扱うための基底クラス
/// </summary>
class PrimitiveMeshRendererBase
    : public MeshRenderer<TextureMesh, TextureVertexData> {
public:
    PrimitiveMeshRendererBase() : MeshRenderer() {}
    PrimitiveMeshRendererBase(const std::vector<TextureMesh>& _meshGroup) : MeshRenderer(_meshGroup) {}
    PrimitiveMeshRendererBase(const std::shared_ptr<std::vector<TextureMesh>>& _meshGroup) : MeshRenderer(_meshGroup) {}

    virtual ~PrimitiveMeshRendererBase()                                                                = default;
    virtual void Initialize(Entity* _entity)                                                            = 0;
    virtual void Finalize()                                                                             = 0;
    virtual void Edit(Scene* _scene, Entity* _entity, [[maybe_unused]] const std::string& _parentLabel) = 0;

    /// <summary>
    /// 自身のプリミティブ情報をもとにメッシュを作成する
    /// </summary>
    /// <param name="_mesh"></param>
    virtual void CreateMesh(TextureMesh* _mesh) = 0;

    /// <summary>
    /// テクスチャを読み込む
    /// </summary>
    void LoadTexture(const std::string& _directory, const std::string& _filename);

protected:
    IConstantBuffer<Transform> transformBuff_;
    int32_t materialIndex_ = -1;
    SimpleConstantBuffer<Material> materialBuff_;

    std::string textureDirectory_;
    std::string textureFileName_;
    uint32_t textureIndex_ = 0;

public:
    Transform& getTransform() {
        return transformBuff_.openData_;
    }
    void setTransform(const Transform& _transform) {
        transformBuff_.openData_ = _transform;
    }
    int32_t getMaterialIndex() const {
        return materialIndex_;
    }
    void setMaterialIndex(int32_t _index) {
        materialIndex_ = _index;
    }

    const IConstantBuffer<Transform>& getTransformBuff() const {
        return transformBuff_;
    }
    IConstantBuffer<Transform>& getTransformBuff() {
        return transformBuff_;
    }
    const SimpleConstantBuffer<Material>& getMaterialBuff() const {
        return materialBuff_;
    }
    SimpleConstantBuffer<Material>& getMaterialBuff() {
        return materialBuff_;
    }

    const std::string& getTextureDirectory() const {
        return textureDirectory_;
    }
    const std::string& getTextureFileName() const {
        return textureFileName_;
    }

    uint32_t getTextureIndex() const {
        return textureIndex_;
    }
};
