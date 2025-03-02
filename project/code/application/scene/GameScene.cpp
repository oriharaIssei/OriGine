#include "GameScene.h"

/// ECS
#include "ECS/ECSManager.h"
// component
#include "renderer/meshRenderer/MeshRender.h"
#include "transform/Transform.h"
// system
#include "system/render/TexturedMeshRenderSystem.h"

GameScene::GameScene()
    : IScene("GameScene") {}

GameScene::~GameScene() {}

void GameScene::Init() {

    IScene::Init();
}

void GameScene::Finalize() {
}

void GameScene::CreateSceneEntity() {
    // Player 作成
    CreateEntity<Transform, TextureMeshRenderer>("Player", Transform(), TextureMeshRenderer());
}

void GameScene::CreateSceneSystem() {
}

void GameScene::LoadSceneEntity() {
}

void GameScene::SaveSceneEntity() {
}
