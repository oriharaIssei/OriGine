#include "RegisterWindowResizeEvent.h"

/// engine
#include "engine.h"

/// component
#include "component/renderer/Sprite.h"

RegisterWindowResizeEvent::RegisterWindowResizeEvent() : ISystem(SystemCategory::Initialize) {}

void RegisterWindowResizeEvent::Initialize() {
    Engine* engine         = Engine::getInstance();
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
}

void RegisterWindowResizeEvent::UpdateEntity(GameEntity* /*_entity*/) {
}
