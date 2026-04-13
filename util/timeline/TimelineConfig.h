#pragma once

/// @file TimelineConfig.h
/// @brief タイムラインエディタ用の定数定義

namespace TimelineConfig {

/// @brief タイムラインのフレーム高さ
constexpr float FRAME_HEIGHT = 20.0f;

/// @brief ノードボタンのサイズ
constexpr float BUTTON_SIZE = 10.0f;

/// @brief コピー時のタイムオフセット
constexpr float COPY_TIME_OFFSET = 0.01f;

/// @brief ノードカラー (RGBA)
constexpr unsigned int NODE_COLOR_R = 200;
constexpr unsigned int NODE_COLOR_G = 200;
constexpr unsigned int NODE_COLOR_B = 200;
constexpr unsigned int NODE_COLOR_A = 255;

/// @brief ノードの最小アルファ値 (カラーノード用)
constexpr float NODE_MIN_ALPHA = 0.1f;

/// @brief ドラッグの速度
constexpr float DRAG_SPEED = 0.1f;

} // namespace TimelineConfig
