#pragma once

#include "input/InputManager.h"

#include "component/transform/CameraTransform.h"

#include <memory>

/// <summary>
/// Debug用カメラ
/// </summary>
class DebugCamera {
#pragma region State
    /// <summary>
    /// 状態遷移用の基底クラス
    /// </summary>
    class IState {
    public:
        IState(DebugCamera* host) : host_(host) {};
        virtual ~IState() {};

        virtual void Update() = 0;

    protected:
        DebugCamera* host_;
    };
    /// <summary>
    /// 入力受付状態
    /// </summary>
    class Neutral : public IState {
    public:
        Neutral(DebugCamera* host) : IState(host) {};
        void Update() override;
    };
    /// <summary>
    /// 回転入力状態
    /// </summary>
    class RotationState : public IState {
    public:
        RotationState(DebugCamera* host) : IState(host) {};
        void Update() override;
    };
    /// <summary>
    /// 移動入力状態
    /// </summary>
    class TranslationState : public IState {
        enum TranslationType {
            NONE,
            Z_WHEEL,
            XY_MOUSEMOVE,
            XYZ_ALL
        };

    public:
        TranslationState(DebugCamera* host) : IState(host) {};
        void Update() override;
    };
#pragma endregion
public:
    void Initialize();
    void Finalize();
    void DebugUpdate();
    void Update();

private:
    Vec2f startMousePos_;
    std::unique_ptr<IState> currentState_ = nullptr;
    CameraTransform cameraBuff_;

public:
    void setViewTranslate(const Vec3f& translate) { cameraBuff_.translate = translate; }
    const CameraTransform& getCameraTransform() const { return cameraBuff_; }
    CameraTransform& getCameraTransformRef() { return cameraBuff_; }

    const Vec2f& getStartMousePos() const { return startMousePos_; }
    void setStartMousePos(const Vec2f& pos) { startMousePos_ = pos; }
};
