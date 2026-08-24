#include "ProfileTree.h"

/// stl
#include <limits>

/// engine
#include "ProfileClock.h"

namespace OriGine::Profiling {

namespace {
/// <summary>
/// 木構築中に使用するスタックエントリ
/// </summary>
struct StackEntry {
    size_t nodeIndex_;
    uint64_t startTicks_;
    double childAccumMs_; // このスコープの子の合計時間(自己時間の算出に使う)
};

constexpr size_t kInvalidIndex = (std::numeric_limits<size_t>::max)();
} // namespace

ProfileThreadTree BuildProfileTree(std::span<const ProfileEvent> _events, const std::string& _threadLabel) {
    ProfileThreadTree tree;
    tree.threadLabel_ = _threadLabel;

    ProfileTreeNode root;
    root.name_ = _threadLabel;
    tree.nodes_.push_back(std::move(root));

    std::vector<StackEntry> stack;
    stack.reserve(64);
    stack.push_back(StackEntry{0, 0, 0.0});

    const uint64_t frameStartTicks = _events.empty() ? 0 : _events.front().timestampTicks_;
    const uint64_t frameEndTicks   = _events.empty() ? 0 : _events.back().timestampTicks_;

    for (const ProfileEvent& ev : _events) {
        if (ev.type_ == ProfileEventType::kBegin) {
            const size_t parentIndex = stack.back().nodeIndex_;

            // 同じ親の下に同名の子が既にあれば再利用する(呼び出し回数の集計のため)
            size_t childIndex = kInvalidIndex;
            for (size_t idx : tree.nodes_[parentIndex].children_) {
                if (tree.nodes_[idx].name_ == ev.name_) {
                    childIndex = idx;
                    break;
                }
            }
            if (childIndex == kInvalidIndex) {
                ProfileTreeNode node;
                node.name_ = ev.name_;
                childIndex = tree.nodes_.size();
                tree.nodes_.push_back(std::move(node));
                tree.nodes_[parentIndex].children_.push_back(childIndex);
            }

            stack.push_back(StackEntry{childIndex, ev.timestampTicks_, 0.0});
        } else {
            if (stack.size() <= 1) {
                // Beginと対応しないEnd(バッファ境界で切れた等)は無視する
                continue;
            }
            const StackEntry entry = stack.back();
            stack.pop_back();

            const double elapsedMs = Clock::TicksToMilliseconds(static_cast<int64_t>(ev.timestampTicks_ - entry.startTicks_));

            ProfileTreeNode& node = tree.nodes_[entry.nodeIndex_];
            node.callCount_ += 1;
            node.totalMs_ += elapsedMs;
            node.selfMs_ += elapsedMs - entry.childAccumMs_;

            stack.back().childAccumMs_ += elapsedMs;
        }
    }

    // ルートノードは記録されている全体区間(先頭イベント〜末尾イベント)を表す
    ProfileTreeNode& rootNode = tree.nodes_[0];
    rootNode.callCount_       = 1;
    rootNode.totalMs_         = Clock::TicksToMilliseconds(static_cast<int64_t>(frameEndTicks - frameStartTicks));
    double childrenTotalMs    = 0.0;
    for (size_t idx : rootNode.children_) {
        childrenTotalMs += tree.nodes_[idx].totalMs_;
    }
    rootNode.selfMs_ = rootNode.totalMs_ - childrenTotalMs;

    return tree;
}

} // namespace OriGine::Profiling
