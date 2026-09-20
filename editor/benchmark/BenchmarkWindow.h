#pragma once

#ifdef ORIGINE_EDITOR_ENABLED

/// interface
#include "editor/IEditor.h"

/// stl
#include <cstdint>
#include <memory>

/// engine
#include "benchmark/BenchmarkTypes.h"

namespace OriGine {
class Scene;
}

/// <summary>
/// エディタ上から衝突判定負荷再現用のベンチマークシーンを生成・実行するウィンドウ.
/// entities/extent/radius/seed をGUIから編集してシーンを生成でき、生成後はウィンドウが
/// 開いている間ずっとシーンを更新し続ける(自動終了はしない。ライブの数値は既存のProfilerWindowで
/// 確認する想定). 現在の実行状況をCSVへスナップショット出力するボタンも備える.
/// </summary>
class BenchmarkWindow
    : public Editor::Window {
public:
    BenchmarkWindow();
    ~BenchmarkWindow() override;

    void Initialize() override;
    void Finalize() override;
};

/// <summary>ベンチマーク操作パネルを配置するエリア</summary>
class BenchmarkControlArea
    : public Editor::Area {
public:
    BenchmarkControlArea();
    ~BenchmarkControlArea() override;
    void Initialize() override;
    void Finalize() override;
};

/// <summary>
/// パラメータ編集・シーン生成/破棄・CSV出力を行うリージョン.
/// Editor::Region には毎フレーム呼ばれるUpdate相当のフックが無いため、
/// DrawGui() 内でシーンの更新(Scene::Update())も併せて行っている
/// (これによりウィンドウを開いている間だけベンチマークシーンが"ライブ"に動く).
/// </summary>
class BenchmarkControlRegion
    : public Editor::Region {
public:
    BenchmarkControlRegion();
    ~BenchmarkControlRegion() override;
    void Initialize() override;
    void DrawGui() override;
    void Finalize() override;

private:
    /// <summary>現在のGUI入力値でベンチマークシーンを(再)生成する</summary>
    void GenerateScene();
    /// <summary>ベンチマークシーンを破棄する</summary>
    void ClearScene();
    /// <summary>現在のProfiler/AllocationCounterの状況をCSVへスナップショット出力する</summary>
    void ExportCsv();

private:
    // --- GUI編集用パラメータ(生成ボタンを押すまでシーンには反映されない) ---
    int32_t entityCountInput_ = 10000;
    float extentInput_        = 500.0f;
    float radiusInput_        = 1.0f;
    int32_t seedInput_        = 12345;

    char csvPathBuffer_[260] = "./generated/benchmark/editor_result.csv";

    // --- 実行中のベンチマークシーン ---
    ::std::unique_ptr<OriGine::Scene> scene_;
    OriGine::Benchmark::BenchmarkConfig lastGeneratedConfig_;
    bool hasScene_ = false;
};

#endif // _DEBUG
