#pragma once
#include "component/renderer/primitive/PrimitiveMeshRenderer.h"
#include "component/renderer/primitive/shape/Cylinder.h"

namespace OriGine {
class CylinderRenderer
    : public PrimitiveMeshRenderer<Primitive::Cylinder> {
    friend void to_json(nlohmann::json& j, const CylinderRenderer& c);
    friend void from_json(const nlohmann::json& j, CylinderRenderer& c);

public:
    CylinderRenderer() : PrimitiveMeshRenderer() {}
    CylinderRenderer(const std::vector<TextureColorMesh>& _meshGroup) : PrimitiveMeshRenderer(_meshGroup) {}
    CylinderRenderer(const std::shared_ptr<std::vector<TextureColorMesh>>& _meshGroup) : PrimitiveMeshRenderer(_meshGroup) {}

    ~CylinderRenderer() override {}

    void Initialize(Scene* _scene, EntityHandle _hostEntity) override;
    void Edit(Scene* _scene, EntityHandle _entity, const std::string& _parentLabel) override;
};
} // namespace OriGine
