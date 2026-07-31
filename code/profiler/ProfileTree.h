#pragma once

/// stl
#include <cstdint>
#include <span>
#include <string>
#include <vector>

/// engine
#include "ProfileEvent.h"

namespace OriGine::Profiling {

/// <summary>
/// 階層プロファイル木の1ノード. 同じ親の下で同名のスコープは1つのノードに集約される
/// (ループ内で複数回呼ばれるスコープの呼び出し回数を数えられるようにするため).
/// </summary>
struct ProfileTreeNode {
    std::string name_;
    int32_t callCount_ = 0;
    double totalMs_    = 0.0; // 子を含めた合計時間
    double selfMs_     = 0.0; // 自分自身の処理のみの時間(子の合計時間を除く)
    std::vector<size_t> children_;
};

/// <summary>
/// 1スレッド分の階層プロファイル木. nodes_[0] がそのスレッドのフレーム全体を表すルートノード.
/// </summary>
struct ProfileThreadTree {
    std::string threadLabel_;
    std::vector<ProfileTreeNode> nodes_;

    /// <summary>
    /// このフレームでバッファ上限により破棄されたイベント数.
    /// 0以外なら Begin/End の対応が壊れているため、この木の数値は信用できない.
    /// </summary>
    uint32_t droppedEventCount_ = 0;
};

/// <summary>
/// Begin/Endイベント列から階層プロファイル木を構築する.
/// これはEditorのUI表示専用の変換処理であり、計測対象のフレーム処理には一切影響しない
/// (呼び出しはEditor描画時にのみ行われ、フレームの計測パス上では実行されない).
/// </summary>
/// <param name="_events">対象スレッドの1フレーム分のイベント列</param>
/// <param name="_threadLabel">表示用スレッド名</param>
/// <returns>構築された階層プロファイル木</returns>
ProfileThreadTree BuildProfileTree(std::span<const ProfileEvent> _events, const std::string& _threadLabel);

} // namespace OriGine::Profiling
