#pragma once

#include "component/renderer/primitive/PrimitiveMeshRenderer.h"
#include "component/renderer/primitive/shape/Box.h"

namespace OriGine {

/// <summary>
/// Box描画コンポーネント
/// </summary>
class BoxRenderer
    : public PrimitiveMeshRenderer<Primitive::Box> {
    friend void to_json(nlohmann::json& _j, const BoxRenderer& _comp);
    friend void from_json(const nlohmann::json& _j, BoxRenderer& _comp);

public:
    BoxRenderer() : PrimitiveMeshRenderer() {}
    BoxRenderer(const std::vector<TextureColorMesh>& _meshGroup) : PrimitiveMeshRenderer(_meshGroup) {}
    BoxRenderer(const std::shared_ptr<std::vector<TextureColorMesh>>& _meshGroup) : PrimitiveMeshRenderer(_meshGroup) {}
    ~BoxRenderer() override {}

    /// <summary>
    /// バッファの生成・メッシュ生成・テクスチャ読み込みなど、Box描画に必要な初期化を行う
    /// </summary>
    void Initialize(Scene* _scene, const EntityHandle& _hostEntity) override;

    /// <summary>
    /// エディタ上でマテリアル・テクスチャ・Boxの形状パラメータを編集するGUIを描画する
    /// </summary>
    void Edit(Scene* _scene, const EntityHandle& _entity, const std::string& _parentLabel) override;
};

} // namespace OriGine
