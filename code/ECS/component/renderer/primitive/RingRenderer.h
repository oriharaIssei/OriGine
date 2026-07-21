#pragma once

#include "component/renderer/primitive/PrimitiveMeshRenderer.h"
#include "component/renderer/primitive/shape/Ring.h"

namespace OriGine {

/// <summary>
/// Ring描画コンポーネント
/// </summary>
class RingRenderer
    : public PrimitiveMeshRenderer<Primitive::Ring> {
    friend void to_json(nlohmann::json& _j, const RingRenderer& _comp);
    friend void from_json(const nlohmann::json& _j, RingRenderer& _comp);

public:
    RingRenderer() : PrimitiveMeshRenderer() {}
    RingRenderer(const std::vector<TextureColorMesh>& _meshGroup) : PrimitiveMeshRenderer(_meshGroup) {}
    RingRenderer(const std::shared_ptr<std::vector<TextureColorMesh>>& _meshGroup) : PrimitiveMeshRenderer(_meshGroup) {}
    ~RingRenderer() override {}

    /// <summary>
    /// バッファの生成・メッシュ生成・テクスチャ読み込みなど、Ring描画に必要な初期化を行う
    /// </summary>
    void Initialize(Scene* _scene, const EntityHandle& _hostEntity) override;

    /// <summary>
    /// エディタ上でマテリアル・テクスチャ・Ringの形状パラメータを編集するGUIを描画する
    /// </summary>
    void Edit(Scene* _scene, const EntityHandle& _entity, const std::string& _parentLabel) override;
};

} // namespace OriGine
