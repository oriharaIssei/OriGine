#pragma once
#include "math/Vector4.h"
#include <stddef.h>
#include <stdint.h>

namespace OriGine::EditorConfig {

// 検索窓などのバッファサイズ
constexpr size_t kSearchBufferSize = 256;

// レイアウト用定数
namespace Layout {
constexpr float kListHeightPaddingMultiplier = 6.0f;
constexpr float kItemHeightMultiplier        = 2.0f;
} // namespace Layout

// ビューポート関連
namespace Viewport {
constexpr float kMinSizeThreshold = 1.0f;
} // namespace Viewport

// Colors
namespace Color {
// Editor UI Colors (Originally from EngineConfig.h)
constexpr Vec4f kWinSelectColor       = Vec4f(0.26f, 0.59f, 0.98f, 1.0f);
constexpr float kWinSelectHeaderAlpha = 0.8f;

constexpr Vec4f kWinTitleColor = Vec4f(0.04f, 0.04f, 0.04f, 1.0f);
constexpr Vec4f kWinBgColor    = Vec4f(0.06f, 0.06f, 0.06f, 0.94f);
} // namespace Color

} // namespace OriGine::EditorConfig
