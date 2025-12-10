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
    PrimitiveMeshRenderer(const std::vector<TextureMesh>& _meshGroup) : PrimitiveMeshRendererBase(_meshGroup) {}
    PrimitiveMeshRenderer(const std::shared_ptr<std::vector<TextureMesh>>& _meshGroup) : PrimitiveMeshRendererBase(_meshGroup) {}

    ~PrimitiveMeshRenderer() override {}

    void Initialize(OriGine::Entity* _entity) = 0;

    virtual void Edit(Scene* _scene, OriGine::Entity* _entity, [[maybe_unused]] const std::string& _parentLabel) override = 0;

    inline void Finalize() override;

    using PrimitiveType = PrimType;

    /// <summary>
    /// 自身のプリミティブ情報をもとにメッシュを作成
    /// </summary>
    virtual void CreateMesh(TextureMesh* _mesh) {
        primitive_.CreateMesh(_mesh);
    }

protected:
    PrimType primitive_;

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
    for (auto& mesh : *meshGroup_) {
        mesh.Finalize();
    }
    meshGroup_.reset();
    transformBuff_.Finalize();
    materialBuff_.Finalize();
}

} // namespace OriGine
