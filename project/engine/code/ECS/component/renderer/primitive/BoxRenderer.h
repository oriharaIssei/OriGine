#pragma once

#include "component/renderer/primitive/PrimitiveMeshRenderer.h"
#include "component/renderer/primitive/shape/Box.h"

class BoxRenderer
    : public PrimitiveMeshRenderer<Primitive::Box> {
    friend void to_json(nlohmann::json& j, const BoxRenderer& r);
    friend void from_json(const nlohmann::json& j, BoxRenderer& r);

public:
    BoxRenderer() : PrimitiveMeshRenderer() {}
    BoxRenderer(const std::vector<TextureMesh>& _meshGroup) : PrimitiveMeshRenderer(_meshGroup) {}
    BoxRenderer(const std::shared_ptr<std::vector<TextureMesh>>& _meshGroup) : PrimitiveMeshRenderer(_meshGroup) {}
    ~BoxRenderer() override {}

    void Initialize(Entity* _hostEntity) override;

    void Edit(Scene* _scene, Entity* _entity, [[maybe_unused]] const std::string& _parentLabel) override;
};
