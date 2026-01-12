#pragma once

#include <array>
#include <string>

/// directX12
#include <d3d12.h>

namespace OriGine {

/// <summary>
/// エンジン内で定義される主要なブレンドモードの定義.
/// </summary>
enum class BlendMode {
    /// <summary>ブレンドなし (上書き)</summary>
    None = 0,
    /// <summary>標準的なアルファブレンド (SrcAlpha * Src + (1 - SrcAlpha) * Dest)</summary>
    Normal = 1,
    /// <summary>Normal と同一</summary>
    Alpha = Normal,
    /// <summary>加算ブレンド (Src + Dest)</summary>
    Add = 2,
    /// <summary>減算ブレンド (Dest - Src)</summary>
    Subtract = 3,
    /// <summary>Subtract と同一</summary>
    Sub = Subtract,
    /// <summary>乗算ブレンド (Src * Dest)</summary>
    Multiply = 4,
    /// <summary>スクリーンブレンド (Src + Dest - Src * Dest)</summary>
    Screen = 5,

    /// <summary>ブレンドモードの総数</summary>
    Count
};

/// <summary>BlendMode の列挙子に対応する文字列配列 (UI 表示用など)</summary>
constexpr size_t kBlendNum                                    = static_cast<size_t>(BlendMode::Count);
static const std::array<std::string, kBlendNum> kBlendModeStr = {
    "None",
    "Normal",
    "Add",
    "Subtract",
    "Multiply",
    "Screen"};

/// <summary>
/// 指定された BlendMode に基づいて、D3D12_BLEND_DESC 構造体を構築して返す.
/// </summary>
/// <param name="mode">変換元のブレンドモード</param>
/// <returns>構築された DirectX 12 ブレンド設定構造体</returns>
D3D12_BLEND_DESC CreateBlendDescByBlendMode(BlendMode mode);

} // namespace OriGine

namespace std {
/// <summary>
/// BlendModeを文字列に変換
/// </summary>
/// <param name="mode">ブレンドモード</param>
/// <returns>文字列</returns>
string to_string(const OriGine::BlendMode& mode);
} // namespace std
