#include "BaseRenderSystem.h"

/// engine
#include "scene/Scene.h"

using namespace OriGine;

BaseRenderSystem::BaseRenderSystem(int32_t _priority) : ISystem(SystemCategory::Render, _priority) {}

/// <summary>
/// デストラクタ
/// </summary>
BaseRenderSystem::~BaseRenderSystem() {}

/// <summary>
/// システムの初期化。DxCommandの生成とPSOの作成を行う。
/// </summary>
void BaseRenderSystem::Initialize() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");

    CreatePSO();
}

/// <summary>
/// システムの更新処理。レンダラーの登録とレンダリングの実行を行う。
/// </summary>
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

/// <summary>
/// システムの終了処理。DxCommandの解放を行う。
/// </summary>
void BaseRenderSystem::Finalize() {
    if (dxCommand_) {
        dxCommand_->Finalize();
        dxCommand_.reset();
    }
}

/// <summary>
/// レンダリングの一連の処理を実行する
/// </summary>
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
