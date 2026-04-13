#pragma once

#ifdef _DEBUG

/// stl
#include <atomic>
#include <thread>

/// editor
#include "editor/IEditor.h"
class SceneEditorWindow;

/// <remarks>
/// Developをビルド・実行するためのエリア
/// </remarks>
class DevelopControlArea
    : public Editor::Area {
public:
    DevelopControlArea(SceneEditorWindow* _parentWindow);
    ~DevelopControlArea() override;

    void Initialize() override;
    // void DrawGui() override;
    // void Finalize() override;

    /// <summary>
    /// UIを描画する
    /// </summary>
    class ControlRegion
        : public Editor::Region {
    public:
        ControlRegion(DevelopControlArea* _parentArea);
        ~ControlRegion() override;
        void Initialize() override;
        void DrawGui() override;
        void Finalize() override;

    private:
        DevelopControlArea* parentArea_ = nullptr; // 親エリアへのポインタ
    };

private:
    /// <summary>
    /// ビルドスレッドの完了を待機する
    /// </summary>
    void WaitForBuildThread();

    SceneEditorWindow* parentWindow_ = nullptr; // 親ウィンドウへのポインタ

    ::std::thread buildThread_; // ビルド用スレッド
    ::std::atomic<bool> isBuilding_        = false; // ビルド中フラグ
    ::std::atomic<bool> shouldCancelBuild_ = false; // キャンセル要求フラグ

    const ::std::string buildTool_        = "msbuild"; // ビルドツールの名前
    const ::std::string projectDirectory_ = "project";
    const ::std::string projectName_      = "OriGine.sln";
    const ::std::string configuration     = "Develop";
    const ::std::string platform          = "x64";

    const ::std::string exePath_ = "/../generated/output/" + configuration + "/OriGineApp.exe"; // 実行ファイルのパス

public:
    SceneEditorWindow* GetParentWindow() const {
        return parentWindow_;
    }
};

#endif // _DEBUG
