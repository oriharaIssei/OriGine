#include "RegisterWindowResizeEvent.h"

/// engine
#include "Engine.h"
// directX12
#include "directX12/RenderTexture.h"

/// component
#include "component/renderer/Sprite.h"
#include "component/scene/SubScene.h"

using namespace OriGine;

/// <summary>
/// コンストラクタ
/// </summary>
RegisterWindowResizeEvent::RegisterWindowResizeEvent() : ISystem(SystemCategory::Initialize) {}

/// <summary>
/// ウィンドウリサイズ時のコールバックをEngineへ登録する。
/// シーンビュー・スプライト・サブシーンそれぞれについて、新しいウィンドウサイズに追従させる処理を登録する
/// </summary>
void RegisterWindowResizeEvent::Initialize() {
    Engine* engine = Engine::GetInstance();

    // コールバックはEngine側に保持され続けるため、thisの生ポインタを直接捕捉すると
    // このシステムが破棄された後にダングリングポインタを呼び出す危険がある。
    // weak_ptrで捕捉し、呼び出し時にlock()で生存確認してから使うことで安全にする
    auto shared                                       = shared_from_this();
    std::weak_ptr<RegisterWindowResizeEvent> weakSelf = shared;

#ifndef _DEBUG
    // シーンビューのリサイズイベント登録
    auto sceneViewResizeEvent = [weakSelf](const Vec2f& size) {
        if (auto self = weakSelf.lock()) {
            auto currentScene = self->GetScene();
            if (currentScene) {
                currentScene->GetSceneView()->Resize(size);
            }
        }
    };
    sceneViewResizeEventIndex_ = engine->AddWindowResizeEvent(sceneViewResizeEvent);
#endif // _DEBUG

    // スプライトのリサイズイベント登録
    auto spriteResizeEvent = [weakSelf](const Vec2f& size) {
        if (auto self = weakSelf.lock()) {
            auto currentScene = self->GetScene();
            if (currentScene) {
                auto spritesArray = currentScene->GetComponentArray<SpriteRenderer>();
                for (auto& sprites : spritesArray->GetSlotsRef()) {
                    for (auto& sprite : sprites.components) {
                        sprite.CalculateWindowRatioPosAndSize(size);
                    }
                }
            }
        }
    };
    spriteResizeEventIndex_ = engine->AddWindowResizeEvent(spriteResizeEvent);

    // サブシーンのリサイズイベント登録
    auto subSceneResizeEvent = [weakSelf](const Vec2f& size) {
        if (auto self = weakSelf.lock()) {
            auto currentScene = self->GetScene();
            if (currentScene) {
                auto subScenesArray = currentScene->GetComponentArray<SubScene>();
                for (auto& subScenes : subScenesArray->GetSlotsRef()) {
                    for (auto& subScene : subScenes.components) {
                        auto scene = subScene.GetSubSceneRef();
                        if (scene) {
                            scene->GetSceneView()->Resize(size);
                        }
                    }
                }
            }
        }
    };
    subSceneResizeEventIndex_ = engine->AddWindowResizeEvent(subSceneResizeEvent);
}

/// <summary>
/// Initialize()で登録したウィンドウリサイズイベントを全て解除する
/// </summary>
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

/// <summary>
/// エンティティごとの更新処理（このシステムはイベント登録専用のため使用しない）
/// </summary>
void RegisterWindowResizeEvent::UpdateEntity(const EntityHandle& /*_owner*/) {
}
