#pragma once

/// stl
#include <memory>
/// ECS
#include "component/transform/CameraTransform.h"

/// math
#include <Vector2.h>

namespace OriGine {

/// <summary>
/// Debug用カメラ
/// </summary>
class DebugCamera {

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

public:
    void Initialize();
    void Finalize();
    void DebugUpdate();
    void Update();

private:
    Vec2f startMousePos_;
    ::std::unique_ptr<IState> currentState_ = nullptr;
    CameraTransform cameraBuff_;

public:
    void SetViewTranslate(const Vec3f& translate) { cameraBuff_.translate = translate; }
    const CameraTransform& GetCameraTransform() const { return cameraBuff_; }
    CameraTransform& GetCameraTransformRef() { return cameraBuff_; }

    const Vec2f& GetStartMousePos() const { return startMousePos_; }
    void SetStartMousePos(const Vec2f& pos) { startMousePos_ = pos; }
};

} // namespace OriGine
