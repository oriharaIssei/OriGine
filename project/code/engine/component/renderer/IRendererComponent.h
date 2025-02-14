#pragma once
/// interface
#include "component/IComponent.h"
#include "module/IModule.h"

/// stl
#include <memory>
// container
#include <unordered_map>
#include <vector>

// Utility
#include <concepts>

/// engine
#include "Engine.h"

// directX12Object
#include "directX12/DxCommand.h"
#include "directX12/DxDevice.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"

//==============================================================
// ↓ IRendererComponent
//==============================================================
/// <summary>
/// 描画を行うコンポーネントの基底クラス
/// </summary>
class IRendererComponent
    : public IComponent {
public:
    IRendererComponent()          = default;
    virtual ~IRendererComponent() = default;

    virtual void Init()     = 0;
    virtual void Update()   = 0;
    virtual void Render()   = 0;
    virtual void Finalize() = 0;
};

/*
    ↓ IRendererCompoenentControllerは RendererComponentController<T>を統一して管理するために用意したもので,
    ↓  基本はRendererComponentController<T>を使用することを想定している
*/

//==============================================================
// ↓ IRendererComponentController
//==============================================================
///< summary>
/// RendererComponent管理クラスのインターフェース
///</summary>
class IRendererComponentController
    : public IModule {
public:
    IRendererComponentController() : IModule() {}
    virtual ~IRendererComponentController() {}

    ///< summary>
    /// 初期化
    ///</summary>
    virtual void Init() {
        dxDevice_  = Engine::getInstance()->getDxDevice();
        dxCommand_ = std::make_unique<DxCommand>();
        dxCommand_->Init("main","main");

        CreatePso();
    }

    ///< summary>
    /// 更新,描画
    ///</summary>
    virtual void RenderFrame() {
        // 更新してから描画
        Update();

        // 描画
        StartRender();
        RenderAll();
        EndRender();
    }

    ///< summary>
    /// 終了処理
    ///</summary>
    virtual void Finalize() {
        dxCommand_->Finalize();
        dxCommand_.reset();
        dxDevice_ = nullptr;
    }

protected:
    ///< summary>
    /// 更新
    ///</summary>
    virtual void Update() = 0;
    ///< summary>
    /// 描画開始
    ///</summary>
    virtual void StartRender() = 0;
    ///< summary>
    /// RenderScheduleをすべて描画
    ///</summary>
    virtual void RenderAll() = 0;
    ///< summary>
    /// 描画終了
    ///</summary>
    virtual void EndRender() = 0;

    ///< summary>
    /// PipelineStateObjectを作成
    ///</summary>
    virtual void CreatePso() = 0;

protected:
    ///< summary>
    /// ブレンドモードを変更 << 描画時にオブジェクトのブレンドモードを変更するための関数
    /// </summary>
    void changeBlendMode(BlendMode blend) {
        currentBlend_                          = blend;
        ID3D12GraphicsCommandList* commandList = dxCommand_->getCommandList();

        commandList->SetGraphicsRootSignature(pso_[currentBlend_]->rootSignature.Get());
        commandList->SetPipelineState(pso_[currentBlend_]->pipelineState.Get());
    }

protected:
    DxDevice* dxDevice_                   = nullptr;
    std::unique_ptr<DxCommand> dxCommand_ = nullptr;

    // 描画設定
    BlendMode currentBlend_ = BlendMode::Alpha;
    std::unordered_map<BlendMode, PipelineStateObj*> pso_;

public: // ↓ Accessor
    //------------------------------ dxCommand ------------------------------//
    DxCommand* getDxCommand() {
        return dxCommand_.get();
    }
};

//==============================================================
// ↓ RendererComponentController
//==============================================================
///< summary>
/// RendererComponent管理クラス
/// </summary>
template <typename RenderComponent>
concept IsDerivedIRenderComponent = std::derived_from<RenderComponent, IRendererComponent>;
template <IsDerivedIRenderComponent RenderComponent>
class RendererComponentController
    : public IRendererComponentController {
public:
    RendererComponentController() : IRendererComponentController() {}
    virtual ~RendererComponentController() {}

    ///< summary>
    /// 初期化
    ///</summary>
    virtual void Init() {
        IRendererComponentController::Init();
    }

    ///< summary>
    /// 更新,描画
    ///</summary>
    virtual void RenderFrame() {
        IRendererComponentController::RenderFrame();
    }

    ///< summary>
    /// 終了処理
    ///</summary>
    virtual void Finalize() {
        for (auto& renderer : renderSchedule_) {
            if (renderer) {
                renderer->Finalize();
                renderer.reset();
            }
        }
        IRendererComponentController::Finalize();
    }

protected:
    ///< summary>
    /// 更新
    ///</summary>
    virtual void Update();
    ///< summary>
    /// 描画開始
    ///</summary>
    virtual void StartRender() = 0;
    ///< summary>
    /// RenderScheduleをすべて描画
    ///</summary>
    virtual void RenderAll();
    ///< summary>
    /// 描画終了
    ///</summary>
    virtual void EndRender() = 0;

    ///< summary>
    /// PipelineStateObjectを作成
    ///</summary>
    virtual void CreatePso() = 0;

protected:
    std::vector<std::shared_ptr<RenderComponent>> renderSchedule_;

protected:
public: // ↓ Accessor
    //------------------------------ RenderSchedule ------------------------------//
    void addRenderer(std::shared_ptr<RenderComponent> _renderer) {
        renderSchedule_.emplace_back(_renderer);
    }

    //------------------------------ id ------------------------------//
    static const std::string& getId() {
        return getTypeName<RenderComponent>();
    }
};

template <IsDerivedIRenderComponent RendererCompoenent>
inline void RendererComponentController<RendererCompoenent>::Update() {
    // renderSchedule_ から nullptr または外部参照が無い(renderer.use_count() == 1)レンダラーを削除
    std::erase_if(renderSchedule_, [](std::shared_ptr<RendererCompoenent>& renderer) {
        bool isExists = (!renderer) || (renderer.use_count() >= 1);
        if (!isExists) {
            // nullptr ではないなら 開放してから削除
            if (renderer) {
                renderer.get()->Finalize();
                renderer.reset();
            }
        }
        return !isExists;
    });

    for (std::shared_ptr<RendererCompoenent>& renderer : renderSchedule_) {
        renderer->Update();
    }
}

template <IsDerivedIRenderComponent RendererCompoenent>
inline void RendererComponentController<RendererCompoenent>::RenderAll() {
    for (auto& renderer : renderSchedule_) {
        if (renderer == nullptr) {
            continue;
        }
        renderer->Render();
    }
}
