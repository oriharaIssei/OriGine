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
    // ===================================== DefaultEntity ===================================== //
    // ----------------------------------- Player ----------------------------------- //
    GameEntity* player = CreateEntity<Transform, TextureMeshRenderer>("Player", Transform(), CreateModelMeshRenderer("resource/Models", "Enemy.obj"));

    // ===================================== DefaultSystem ===================================== //
    // ----------------------------------- Render ----------------------------------- //
    ECSManager::getInstance()->registerSystem<TexturedMeshRenderSystem>();
    {
        TexturedMeshRenderSystem* texturedMeshRenderSystem = ECSManager::getInstance()->getSystem<TexturedMeshRenderSystem>();
        texturedMeshRenderSystem->Init();

        texturedMeshRenderSystem->addEntity(player);
    }
}

void GameScene::Finalize() {
    IScene::Finalize();
}

void GameScene::LoadSceneEntity() {
}

void GameScene::SaveSceneEntity() {
}
