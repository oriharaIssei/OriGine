#include "IScene.h"

/// engine
#include "Engine.h"
#include "manager/SceneManager.h"
// Ecs
#include "component/IComponent.h"
#include "ECSManager.h"
/// directX12Object
#include "directX12/DxRtvArrayManager.h"
#include "directX12/DxSrvArrayManager.h"
// lib
#include "lib/binaryIO/BinaryIO.h"

IScene::IScene(const std::string& sceneName) : name_(sceneName) {
}

IScene::~IScene() {}

void IScene::Init() {
    LoadSceneEntity();
}

void IScene::Finalize() {
#ifdef _DEBUG
    SaveSceneEntity();
#endif
    sceneRtvArray_.reset();
    sceneSrvArray_.reset();

    ECSManager* ecsManager = ECSManager::getInstance();
    ecsManager->clearComponentArray();
    ecsManager->clearSystem();
    ecsManager->clearEntity();
}
