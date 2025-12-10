#pragma once

#include "component/renderer/primitive/PrimitiveMeshRenderer.h"
#include "component/renderer/primitive/shape/Ring.h"

namespace OriGine {

class RingRenderer
    : public PrimitiveMeshRenderer<Primitive::Ring> {
    friend void to_json(nlohmann::json& j, const RingRenderer& r);
    friend void from_json(const nlohmann::json& j, RingRenderer& r);

public:
    RingRenderer() : PrimitiveMeshRenderer() {}
    RingRenderer(const std::vector<TextureMesh>& _meshGroup) : PrimitiveMeshRenderer(_meshGroup) {}
    RingRenderer(const std::shared_ptr<std::vector<TextureMesh>>& _meshGroup) : PrimitiveMeshRenderer(_meshGroup) {}
    ~RingRenderer() override {}

    void Initialize(Entity* _hostEntity) override;

    void Edit(Scene* _scene, Entity* _entity, [[maybe_unused]] const std::string& _parentLabel) override;
};

} // namespace OriGine
