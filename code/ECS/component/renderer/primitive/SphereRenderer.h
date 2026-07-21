#pragma once

#include "component/renderer/primitive/PrimitiveMeshRenderer.h"
#include "component/renderer/primitive/shape/Sphere.h"

namespace OriGine {

/// <summary>
/// Sphere描画コンポーネント
/// </summary>
class SphereRenderer
    : public PrimitiveMeshRenderer<Primitive::Sphere> {
    friend void to_json(nlohmann::json& _j, const SphereRenderer& _comp);
    friend void from_json(const nlohmann::json& _j, SphereRenderer& _comp);

public:
    SphereRenderer() : PrimitiveMeshRenderer() {}
    SphereRenderer(const std::vector<TextureColorMesh>& _meshGroup) : PrimitiveMeshRenderer(_meshGroup) {}
    SphereRenderer(const std::shared_ptr<std::vector<TextureColorMesh>>& _meshGroup) : PrimitiveMeshRenderer(_meshGroup) {}

    ~SphereRenderer() override {}

    /// <summary>
    /// バッファの生成・メッシュ生成・テクスチャ読み込みなど、Sphere描画に必要な初期化を行う
    /// </summary>
    void Initialize(Scene* _scene, const EntityHandle& _hostEntity) override;
    /// <summary>
    /// エディタ上でマテリアル・テクスチャ・Sphereの形状パラメータを編集するGUIを描画する
    /// </summary>
    void Edit(Scene* _scene, const EntityHandle& _entity, const std::string& _parentLabel) override;
};

} // namespace OriGine
