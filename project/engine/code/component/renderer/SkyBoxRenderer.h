#pragma once

/// parent
#include "MeshRenderer.h"

/// math
#include "math/Vector2.h"
#include "math/Vector4.h"

struct SkyBoxVertex {
    Vec4f position;
};

class SkyBoxRenderer
    : public MeshRenderer<Mesh<SkyBoxVertex>, SkyBoxVertex> {
public:
    SkyBoxRenderer() : MeshRenderer() {}
    ~SkyBoxRenderer() {}

    void Initialize(GameEntity* _hostEntity) override;

private:
    std::string filePath_;
    int32_t textureIndex_ = 0;
};
