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

    ~PrimitiveMeshRenderer() {}

    // Initialize / Edit はここでは実装を持たない。BoxRenderer等の具象クラス側が
    // 自分の非仮想メンバとして実装を提供する(Phase 3 3B。旧: ここで改めて=0と
    // 再宣言することで「PrimitiveMeshRenderer<T>自身も引き続き抽象クラスである」ことを
    // 明示していたが、非仮想化に伴い抽象クラスという概念自体が無くなったため削除した)。

    inline void Finalize();

    using PrimitiveType = PrimType;

    /// <summary>
    /// 自身のプリミティブ情報をもとにメッシュを作成
    /// </summary>
    void CreateMesh(TextureColorMesh* _mesh) {
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
