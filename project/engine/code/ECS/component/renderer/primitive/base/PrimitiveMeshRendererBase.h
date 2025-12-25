#pragma once

/// engine
// directX12
#include "component/renderer/MeshRenderer.h"

namespace OriGine {

/// <summary>
/// PrimitiveRendererをポリモーフィズムで扱うための基底クラス
/// </summary>
class PrimitiveMeshRendererBase
    : public MeshRenderer<TextureColorMesh, TextureColorVertexData> {
public:
    PrimitiveMeshRendererBase() : MeshRenderer() {}
    PrimitiveMeshRendererBase(const std::vector<TextureColorMesh>& _meshGroup) : MeshRenderer(_meshGroup) {}
    PrimitiveMeshRendererBase(const std::shared_ptr<std::vector<TextureColorMesh>>& _meshGroup) : MeshRenderer(_meshGroup) {}

    virtual ~PrimitiveMeshRendererBase()                                               = default;
    virtual void Initialize(Scene* _scene, EntityHandle _entity)                                           = 0;
    virtual void Finalize()                                                            = 0;
    virtual void Edit(Scene* _scene, EntityHandle _handle, const std::string& _parentLabel) = 0;

    /// <summary>
    /// 自身のプリミティブ情報をもとにメッシュを作成する
    /// </summary>
    /// <param name="_mesh"></param>
    virtual void CreateMesh(TextureColorMesh* _mesh) = 0;

    /// <summary>
    /// テクスチャを読み込む
    /// </summary>
    void LoadTexture(const std::string& _directory, const std::string& _filename);
    /// <summary>
    /// テクスチャを読み込む
    /// </summary>
    void LoadTexture(const std::string& _filePath);

protected:
    IConstantBuffer<Transform> transformBuff_;
    int32_t materialIndex_ = -1;
    SimpleConstantBuffer<Material> materialBuff_;

    std::string textureFilePath_;
    uint32_t textureIndex_ = 0;

public:
    Transform& GetTransform() {
        return transformBuff_.openData_;
    }
    void SetTransform(const Transform& _transform) {
        transformBuff_.openData_ = _transform;
    }
    int32_t GetMaterialIndex() const {
        return materialIndex_;
    }
    void SetMaterialIndex(int32_t _index) {
        materialIndex_ = _index;
    }

    const IConstantBuffer<Transform>& GetTransformBuff() const {
        return transformBuff_;
    }
    IConstantBuffer<Transform>& GetTransformBuff() {
        return transformBuff_;
    }
    const SimpleConstantBuffer<Material>& GetMaterialBuff() const {
        return materialBuff_;
    }
    SimpleConstantBuffer<Material>& GetMaterialBuff() {
        return materialBuff_;
    }

    const std::string& GetTexturePath() const {
        return textureFilePath_;
    }

    uint32_t GetTextureIndex() const {
        return textureIndex_;
    }
};

} // namespace OriGine
