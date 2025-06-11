#include "IScene.h"

/// engine
#define ENGINE_INCLUDE
#include "sceneManager/SceneManager.h"
#define RESOURCE_DIRECTORY
// Ecs
#define ENGINE_ECS

// lib

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

    ecsManager->clearUniqueEntities();
    ecsManager->clearAliveEntities();
    ecsManager->ExecuteEntitiesDelete();

#ifdef _DEBUG
    EditorController::getInstance()->clearCommandHistory();
#endif // _DEBUG
}
