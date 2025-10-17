#pragma once

#include "component/renderer/primitive/PrimitiveMeshRenderer.h"
#include "component/renderer/primitive/shape/Plane.h"

class PlaneRenderer
    : public PrimitiveMeshRenderer<Primitive::Plane> {
    friend void to_json(nlohmann::json& j, const PlaneRenderer& r);
    friend void from_json(const nlohmann::json& j, PlaneRenderer& r);

public:
    PlaneRenderer() : PrimitiveMeshRenderer() {}
    PlaneRenderer(const std::vector<TextureMesh>& _meshGroup) : PrimitiveMeshRenderer(_meshGroup) {}
    PlaneRenderer(const std::shared_ptr<std::vector<TextureMesh>>& _meshGroup) : PrimitiveMeshRenderer(_meshGroup) {}
    ~PlaneRenderer() override {}

    void Initialize(GameEntity* _hostEntity) override;

    void Edit(Scene* _scene, GameEntity* _entity, [[maybe_unused]] const std::string& _parentLabel) override;
};
