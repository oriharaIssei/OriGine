#include "IScene.h"

/// engine
#include "Engine.h"
#include "manager/SceneManager.h"
// Ecs
#include "ECS/component/IComponent.h"
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
    void CreateSceneSystem();
    void CreateSceneEntity();

    void LoadSceneEntity();
    void SaveSceneEntity();
}

void IScene::Finalize() {
#ifdef _DEBUG
    SaveSceneEntity();
#endif
}
