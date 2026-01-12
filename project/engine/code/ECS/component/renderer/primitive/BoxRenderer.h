#pragma once

#include "component/renderer/primitive/PrimitiveMeshRenderer.h"
#include "component/renderer/primitive/shape/Box.h"

namespace OriGine {

/// <summary>
/// Box描画コンポーネント
/// </summary>
class BoxRenderer
    : public PrimitiveMeshRenderer<Primitive::Box> {
    friend void to_json(nlohmann::json& j, const BoxRenderer& r);
    friend void from_json(const nlohmann::json& j, BoxRenderer& r);

public:
    BoxRenderer() : PrimitiveMeshRenderer() {}
    BoxRenderer(const std::vector<TextureColorMesh>& _meshGroup) : PrimitiveMeshRenderer(_meshGroup) {}
    BoxRenderer(const std::shared_ptr<std::vector<TextureColorMesh>>& _meshGroup) : PrimitiveMeshRenderer(_meshGroup) {}
    ~BoxRenderer() override {}

    void Initialize(Scene* _scene, EntityHandle _hostEntity) override;

    void Edit(Scene* _scene, EntityHandle _entity, const std::string& _parentLabel) override;
};

} // namespace OriGine
