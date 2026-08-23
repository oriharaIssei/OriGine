#include "editor/benchmark/BenchmarkWindow.h"

#ifdef _DEBUG

/// stl
#include <algorithm>
#include <cstring>
#include <vector>

/// engine
#include "Engine.h"
#include "directX12/RenderTexture.h"
#include "input/InputManager.h"
#include "scene/Scene.h"
#include "winApp/WinApp.h"

/// editor
#include "editor/EditorController.h"

/// benchmark
#include "benchmark/BenchmarkCsv.h"
#include "benchmark/BenchmarkSceneBuilder.h"

/// profiler
#include "profiler/AllocationCounter.h"
#include "profiler/CallCounter.h"
#include "profiler/Profiler.h"

/// util
#include "util/nameof.h"

/// externals
#include "logger/Logger.h"
#include "myGui/MyGui.h"

using namespace OriGine;

#pragma region "BenchmarkWindow"

BenchmarkWindow::BenchmarkWindow() : Editor::Window(nameof<BenchmarkWindow>()) {}
BenchmarkWindow::~BenchmarkWindow() {}

void BenchmarkWindow::Initialize() {
    windowFlags_ = ImGuiWindowFlags_NoCollapse;

    AddArea(std::make_shared<BenchmarkControlArea>());
}

void BenchmarkWindow::Finalize() {
    Editor::Window::Finalize();
}

#pragma endregion

#pragma region "BenchmarkControlArea"

BenchmarkControlArea::BenchmarkControlArea() : Editor::Area(nameof<BenchmarkControlArea>()) {}
BenchmarkControlArea::~BenchmarkControlArea() {}

void BenchmarkControlArea::Initialize() {
    AddRegion(std::make_shared<BenchmarkControlRegion>());
}
void BenchmarkControlArea::Finalize() {}

#pragma endregion

#pragma region "BenchmarkControlRegion"

BenchmarkControlRegion::BenchmarkControlRegion() : Editor::Region(nameof<BenchmarkControlRegion>()) {}
BenchmarkControlRegion::~BenchmarkControlRegion() {}

void BenchmarkControlRegion::Initialize() {}

void BenchmarkControlRegion::Finalize() {
    ClearScene();
}

void BenchmarkControlRegion::GenerateScene() {
    ClearScene();

    Benchmark::BenchmarkConfig config;
    config.entityCount = static_cast<uint32_t>((std::max)(entityCountInput_, 0));
    config.extent       = (std::max)(extentInput_, 0.0f);
    config.radius        = (std::max)(radiusInput_, 0.0f);
    config.seed           = static_cast<uint32_t>(seedInput_);
    // frames/warmup はCLIベンチ専用のパラメータであり、エディタでのライブ実行では使用しない

    scene_ = std::make_unique<Scene>("__EditorBenchmarkScene__");
    scene_->SetInputDevices(
        InputManager::GetInstance()->GetKeyboard(),
        InputManager::GetInstance()->GetMouse(),
        InputManager::GetInstance()->GetGamePad());
    scene_->Initialize();
    scene_->GetSceneView()->Resize(Engine::GetInstance()->GetWinApp()->GetWindowSize());

    Benchmark::BuildBenchmarkScene(scene_.get(), config);

    lastGeneratedConfig_ = config;
    hasScene_             = true;
}

void BenchmarkControlRegion::ClearScene() {
    if (scene_) {
        scene_->Finalize();
        scene_.reset();
    }
    hasScene_ = false;
}

void BenchmarkControlRegion::ExportCsv() {
    Benchmark::BenchmarkSummary summary;
    summary.config_ = lastGeneratedConfig_;

    const FrameTimeStats frameStats = Profiler::GetInstance()->GetFrameTimeStats();
    summary.avgFrameMs_ = frameStats.averageMs_;
    summary.p99FrameMs_ = frameStats.p99Ms_;
    summary.maxFrameMs_ = frameStats.maxMs_;

    const auto& frameHistory = Profiler::GetInstance()->GetFrameTimeHistory();
    const auto& allocHistory = AllocationCounter::GetHistory();

    std::vector<Benchmark::BenchmarkFrameRecord> frameRows;
    frameRows.reserve(frameHistory.size());
    double allocCountSum = 0.0;
    double allocBytesSum = 0.0;
    uint32_t validCount  = 0;
    for (size_t i = 0; i < frameHistory.size(); ++i) {
        if (frameHistory[i] <= 0.0f) {
            continue; // 未記録のリングバッファスロットはスキップする(Profiler::GetFrameTimeStatsと同じ扱い)
        }
        Benchmark::BenchmarkFrameRecord rec;
        rec.frameIndex_ = static_cast<uint32_t>(i);
        rec.frameMs_    = static_cast<double>(frameHistory[i]);
        rec.allocCount_ = allocHistory[i].allocCount_;
        rec.allocBytes_ = allocHistory[i].allocBytes_;
        frameRows.push_back(rec);

        allocCountSum += static_cast<double>(allocHistory[i].allocCount_);
        allocBytesSum += static_cast<double>(allocHistory[i].allocBytes_);
        ++validCount;
    }

    summary.sampleFrameCount_       = validCount;
    summary.avgAllocCountPerFrame_ = validCount > 0 ? allocCountSum / static_cast<double>(validCount) : 0.0;
    summary.avgAllocBytesPerFrame_ = validCount > 0 ? allocBytesSum / static_cast<double>(validCount) : 0.0;
    summary.config_.frames         = validCount; // このスナップショットが対象にしたフレーム数
    summary.config_.warmup         = 0; // ライブスナップショットにwarmup除外の概念は無い

    // スコープ別集計はこのボタンを押した瞬間の1フレーム分のスナップショット。
    // 明示的なユーザー操作でのみ発生する一度きりの処理のため、木構築による動的確保を許容する
    // (計測ループの中で毎フレーム呼ぶCLIベンチとは異なり、ここでの確保はプロファイラの数値を汚さない)。
    const std::vector<Profiling::ProfileThreadTree> trees = Profiler::GetInstance()->BuildLastFrameTrees();
    std::vector<Benchmark::BenchmarkScopeStat> scopeRows;
    for (const Profiling::ProfileThreadTree& tree : trees) {
        for (size_t i = 1; i < tree.nodes_.size(); ++i) { // index 0はスレッドルートなので除く
            const Profiling::ProfileTreeNode& node = tree.nodes_[i];
            Benchmark::BenchmarkScopeStat stat;
            stat.name_             = node.name_;
            stat.callCount_        = static_cast<uint64_t>(node.callCount_);
            stat.totalMsSum_       = node.totalMs_;
            stat.selfMsSum_        = node.selfMs_;
            stat.frameSampleCount_ = 1;
            scopeRows.push_back(std::move(stat));
        }
    }

    // 呼び出し回数の内訳もスコープ別集計と同じく「このボタンを押した瞬間の1フレーム分」のスナップショット。
    // CallCounter::GetLastFrameStats()が返す配列はハンドル=添字で安定しているため名前検索は不要だが、
    // ここは毎フレーム呼ばれるホットパスではない一度きりのUI操作なので、素直にvectorへ詰め替えてよい。
    const CallCounter::FrameStat* counterStats = nullptr;
    const size_t counterStatCount               = CallCounter::GetLastFrameStats(&counterStats);
    std::vector<Benchmark::BenchmarkCounterStat> counterRows;
    counterRows.reserve(counterStatCount);
    for (size_t i = 0; i < counterStatCount; ++i) {
        Benchmark::BenchmarkCounterStat stat;
        stat.name_             = counterStats[i].name_;
        stat.totalCount_       = counterStats[i].count_;
        stat.frameSampleCount_ = 1;
        counterRows.push_back(std::move(stat));
    }

    const std::string csvPath = csvPathBuffer_;
    if (Benchmark::WriteBenchmarkCsv(csvPath, summary, scopeRows, frameRows, counterRows)) {
        LOG_INFO("BenchmarkControlRegion: exported CSV snapshot to '{}'.", csvPath);
    } else {
        LOG_ERROR("BenchmarkControlRegion: failed to export CSV snapshot to '{}'.", csvPath);
    }
}

void BenchmarkControlRegion::DrawGui() {
    ImGui::Text("Benchmark Scene Parameters");
    ImGui::Separator();
    ImGui::InputInt("Entities", &entityCountInput_);
    ImGui::InputFloat("Extent", &extentInput_, 0.0f, 0.0f, "%.2f");
    ImGui::InputFloat("Radius", &radiusInput_, 0.0f, 0.0f, "%.3f");
    ImGui::InputInt("Seed", &seedInput_);

    entityCountInput_ = (std::max)(entityCountInput_, 0);
    extentInput_       = (std::max)(extentInput_, 0.0f);
    radiusInput_        = (std::max)(radiusInput_, 0.0f);

    ImGui::Spacing();
    if (ImGui::Button("Generate Scene")) {
        GenerateScene();
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear Scene")) {
        ClearScene();
    }

    ImGui::Spacing();
    ImGui::Separator();

    if (hasScene_) {
        ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f),
            "Scene active: %u entities (extent=%.1f, radius=%.2f, seed=%u)",
            lastGeneratedConfig_.entityCount, lastGeneratedConfig_.extent,
            lastGeneratedConfig_.radius, lastGeneratedConfig_.seed);
        ImGui::TextDisabled("Open the Profiler window to watch live frame time / allocation / scope stats.");
    } else {
        ImGui::TextDisabled("No benchmark scene generated yet.");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::InputText("CSV Path", csvPathBuffer_, sizeof(csvPathBuffer_));
    if (ImGui::Button("Export CSV Snapshot")) {
        ExportCsv();
    }

    // Editor::Region には毎フレーム呼ばれるUpdate相当のフックが存在しないため、
    // ウィンドウが開いている間だけベンチマークシーンを"ライブ"に動かすためにここで更新する。
    if (hasScene_ && scene_) {
        scene_->Update();
    }
}

#pragma endregion

#endif // _DEBUG
