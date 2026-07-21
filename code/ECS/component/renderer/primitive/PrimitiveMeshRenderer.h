#pragma once

/// base
#include "component/renderer/primitive/base/IPrimitive.h"
#include "component/renderer/primitive/base/PrimitiveMeshRendererBase.h"

namespace OriGine {

/// <summary>
/// プリミティブ形状を描画するためのメッシュレンダラー
/// </summary>
template <Primitive::IsPrimitive PrimType>
class PrimitiveMeshRenderer
    : public PrimitiveMeshRendererBase {
public:
    PrimitiveMeshRenderer() : PrimitiveMeshRendererBase() {}
    PrimitiveMeshRenderer(const std::vector<TextureColorMesh>& _meshGroup) : PrimitiveMeshRendererBase(_meshGroup) {}
    PrimitiveMeshRenderer(const std::shared_ptr<std::vector<TextureColorMesh>>& _meshGroup) : PrimitiveMeshRendererBase(_meshGroup) {}

    ~PrimitiveMeshRenderer() override {}

    void Initialize(OriGine::Scene* _scene, const OriGine::EntityHandle& _entity) = 0;

    virtual void Edit(Scene* _scene, const EntityHandle& _entity, const std::string& _parentLabel) override = 0;

    inline void Finalize() override;

    using PrimitiveType = PrimType;

    /// <summary>
    /// 自身のプリミティブ情報をもとにメッシュを作成
    /// </summary>
    virtual void CreateMesh(TextureColorMesh* _mesh) {
        primitive_.CreateMesh(_mesh);
    }

protected:
    PrimType primitive_; // 描画対象となる形状データ本体

public:
    const PrimType& GetPrimitive() const {
        return primitive_;
    }
    PrimType& GetPrimitive() {
        return primitive_;
    }
};

template <Primitive::IsPrimitive PrimType>
inline void PrimitiveMeshRenderer<PrimType>::Finalize() {
    // メッシュ・定数バッファなど保持しているリソースを解放する
    for (auto& mesh : *meshGroup_) {
        mesh.Finalize();
    }
    meshGroup_.reset();
    transformBuff_.Finalize();
    materialBuff_.Finalize();
}

} // namespace OriGine
