#pragma once

/// stl
#include <memory>
/// ECS
#include "component/transform/CameraTransform.h"

/// math
#include <Vector2.h>

namespace OriGine {

/// <summary>
/// デバッグ用カメラクラス.
/// Alt + マウス操作によるカメラの自由移動を可能にする.
/// </summary>
class DebugCamera {

    /// <summary>
    /// カメラの状態（入力待ち、回転中、移動中など）を管理する基底クラス.
    /// </summary>
    class IState {
    public:
        IState(DebugCamera* host) : host_(host) {};
        virtual ~IState() {};

        /// <summary>
        /// 各状態における更新処理.
        /// </summary>
        virtual void Update() = 0;

    protected:
        /// <summary>持ち主となるカメラへのポインタ</summary>
        DebugCamera* host_;
    };

    /// <summary>
    /// 入力受付待機状態. Alt + 各種マウスボタンで他の状態に遷移する.
    /// </summary>
    class Neutral : public IState {
    public:
        Neutral(DebugCamera* host) : IState(host) {};
        void Update() override;
    };

    /// <summary>
    /// カメラ回転状態. Alt + 右ドラッグで視点を回転させる.
    /// </summary>
    class RotationState : public IState {
    public:
        RotationState(DebugCamera* host) : IState(host) {};
        void Update() override;
    };

    /// <summary>
    /// カメラ並行移動・ズーム状態. Alt + 左ドラッグ/ホイールで位置を変更する.
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
    /// <summary>
    /// 初期化処理を行う.
    /// </summary>
    void Initialize();

    /// <summary>
    /// 終了処理を行う.
    /// </summary>
    void Finalize();

    /// <summary>
    /// ImGui によるデバッグ表示・操作を行う.
    /// </summary>
    void DebugUpdate();

    /// <summary>
    /// カメラの更新（ステート更新と行列計算）を行う.
    /// </summary>
    void Update();

private:
    /// <summary>ドラッグ開始時のマウス座標</summary>
    Vec2f startMousePos_;
    /// <summary>現在の操作状態（ステート）</summary>
    ::std::unique_ptr<IState> currentState_ = nullptr;
    /// <summary>カメラのトランスフォーム情報</summary>
    CameraTransform cameraBuff_;

public:
    /// <summary>
    /// カメラの座標を直接設定する.
    /// </summary>
    /// <param name="translate">設定する座標</param>
    void SetViewTranslate(const Vec3f& translate) { cameraBuff_.translate = translate; }

    /// <summary>
    /// カメラのトランスフォーム情報を取得する（読み取り専用）.
    /// </summary>
    /// <returns>カメラトランスフォーム</returns>
    const CameraTransform& GetCameraTransform() const { return cameraBuff_; }

    /// <summary>
    /// カメラのトランスフォーム情報を取得する（参照）.
    /// </summary>
    /// <returns>カメラトランスフォームへの参照</returns>
    CameraTransform& GetCameraTransformRef() { return cameraBuff_; }

    /// <summary>
    /// マウスの開始座標を取得する.
    /// </summary>
    /// <returns>マウス座標</returns>
    const Vec2f& GetStartMousePos() const { return startMousePos_; }

    /// <summary>
    /// マウスの開始座標を設定する.
    /// </summary>
    /// <param name="pos">マウス座標</param>
    void SetStartMousePos(const Vec2f& pos) { startMousePos_ = pos; }
};

} // namespace OriGine
