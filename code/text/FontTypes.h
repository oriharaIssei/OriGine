#pragma once

#include <cstdint>
#include <Vector2.h>
#include <Vector4.h>

namespace OriGine {

/// <summary>
/// アトラス上の1グリフ分のメトリクス情報（UV範囲・サイズ・ベアリング・送り幅）。
/// </summary>
struct GlyphMetrics {
    Vec2f uvMin;    // アトラス内UV座標の最小
    Vec2f uvMax;    // アトラス内UV座標の最大
    Vec2f size;     // グリフの描画サイズ（px）
    Vec2f bearing;  // 基準位置からのオフセット
    float advance = 0.0f; // 次の文字へ進める幅
};

/// <summary>
/// 描画用に展開した1文字分の矩形情報（スクリーン座標＋UV＋色）。
/// </summary>
struct GlyphQuad {
    Vec2f posMin;
    Vec2f posMax;
    Vec2f uvMin;
    Vec2f uvMax;
    Vec4f color;
};

/// <summary>
/// テキスト描画に使用する頂点データ。
/// </summary>
struct TextVertex {
    Vec4f pos;
    Vec2f uv;
    Vec4f color;
};

using FontHandle = uint32_t;
/// <summary>未登録・無効なフォントを表す値</summary>
constexpr FontHandle kInvalidFontHandle = 0xFFFFFFFF;

} // namespace OriGine
