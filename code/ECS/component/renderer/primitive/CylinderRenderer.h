#pragma once
#include "component/renderer/primitive/PrimitiveMeshRenderer.h"
#include "component/renderer/primitive/shape/Cylinder.h"

namespace OriGine {
/// <summary>
/// Cylinder描画コンポーネント
/// </summary>
class CylinderRenderer
    : public PrimitiveMeshRenderer<Primitive::Cylinder> {
    friend void to_json(nlohmann::json& _j, const CylinderRenderer& _comp);
    friend void from_json(const nlohmann::json& _j, CylinderRenderer& _comp);

public:
    CylinderRenderer() : PrimitiveMeshRenderer() {}
    CylinderRenderer(const std::vector<TextureColorMesh>& _meshGroup) : PrimitiveMeshRenderer(_meshGroup) {}
    CylinderRenderer(const std::shared_ptr<std::vector<TextureColorMesh>>& _meshGroup) : PrimitiveMeshRenderer(_meshGroup) {}

    ~CylinderRenderer() override {}

    /// <summary>
    /// バッファの生成・メッシュ生成・テクスチャ読み込みなど、Cylinder描画に必要な初期化を行う
    /// </summary>
    void Initialize(Scene* _scene, const EntityHandle& _hostEntity) override;
    /// <summary>
    /// エディタ上でマテリアル・テクスチャ・Cylinderの形状パラメータを編集するGUIを描画する
    /// </summary>
    void Edit(Scene* _scene, const EntityHandle& _entity, const std::string& _parentLabel) override;
};
} // namespace OriGine
