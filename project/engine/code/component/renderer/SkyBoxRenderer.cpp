#include "SkyBoxRenderer.h"

/// engine
#define ENGINE_ECS
#include "EngineInclude.h"

void SkyBoxRenderer::Initialize(GameEntity* _hostEntity) {
    MeshRenderer::Initialize(_hostEntity);

    /// mesh
    meshGroup_->push_back(Mesh<SkyBoxVertex>());
    auto& mesh = meshGroup_->back();
    mesh.Initialize(12, 24);

    mesh.vertexes_[0].position = {1.f, 1.f, 1.f, 1.f};
    mesh.vertexes_[1].position = {1.f, 1.f, -1.f, 1.f};
    mesh.vertexes_[2].position = {1.f, -1.f, 1.f, 1.f};
    mesh.vertexes_[3].position = {1.f, -1.f, -1.f, 1.f};

    mesh.vertexes_[4].position = {-1.f, 1.f, -1.f, 1.f};
    mesh.vertexes_[5].position = {-1.f, 1.f, 1.f, 1.f};
    mesh.vertexes_[6].position = {-1.f, -1.f, -1.f, 1.f};
    mesh.vertexes_[7].position = {-1.f, -1.f, 1.f, 1.f};

    mesh.vertexes_[8].position  = {1.f, 1.f, 1.f, 1.f};
    mesh.vertexes_[9].position  = {1.f, 1.f, 1.f, 1.f};
    mesh.vertexes_[10].position = {-1.f, -1.f, 1.f, 1.f};
    mesh.vertexes_[11].position = {1.f, -1.f, 1.f, 1.f};
}
