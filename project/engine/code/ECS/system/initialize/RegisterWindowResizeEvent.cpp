#include "RegisterWindowResizeEvent.h"

/// engine
#include "engine.h"
// directX12
#include "directX12/RenderTexture.h"

/// component
#include "component/renderer/Sprite.h"

RegisterWindowResizeEvent::RegisterWindowResizeEvent() : ISystem(SystemCategory::Initialize) {}

void RegisterWindowResizeEvent::Initialize() {
    Engine* engine = Engine::getInstance();

#ifndef _DEBUG
    auto sceneViewResizeEvent = [this](const Vec2f& size) {
        auto currentScene = getScene();
        if (currentScene) {
            currentScene->getSceneView()->Resize(size);
        }
    };
    sceneViewResizeEventIndex_ = engine->addWindowResizeEvent(sceneViewResizeEvent);
#endif // _DEBUG

    auto spriteResizeEvent = [this](const Vec2f& size) {
        auto currentScene = getScene();
        if (currentScene) {
            auto spritesArray = currentScene->getComponentArray<SpriteRenderer>();
            for (auto& sprites : *spritesArray->getAllComponents()) {
                for (auto& sprite : sprites) {
                    sprite.CalculateWindowRatioPosAndSize(size);
                }
            }
        }
    };
    spriteResizeEventIndex_ = engine->addWindowResizeEvent(spriteResizeEvent);
}

void RegisterWindowResizeEvent::Finalize() {
    Engine* engine = Engine::getInstance();
    if (spriteResizeEventIndex_ != -1) {
        engine->removeWindowResizeEvent(spriteResizeEventIndex_);
        spriteResizeEventIndex_ = -1;
    }
#ifndef _DEBUG
    if (sceneViewResizeEventIndex_ != -1) {
        engine->removeWindowResizeEvent(sceneViewResizeEventIndex_);
        sceneViewResizeEventIndex_ = -1;
    }
#endif // _DEBUG
}

void RegisterWindowResizeEvent::UpdateEntity(GameEntity* /*_entity*/) {
}
