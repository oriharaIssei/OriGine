#pragma once

#include "component/renderer/primitive/PrimitiveMeshRenderer.h"
#include "component/renderer/primitive/shape/Plane.h"

namespace OriGine {

/// <summary>
/// Plane描画コンポーネント
/// </summary>
class PlaneRenderer
    : public PrimitiveMeshRenderer<Primitive::Plane> {
    friend void to_json(nlohmann::json& _j, const PlaneRenderer& _comp);
    friend void from_json(const nlohmann::json& _j, PlaneRenderer& _comp);

public:
    PlaneRenderer() : PrimitiveMeshRenderer() {}
    PlaneRenderer(const std::vector<TextureColorMesh>& _meshGroup) : PrimitiveMeshRenderer(_meshGroup) {}
    PlaneRenderer(const std::shared_ptr<std::vector<TextureColorMesh>>& _meshGroup) : PrimitiveMeshRenderer(_meshGroup) {}
    ~PlaneRenderer() override {}

    /// <summary>
    /// バッファの生成・メッシュ生成・テクスチャ読み込みなど、Plane描画に必要な初期化を行う
    /// </summary>
    void Initialize(Scene* _scene, const EntityHandle& _hostEntity) override;

    /// <summary>
    /// エディタ上でマテリアル・テクスチャ・Planeの形状パラメータを編集するGUIを描画する
    /// </summary>
    void Edit(Scene* _scene, const EntityHandle& _entity, const std::string& _parentLabel) override;
};

} // namespace OriGine
