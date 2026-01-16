#pragma once

#ifdef _DEBUG

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
    SceneEditorWindow* parentWindow_ = nullptr; // 親ウィンドウへのポインタ

    ::std::atomic<bool> isBuilding_ = false;

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
