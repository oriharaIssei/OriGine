#include "BasePostRenderingSystem.h"

/// engine
#include "scene/Scene.h"

using namespace OriGine;

BasePostRenderingSystem::BasePostRenderingSystem(int32_t _priority) : ISystem(SystemCategory::PostRender, _priority) {}
BasePostRenderingSystem::~BasePostRenderingSystem() {}

void BasePostRenderingSystem::Initialize() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main", D3D12_COMMAND_LIST_TYPE_DIRECT);
    CreatePSO();
}

void BasePostRenderingSystem::Update() {
    // 有効判定
    if (entities_.empty()) {
        return;
    }

    // 無効エンティティの削除
    EraseDeadEntity();

    // renderTargetがnullなら sceneのsceneViewをセットする
    if (renderTarget_ == nullptr) {
        renderTarget_ = GetScene()->GetSceneView();
    }

    // コンポーネントの登録
    for (auto entityID : entities_) {
        Entity* entity = GetScene()->GetEntity(entityID);
        DispatchComponent(entity);
    }

    // ポストレンダリングスキップ判定
    if (ShouldSkipPostRender()) {
        return;
    }

    // レンダリング実行
    Rendering();
}

void BasePostRenderingSystem::Finalize() {
    if (dxCommand_) {
        dxCommand_->Finalize();
        dxCommand_.reset();
    }
}
