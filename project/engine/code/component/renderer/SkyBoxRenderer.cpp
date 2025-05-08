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

    // 右
    mesh.vertexes_[0].position = {1.f, 1.f, 1.f, 1.f};
    mesh.vertexes_[1].position = {1.f, 1.f, -1.f, 1.f};
    mesh.vertexes_[2].position = {1.f, -1.f, 1.f, 1.f};
    mesh.vertexes_[3].position = {1.f, -1.f, -1.f, 1.f};
    // 左
    mesh.vertexes_[4].position = {-1.f, 1.f, -1.f, 1.f};
    mesh.vertexes_[5].position = {-1.f, 1.f, 1.f, 1.f};
    mesh.vertexes_[6].position = {-1.f, -1.f, -1.f, 1.f};
    mesh.vertexes_[7].position = {-1.f, -1.f, 1.f, 1.f};
    // 前
    mesh.vertexes_[8].position  = {1.f, 1.f, 1.f, 1.f};
    mesh.vertexes_[9].position  = {1.f, 1.f, 1.f, 1.f};
    mesh.vertexes_[10].position = {-1.f, -1.f, 1.f, 1.f};
    mesh.vertexes_[11].position = {1.f, -1.f, 1.f, 1.f};
    // 後ろ
    mesh.vertexes_[12].position = {-1.f, 1.f, -1.f, 1.f};
    mesh.vertexes_[13].position = {-1.f, 1.f, -1.f, 1.f};
    mesh.vertexes_[14].position = {1.f, -1.f, -1.f, 1.f};
    mesh.vertexes_[15].position = {-1.f, -1.f, -1.f, 1.f};
    // 上
    mesh.vertexes_[16].position = {1.f, 1.f, -1.f, 1.f};
    mesh.vertexes_[17].position = {1.f, 1.f, -1.f, 1.f};
    mesh.vertexes_[18].position = {1.f, 1.f, 1.f, 1.f};
    mesh.vertexes_[19].position = {1.f, 1.f, 1.f, 1.f};
    // 下
    mesh.vertexes_[20].position = {-1.f, -1.f, 1.f, 1.f};
    mesh.vertexes_[21].position = {-1.f, -1.f, 1.f, 1.f};
    mesh.vertexes_[22].position = {-1.f, -1.f, -1.f, 1.f};
    mesh.vertexes_[23].position = {-1.f, -1.f, -1.f, 1.f};

    mesh.indexes_ = {
        0,
        1,
        2,
        2,
        1,
        3, // 右
        4,
        5,
        6,
        6,
        5,
        7, // 左
        8,
        9,
        10,
        10,
        9,
        11, // 前
        12,
        13,
        14,
        14,
        13,
        15, // 後ろ
        16,
        17,
        18,
        18,
        17,
        19, // 上
        20,
        21,
        22,
        22,
        21,
        23, // 下
    };
}
