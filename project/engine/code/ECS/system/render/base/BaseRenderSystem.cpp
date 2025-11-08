#include "BaseRenderSystem.h"

BaseRenderSystem::BaseRenderSystem(int32_t _priority) : ISystem(SystemCategory::Render, _priority) {}
BaseRenderSystem::~BaseRenderSystem() {}

void BaseRenderSystem::Initialize() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");

    CreatePSO();
}

void BaseRenderSystem::Update() {
    // レンダリング対象が無ければスキップ
    if (entityIDs_.empty()) {
        return;
    }

    // 有効でないエンティティを削除
    eraseDeadEntity();

    // レンダラー登録
    auto* hostScene = getScene();
    for (auto& entityID : entityIDs_) {
        Entity* entity = hostScene->getEntityRepositoryRef()->getEntity(entityID);
        DispatchRenderer(entity);
    }

    // レンダリングスキップ判定
    if (IsSkipRendering()) {
        return;
    }

    // レンダリング実行
    Rendering();
}

void BaseRenderSystem::Finalize() {
    dxCommand_->Finalize();
    dxCommand_.reset();
}

void BaseRenderSystem::Rendering() {
    // レンダリング開始
    StartRender();

    // ブレンドモードごとに描画
    for (int32_t cullingIndex = 0; cullingIndex < 2; ++cullingIndex) {
        for (int32_t blend = 0; blend < static_cast<int32_t>(BlendMode::Count); ++blend) {
            RenderingBy(static_cast<BlendMode>(blend), cullingIndex);
        }
    }
}
