#include "BaseRenderSystem.h"

/// engine
#include "scene/Scene.h"

using namespace OriGine;

BaseRenderSystem::BaseRenderSystem(int32_t _priority) : ISystem(SystemCategory::Render, _priority) {}
BaseRenderSystem::~BaseRenderSystem() {}

void BaseRenderSystem::Initialize() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");

    CreatePSO();
}

void BaseRenderSystem::Update() {
    // レンダリング対象が無ければスキップ
    if (entities_.empty()) {
        return;
    }

    // 有効でないエンティティを削除
    EraseDeadEntity();

    // レンダラー登録
    for (auto& entityID : entities_) {
        DispatchRenderer(entityID);
    }

    // レンダリングスキップ判定
    if (ShouldSkipRender()) {
        return;
    }

    // レンダリング実行
    Rendering();
}

void BaseRenderSystem::Finalize() {
    if (dxCommand_) {
        dxCommand_->Finalize();
        dxCommand_.reset();
    }
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
