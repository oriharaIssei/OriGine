#pragma once

/// std
#include <array>
#include <string>

/// directX12
#include <d3d12.h>

namespace OriGine {

/// <summary>
/// ブレンドモード
/// </summary>
enum class BlendMode {
    None     = 0,
    Normal   = 1,
    Alpha    = Normal,
    Add      = 2,
    Subtract = 3,
    Sub      = Subtract,
    Multiply = 4,
    Screen   = 5,

    Count
};

// BlendModeの数
constexpr size_t kBlendNum                                       = static_cast<size_t>(BlendMode::Count);
static const ::std::array<::std::string, kBlendNum> blendModeStr = {
    "None",
    "Normal",
    "Add",
    "Subtract",
    "Multiply",
    "Screen"};

D3D12_BLEND_DESC CreateBlendDescByBlendMode(BlendMode mode);

} // namespace OriGine

namespace std {
std::string to_string(const OriGine::BlendMode& mode);
} // namespace std
