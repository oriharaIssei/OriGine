#pragma once

/// base
#include "component/renderer/primitive/base/IPrimitive.h"
#include "component/renderer/primitive/base/PrimitiveMeshRendererBase.h"

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

    void Initialize(Entity* _entity) = 0;

    inline void Finalize() override;

    using PrimitiveType = PrimType;

    /// <summary>
    /// 自身のプリミティブ情報をもとにメッシュを作成
    /// </summary>
    virtual void createMesh(TextureMesh* _mesh) {
        primitive_.createMesh(_mesh);
    }

protected:
    PrimType primitive_;

public:
    const PrimType& getPrimitive() const {
        return primitive_;
    }
    PrimType& getPrimitive() {
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
