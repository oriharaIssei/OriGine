#include "ProfilerWindow.h"

#ifdef _DEBUG

/// stl
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>
#include <vector>

/// editor
#include "editor/EditorController.h"

/// util
#include "util/nameof.h"

/// profiler
#include "profiler/AllocationCounter.h"
#include "profiler/Profiler.h"

/// externals
#include "myGui/MyGui.h"

using namespace OriGine;

namespace {

/// <summary>
/// バイト数を "1.23 MB" のような人間可読な文字列に変換する
/// </summary>
/// <param name="_bytes">バイト数</param>
/// <returns>整形済み文字列</returns>
std::string FormatBytes(int64_t _bytes) {
    constexpr const char* kUnits[] = {"B", "KB", "MB", "GB"};
    double value                   = static_cast<double>(_bytes);
    int32_t unitIndex              = 0;
    while (std::fabs(value) >= 1024.0 && unitIndex < 3) {
        value /= 1024.0;
        ++unitIndex;
    }
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.2f %s", value, kUnits[unitIndex]);
    return std::string(buf);
}

/// <summary>
/// リングバッファをImGui::PlotLinesへ渡す際のvalues_offset(最古の要素の位置)を計算する
/// </summary>
/// <param name="_cursor">最新の要素が格納されているインデックス</param>
/// <param name="_size">バッファサイズ</param>
/// <returns>values_offsetに渡す値</returns>
int32_t ComputePlotOffset(size_t _cursor, size_t _size) {
    if (_size == 0) {
        return 0;
    }
    return static_cast<int32_t>((_cursor + 1) % _size);
}

/// <summary>
/// 階層プロファイル木を再帰的にImGuiテーブルへ描画する
/// </summary>
/// <param name="_tree">対象の階層木</param>
/// <param name="_nodeIndex">描画するノードのインデックス</param>
/// <param name="_sortMode">ソート方法 (0:合計 1:自己 2:呼び出し回数 3:名前)</param>
/// <param name="_descending">降順にするか</param>
void DrawTreeNode(const OriGine::Profiling::ProfileThreadTree& _tree, size_t _nodeIndex, int32_t _sortMode, bool _descending) {
    const OriGine::Profiling::ProfileTreeNode& node = _tree.nodes_[_nodeIndex];

    // 子を指定ソート順に並び替える(表示専用のコピーなので描画コストのみ。計測対象には影響しない)
    std::vector<size_t> sortedChildren = node.children_;
    std::sort(sortedChildren.begin(), sortedChildren.end(), [&](size_t _a, size_t _b) {
        const auto& a = _tree.nodes_[_a];
        const auto& b = _tree.nodes_[_b];
        bool less     = false;
        switch (_sortMode) {
        case 1:
            less = a.selfMs_ < b.selfMs_;
            break;
        case 2:
            less = a.callCount_ < b.callCount_;
            break;
        case 3:
            less = a.name_ > b.name_; // 文字列は昇順を基準に反転させて扱う
            break;
        default:
            less = a.totalMs_ < b.totalMs_;
            break;
        }
        return _descending ? !less : less;
    });

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen;
    if (sortedChildren.empty()) {
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }
    const bool open = ImGui::TreeNodeEx(node.name_.c_str(), flags);

    ImGui::TableSetColumnIndex(1);
    ImGui::Text("%d", node.callCount_);
    ImGui::TableSetColumnIndex(2);
    ImGui::Text("%.3f", node.totalMs_);
    ImGui::TableSetColumnIndex(3);
    ImGui::Text("%.3f", node.selfMs_);

    if (open) {
        for (size_t child : sortedChildren) {
            DrawTreeNode(_tree, child, _sortMode, _descending);
        }
        if (!sortedChildren.empty()) {
            ImGui::TreePop();
        }
    }
}

} // namespace

#pragma region "ProfilerWindow"

ProfilerWindow::ProfilerWindow() : Editor::Window(nameof<ProfilerWindow>()) {}
ProfilerWindow::~ProfilerWindow() {}

void ProfilerWindow::Initialize() {
    windowFlags_ = ImGuiWindowFlags_NoCollapse;

    AddArea(std::make_shared<ProfilerOverviewArea>());
    AddArea(std::make_shared<ProfilerTreeArea>());

    OriGine::EditorController::GetInstance()->AddMainMenu(std::make_unique<ProfilerMenu>());
}

void ProfilerWindow::Finalize() {
    Editor::Window::Finalize();
}

#pragma endregion

#pragma region "ProfilerOverview"

ProfilerOverviewArea::ProfilerOverviewArea() : Editor::Area(nameof<ProfilerOverviewArea>()) {}
ProfilerOverviewArea::~ProfilerOverviewArea() {}

void ProfilerOverviewArea::Initialize() {
    AddRegion(std::make_shared<ProfilerOverviewRegion>());
}
void ProfilerOverviewArea::Finalize() {}

ProfilerOverviewRegion::ProfilerOverviewRegion() : Editor::Region(nameof<ProfilerOverviewRegion>()) {}
ProfilerOverviewRegion::~ProfilerOverviewRegion() {}
void ProfilerOverviewRegion::Initialize() {}
void ProfilerOverviewRegion::Finalize() {}

void ProfilerOverviewRegion::DrawGui() {
    Profiler* profiler = Profiler::GetInstance();

    ///-------------------------------------------------------------
    // フレームタイム
    ///-------------------------------------------------------------
    ImGui::Text("Frame Time");
    ImGui::Separator();

    const FrameTimeStats frameStats = profiler->GetFrameTimeStats();
    ImGui::Text("Average: %.3f ms (%.1f FPS)", frameStats.averageMs_,
        frameStats.averageMs_ > 0.0f ? 1000.0f / frameStats.averageMs_ : 0.0f);
    ImGui::Text("99th Percentile: %.3f ms", frameStats.p99Ms_);
    ImGui::Text("Max: %.3f ms", frameStats.maxMs_);

    const auto& history = profiler->GetFrameTimeHistory();
    ImGui::PlotLines(
        "##FrameTimeGraph",
        history.data(),
        static_cast<int32_t>(history.size()),
        ComputePlotOffset(profiler->GetFrameTimeHistoryCursor(), history.size()),
        nullptr,
        0.0f,
        (std::max)(frameStats.maxMs_ * 1.2f, 1.0f),
        ImVec2(0, 80));

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ///-------------------------------------------------------------
    // アロケーション (Debug/Develop構成のみ意味のある値になる)
    ///-------------------------------------------------------------
    ImGui::Text("Allocations (Debug / Develop only)");
    ImGui::Separator();

    const auto& allocStats = AllocationCounter::GetLastFrameStats();
    ImGui::Text("Alloc Count: %llu / frame", static_cast<unsigned long long>(allocStats.allocCount_));
    ImGui::Text("Free Count : %llu / frame", static_cast<unsigned long long>(allocStats.freeCount_));
    ImGui::Text("Alloc Bytes: %s / frame", FormatBytes(static_cast<int64_t>(allocStats.allocBytes_)).c_str());
    ImGui::Text("Peak Usage : %s", FormatBytes(allocStats.peakBytes_).c_str());

    const auto& allocHistory = AllocationCounter::GetHistory();
    std::vector<float> allocCountGraph(allocHistory.size());
    std::vector<float> allocBytesGraph(allocHistory.size());
    for (size_t i = 0; i < allocHistory.size(); ++i) {
        allocCountGraph[i] = static_cast<float>(allocHistory[i].allocCount_);
        allocBytesGraph[i] = static_cast<float>(allocHistory[i].allocBytes_);
    }
    const int32_t allocOffset = ComputePlotOffset(AllocationCounter::GetHistoryCursor(), allocHistory.size());

    ImGui::Text("Alloc Count / Frame");
    ImGui::PlotLines(
        "##AllocCountGraph", allocCountGraph.data(), static_cast<int32_t>(allocCountGraph.size()),
        allocOffset, nullptr, 0.0f, (std::numeric_limits<float>::max)(), ImVec2(0, 60));

    ImGui::Text("Alloc Bytes / Frame");
    ImGui::PlotLines(
        "##AllocBytesGraph", allocBytesGraph.data(), static_cast<int32_t>(allocBytesGraph.size()),
        allocOffset, nullptr, 0.0f, (std::numeric_limits<float>::max)(), ImVec2(0, 60));
}

#pragma endregion

#pragma region "ProfilerTree"

ProfilerTreeArea::ProfilerTreeArea() : Editor::Area(nameof<ProfilerTreeArea>()) {}
ProfilerTreeArea::~ProfilerTreeArea() {}
void ProfilerTreeArea::Initialize() {
    AddRegion(std::make_shared<ProfilerTreeRegion>());
}
void ProfilerTreeArea::Finalize() {}

ProfilerTreeRegion::ProfilerTreeRegion() : Editor::Region(nameof<ProfilerTreeRegion>()) {}
ProfilerTreeRegion::~ProfilerTreeRegion() {}
void ProfilerTreeRegion::Initialize() {}
void ProfilerTreeRegion::Finalize() {}

void ProfilerTreeRegion::DrawGui() {
    const char* sortItems[] = {"Total Time", "Self Time", "Call Count", "Name"};
    ImGui::SetNextItemWidth(160);
    ImGui::Combo("Sort By", &sortMode_, sortItems, IM_ARRAYSIZE(sortItems));
    ImGui::SameLine();
    ImGui::Checkbox("Descending", &sortDescending_);

    ImGui::Spacing();

    // 直前フレームの階層プロファイル木を構築する(Editor描画時のみ。動的確保を伴うがフレーム計測には影響しない)
    const std::vector<OriGine::Profiling::ProfileThreadTree> trees = Profiler::GetInstance()->BuildLastFrameTrees();

    if (trees.empty()) {
        ImGui::TextDisabled("No profile data recorded yet.");
        return;
    }

    const ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable;

    for (const OriGine::Profiling::ProfileThreadTree& tree : trees) {
        if (tree.nodes_.empty()) {
            continue;
        }

        // イベントが破棄されたフレームは Begin/End の対応が壊れており、以下の数値は信用できない。
        // 黙って壊れた木を見せるのが計測器として最悪なので、必ず警告を出す。
        if (tree.droppedEventCount_ > 0) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.42f, 0.32f, 1.0f));
            ImGui::TextWrapped(
                "[%s] イベントバッファ超過: %u 件を破棄しました。この木の数値は信用できません。"
                "EngineConfig.h の kEventStreamCapacity を増やすか、PROFILE_SCOPE を減らしてください。",
                tree.threadLabel_.c_str(),
                tree.droppedEventCount_);
            ImGui::PopStyleColor();
        }

        const std::string tableId = tree.threadLabel_ + "##ProfilerTreeTable";
        if (ImGui::BeginTable(tableId.c_str(), 4, tableFlags)) {
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Calls", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableSetupColumn("Total (ms)", ImGuiTableColumnFlags_WidthFixed, 90.0f);
            ImGui::TableSetupColumn("Self (ms)", ImGuiTableColumnFlags_WidthFixed, 90.0f);
            ImGui::TableHeadersRow();

            DrawTreeNode(tree, 0, sortMode_, sortDescending_);

            ImGui::EndTable();
        }
        ImGui::Spacing();
    }
}

#pragma endregion

#pragma region "ProfilerMenu"

ProfilerMenu::ProfilerMenu() : Editor::Menu(nameof<ProfilerMenu>()) {}
ProfilerMenu::~ProfilerMenu() {}
void ProfilerMenu::Initialize() {
    AddMenuItem(std::make_shared<ProfilerWindowOpenItem>());
}
void ProfilerMenu::Finalize() {}

ProfilerWindowOpenItem::ProfilerWindowOpenItem() : Editor::MenuItem("ProfilerWindowOpen") {}
ProfilerWindowOpenItem::~ProfilerWindowOpenItem() {}
void ProfilerWindowOpenItem::Initialize() {}
void ProfilerWindowOpenItem::DrawGui() {
    ProfilerWindow* window = OriGine::EditorController::GetInstance()->GetWindow<ProfilerWindow>();
    const bool isOpenWindow = window->IsOpen();

    if (ImGui::MenuItem("Profiler", nullptr, isOpenWindow, !isOpenWindow)) {
        window->WindowOpenMessage();
    }
}
void ProfilerWindowOpenItem::Finalize() {}

#pragma endregion

#endif // _DEBUG
