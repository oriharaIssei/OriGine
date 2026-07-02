#pragma once

#ifdef _DEBUG

/// engine
#include "camera/debugCamera/DebugCamera.h"

/// editor
#include "editor/IEditor.h"
#include "editor/sceneEditor/SceneEditor.h"
#include "editor/tool/ObjectPicker.h"

/// ECS
// component
#include "component/transform/CameraTransform.h"
#include "component/transform/Transform.h"
#include "component/transform/Transform2d.h"

/// externals
#include <imgui/ImGuizmo/ImGuizmo.h>

/// <summary>
/// シーンビューエリア(シーンのDebug描画を確認する)
/// </summary>
class SceneViewArea
    : public Editor::Area {
public:
    SceneViewArea(SceneEditorWindow* _parentWindow);
    void Initialize() override;
    void DrawGui() override;
    void Finalize() override;

private:
    enum class ToolInteractionType {
        None,
        Gizmo,
        Camera,
        ObjectPicker
    };

private:
    /// <summary>
    /// 入力処理(GuizmoかカメラかObject選択かを入力で判断する)
    /// </summary>
    ToolInteractionType DetermineInteractionType();

    /// <summary>
    /// Gizmoの設定を行う
    /// </summary>
    /// <param name="_sceneViewPos"></param>
    /// <param name="_originalResolution"></param>
    void SetupGizmoSettings(const ImVec2& _sceneViewPos);
    /// <summary>
    /// Gizmoの対象Transformを取得する
    /// </summary>
    /// <returns></returns>
    OriGine::Transform* GetTargetTransform();
    /// <summary>
    /// Gizmoの対象 Transform2d(2D) を取得する。無ければ nullptr。
    /// </summary>
    OriGine::Transform2d* GetTargetTransform2d();
    /// <summary>
    /// Gizmoの操作モードを決定する
    /// </summary>
    void DetermineGizmoOperation();
    /// <summary>
    /// Gizmoの操作を適用する
    /// </summary>
    /// <param name="transform"></param>
    /// <param name="cameraTrans"></param>
    /// <returns></returns>
    bool ApplyGizmoManipulation(OriGine::Transform* transform, const OriGine::CameraTransform& cameraTrans);
    /// <summary>
    /// 2D(Transform2d)用の Gizmo 操作を適用する。スクリーン空間(正射影)で translate を編集する。
    /// </summary>
    /// <param name="_transform2d">操作対象の Transform2d</param>
    /// <returns>操作されたら true</returns>
    bool ApplyGizmoManipulation2d(OriGine::Transform2d* _transform2d);
    /// <summary>
    /// Gizmoのコマンド管理
    /// </summary>
    /// <param name="editEntity"></param>
    /// <param name="transform"></param>
    /// <param name="isManipulated"></param>
    void ManageGizmoCommands(OriGine::Entity* editEntity, OriGine::Transform* transform, bool isManipulated);
    /// <summary>
    /// 2D(Transform2d)用の Gizmo コマンド管理（Undo/Redo）
    /// </summary>
    void ManageGizmoCommands2d(OriGine::Entity* editEntity, OriGine::Transform2d* transform2d, bool isManipulated);

    /// <summary>
    /// シーンの描画
    /// </summary>
    /// <param name="_sceneViewPos"></param>
    /// <param name="_originalResolution"></param>
    void UseImGuizmo(const ImVec2& _sceneViewPos);

    /// <summary>
    /// シーンの描画
    void DrawScene();
    /// </summary>
private:
    ImGuizmo::OPERATION currentGizmoOperation_;
    // Transform と Transform2d を両方持つエンティティで、どちらを Gizmo 対象にするか
    // (true=2D)。片方しか無い場合はそれが自動的に使われる。
    bool gizmoUse2d_ = false;

    SceneEditorWindow* parentWindow_; // 親ウィンドウへのポインタ
    std::unique_ptr<OriGine::ObjectPicker> objectPicker_; // オブジェクトピッカー

    ::std::unique_ptr<OriGine::DebugCamera> debugCamera_; // デバッグカメラ
};

#endif
