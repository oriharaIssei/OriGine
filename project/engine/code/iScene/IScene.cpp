#include "IScene.h"

/// stl
// container
#include <list>

/// engine
#define ENGINE_INCLUDE
#include "sceneManager/SceneManager.h"
#define RESOURCE_DIRECTORY
// Ecs
#define ENGINE_ECS

/// directX12Object
#include "directX12/DxRtvArrayManager.h"
#include "directX12/DxSrvArrayManager.h"
// lib
#include "lib/binaryIO/BinaryIO.h"

#include "engine/EngineInclude.h"

void SceneInitialize(const std::string& _sceneName) {
    // 読み込み (component,System の登録のあと)
    SceneSerializer serializer;
    serializer.Deserialize(_sceneName);

#ifdef _DEBUG
    SceneManager* sceneManager = SceneManager::getInstance();
    if (sceneManager->inDebugMode()) {
        ECSManager::getInstance()->RunInitialize();
    }
   /* if (!EngineEditor::getInstance()->isActive()) {
        ECSManager::getInstance()->RunInitialize();
    }*/
#else
    ECSManager::getInstance()->RunInitialize();
#endif // _DEBUG
}

void SceneFinalize() {
    ECSManager* ecsManager = ECSManager::getInstance();
    ecsManager->clearWorkSystems();

    ecsManager->clearAliveEntities();
    ecsManager->clearUniqueEntities();
    ecsManager->ExecuteEntitiesDelete();
}
