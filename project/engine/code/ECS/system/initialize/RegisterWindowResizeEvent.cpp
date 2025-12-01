#include "RegisterWindowResizeEvent.h"

/// engine
#include "engine.h"
// directX12
#include "directX12/RenderTexture.h"

/// component
#include "component/renderer/Sprite.h"
#include "component/SubScene.h"

RegisterWindowResizeEvent::RegisterWindowResizeEvent() : ISystem(SystemCategory::Initialize) {}

void RegisterWindowResizeEvent::Initialize() {
    Engine* engine = Engine::GetInstance();

#ifndef _DEBUG
    // シーンビューのリサイズイベント登録
    auto sceneViewResizeEvent = [this](const Vec2f& size) {
        auto currentScene = GetScene();
        if (currentScene) {
            currentScene->GetSceneView()->Resize(size);
        }
    };
    sceneViewResizeEventIndex_ = engine->AddWindowResizeEvent(sceneViewResizeEvent);
#endif // _DEBUG

    // スプライトのリサイズイベント登録
    auto spriteResizeEvent = [this](const Vec2f& size) {
        auto currentScene = GetScene();
        if (currentScene) {
            auto spritesArray = currentScene->GetComponentArray<SpriteRenderer>();
            for (auto& sprites : spritesArray->GetAllComponents()) {
                for (auto& sprite : sprites) {
                    sprite.CalculateWindowRatioPosAndSize(size);
                }
            }
        }
    };
    spriteResizeEventIndex_ = engine->AddWindowResizeEvent(spriteResizeEvent);

    // サブシーンのリサイズイベント登録
    auto subSceneResizeEvent = [this](const Vec2f& size) {
        auto currentScene = GetScene();
        if (currentScene) {
            auto subScenesArray = currentScene->GetComponentArray<SubScene>();
            for (auto& subScenes : subScenesArray->GetAllComponents()) {
                for (auto& subScene : subScenes) {
                    auto scene = subScene.GetSubSceneRef();
                    if (scene) {
                        scene->GetSceneView()->Resize(size);
                    }
                }
            }
        }
    };
    subSceneResizeEventIndex_ = engine->AddWindowResizeEvent(subSceneResizeEvent);
}

void RegisterWindowResizeEvent::Finalize() {
    Engine* engine = Engine::GetInstance();
    if (subSceneResizeEventIndex_ != -1) {
        engine->RemoveWindowResizeEvent(subSceneResizeEventIndex_);
        subSceneResizeEventIndex_ = -1;
    }

    if (spriteResizeEventIndex_ != -1) {
        engine->RemoveWindowResizeEvent(spriteResizeEventIndex_);
        spriteResizeEventIndex_ = -1;
    }

#ifndef _DEBUG
    if (sceneViewResizeEventIndex_ != -1) {
        engine->RemoveWindowResizeEvent(sceneViewResizeEventIndex_);
        sceneViewResizeEventIndex_ = -1;
    }
#endif // _DEBUG
}

void RegisterWindowResizeEvent::UpdateEntity(Entity* /*_entity*/) {
}
