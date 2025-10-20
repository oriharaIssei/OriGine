#pragma once

#include <array>
#include <string>

/// <summary>
/// システムの種類(この種類によって処理順序が決定する)
/// </summary>
enum class SystemCategory {
    Initialize, // 初期化処理

    Input, // Userによる入力に対する処理
    StateTransition, // 入力等による状態遷移の処理
    Movement, // 移動や行動の処理
    Collision, // 衝突判定処理

    Effect, // エフェクト処理

    Render, // 更新処理のあとに 描画処理
    PostRender, // 描画処理のあとに 処理

    Count
};

static const std::array<std::string, static_cast<int>(SystemCategory::Count)> SystemCategoryString = {
    "Initialize",
    "Input",
    "StateTransition",
    "Movement",
    "Collision",
    "Effect",
    "Render",
    "PostRender"};

namespace std {
string to_string(const SystemCategory& _category);
}
