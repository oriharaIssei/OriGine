#include "BasePostRenderingSystem.h"

BasePostRenderingSystem::BasePostRenderingSystem(int32_t _priority) : ISystem(SystemCategory::PostRender, _priority) {}
BasePostRenderingSystem::~BasePostRenderingSystem() {}

void BasePostRenderingSystem::Initialize() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main", D3D12_COMMAND_LIST_TYPE_DIRECT);
    CreatePSO();
}

void BasePostRenderingSystem::Update() {
    if (entityIDs_.empty()) {
        return;
    }

    // 無効エンティティの削除
    eraseDeadEntity();

    // コンポーネントの登録
    for (auto entityID : entityIDs_) {
        Entity* entity = getScene()->getEntity(entityID);
        DispatchComponent(entity);
    }

    // ポストレンダリングスキップ判定
    if (ShouldSkipPostRender()) {
        return;
    }

    // レンダリング実行
    RenderStart();
    Rendering();
    RenderEnd();
}

void BasePostRenderingSystem::Finalize() {
    if (dxCommand_) {
        dxCommand_.reset();
    }
}
