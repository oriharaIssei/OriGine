#pragma once

#include "component/renderer/primitive/PrimitiveMeshRenderer.h"
#include "component/renderer/primitive/shape/Sphere.h"


class SphereRenderer
    : public PrimitiveMeshRenderer<Primitive::Sphere> {
    friend void to_json(nlohmann::json& j, const SphereRenderer& r);
    friend void from_json(const nlohmann::json& j, SphereRenderer& r);

public:
    SphereRenderer() : PrimitiveMeshRenderer() {}
    SphereRenderer(const std::vector<TextureMesh>& _meshGroup) : PrimitiveMeshRenderer(_meshGroup) {}
    SphereRenderer(const std::shared_ptr<std::vector<TextureMesh>>& _meshGroup) : PrimitiveMeshRenderer(_meshGroup) {}

    ~SphereRenderer() override {}

    void Initialize(Entity* _hostEntity) override;
    void Edit(Scene* _scene, Entity* _entity, [[maybe_unused]] const std::string& _parentLabel) override;
};
