#include "RegisterWindowResizeEvent.h"

/// engine
#include "engine.h"
// directX12
#include "directX12/RenderTexture.h"

/// component
#include "component/renderer/Sprite.h"

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
}

void RegisterWindowResizeEvent::Finalize() {
    Engine* engine = Engine::GetInstance();
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
