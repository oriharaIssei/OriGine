#include "GameScene.h"

/// ECS
#include "ECS/ECSManager.h"
// component
#include "component/renderer/MeshRender.h"
#include "component/transform/Transform.h"
// system
#include "system/render/TexturedMeshRenderSystem.h"

GameScene::GameScene()
    : IScene("GameScene") {}

GameScene::~GameScene() {}

void GameScene::Init() {
    IScene::Init();
}

void GameScene::Finalize() {
    IScene::Finalize();
}
