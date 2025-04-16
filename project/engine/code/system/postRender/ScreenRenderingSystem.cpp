#include "ScreenRenderingSystem.h"

/// engine
#include "Engine.h"
#include "sceneManager/SceneManager.h"

// directX12
#include "directX12/RenderTexture.h"

void ScreenRenderingSystem::Initialize() {
}

void ScreenRenderingSystem::Update() {
    Engine::getInstance()->ScreenPreDraw();
    SceneManager::getInstance()->getSceneView()->DrawTexture();
    Engine::getInstance()->ScreenPostDraw();
}

void ScreenRenderingSystem::Finalize() {}
