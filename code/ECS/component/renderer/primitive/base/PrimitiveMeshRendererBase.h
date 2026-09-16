#pragma once

/// engine
// directX12
#include "component/renderer/MeshRenderer.h"
#include "directX12/instancing/InstanceHandle.h"

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

    ~PrimitiveMeshRendererBase() = default;

    // Initialize / Finalize / Edit / CreateMesh はここでは実装を持たない(旧: 純粋仮想)ため、
    // 宣言ごと派生側へ移した(Phase 3 3B)。
    // Initialize/Editは具象型(BoxRenderer等)、Finalize/CreateMeshはPrimitiveMeshRenderer<PrimType>が持つ。

    /// <summary>
    /// テクスチャを読み込む
    /// </summary>
    void LoadTexture(const std::string& _directory, const std::string& _filename);
    /// <summary>
    /// テクスチャを読み込む
    /// </summary>
    void LoadTexture(const std::string& _filePath);

protected:
    IConstantBuffer<Transform> transformBuff_; // 座標変換用定数バッファ
    int32_t materialIndex_ = -1; // 使用するマテリアルのインデックス
    SimpleConstantBuffer<Material> materialBuff_; // マテリアル用定数バッファ

    std::string textureFilePath_; // 読み込んだテクスチャのファイルパス
    size_t textureIndex_ = 0; // 読み込んだテクスチャのインデックス

    /// <summary>インスタンシング描画用ハンドル</summary>
    InstanceHandle instanceHandle_;
    /// <summary>インスタンシング描画を使用するかどうか（派生クラスで設定）</summary>
    bool useInstancing_ = false;

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

    size_t GetTextureIndex() const {
        return textureIndex_;
    }

    /// <summary>インスタンシング描画を使用するかどうか</summary>
    bool IsInstancing() const { return useInstancing_; }
    /// <summary>インスタンシング描画の有効/無効を設定する</summary>
    void SetInstancing(bool _useInstancing) { useInstancing_ = _useInstancing; }

    /// <summary>インスタンスハンドルを取得する</summary>
    const InstanceHandle& GetInstanceHandle() const { return instanceHandle_; }
    /// <summary>インスタンスハンドルを設定する</summary>
    void SetInstanceHandle(const InstanceHandle& _handle) { instanceHandle_ = _handle; }
};

} // namespace OriGine
