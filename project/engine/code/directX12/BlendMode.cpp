#include "BlendMode.h"

/// <summary>
/// BlendMode から D3D12_BLEND_DESC を生成するテンプレート関数
/// </summary>
/// <typeparam name="T"></typeparam>
/// <returns></returns>
template <BlendMode T>
D3D12_BLEND_DESC CreateBlendDescByBlendMode() {
    // 特殊化されていなければ 空の構造体を返す
    return D3D12_BLEND_DESC{};
};
template <>
D3D12_BLEND_DESC CreateBlendDescByBlendMode<BlendMode::None>() {
    D3D12_BLEND_DESC blendDesc{};

    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    return blendDesc;
}

template <>
D3D12_BLEND_DESC CreateBlendDescByBlendMode<BlendMode::Normal>() {
    D3D12_BLEND_DESC blendDesc{};

    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[0].BlendEnable           = true;
    blendDesc.RenderTarget[0].SrcBlend              = D3D12_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp               = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].DestBlend             = D3D12_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].SrcBlendAlpha         = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOpAlpha          = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].DestBlendAlpha        = D3D12_BLEND_INV_SRC_ALPHA;

    return blendDesc;
}
template <>
D3D12_BLEND_DESC CreateBlendDescByBlendMode<BlendMode::Add>() {
    D3D12_BLEND_DESC blendDesc{};

    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[0].BlendEnable           = true;
    blendDesc.RenderTarget[0].SrcBlend              = D3D12_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp               = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].DestBlend             = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].SrcBlendAlpha         = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOpAlpha          = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].DestBlendAlpha        = D3D12_BLEND_ZERO;

    return blendDesc;
}
template <>
D3D12_BLEND_DESC CreateBlendDescByBlendMode<BlendMode::Subtract>() {
    D3D12_BLEND_DESC blendDesc{};

    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[0].BlendEnable           = true;
    blendDesc.RenderTarget[0].SrcBlend              = D3D12_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp               = D3D12_BLEND_OP_REV_SUBTRACT;
    blendDesc.RenderTarget[0].DestBlend             = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].SrcBlendAlpha         = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOpAlpha          = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].DestBlendAlpha        = D3D12_BLEND_ZERO;

    return blendDesc;
}
template <>
D3D12_BLEND_DESC CreateBlendDescByBlendMode<BlendMode::Multiply>() {
    D3D12_BLEND_DESC blendDesc{};

    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[0].BlendEnable           = true;
    blendDesc.RenderTarget[0].SrcBlend              = D3D12_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOp               = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].DestBlend             = D3D12_BLEND_SRC_COLOR;
    blendDesc.RenderTarget[0].SrcBlendAlpha         = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOpAlpha          = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].DestBlendAlpha        = D3D12_BLEND_ZERO;

    return blendDesc;
}
template <>
D3D12_BLEND_DESC CreateBlendDescByBlendMode<BlendMode::Screen>() {
    D3D12_BLEND_DESC blendDesc{};

    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[0].BlendEnable           = true;
    blendDesc.RenderTarget[0].SrcBlend              = D3D12_BLEND_INV_DEST_COLOR;
    blendDesc.RenderTarget[0].BlendOp               = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].DestBlend             = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].SrcBlendAlpha         = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOpAlpha          = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].DestBlendAlpha        = D3D12_BLEND_ZERO;

    return blendDesc;
}

D3D12_BLEND_DESC CreateBlendDescByBlendMode(BlendMode mode) {
    switch (mode) {
    case BlendMode::None:
        return CreateBlendDescByBlendMode<BlendMode::None>();
    case BlendMode::Normal:
        return CreateBlendDescByBlendMode<BlendMode::Normal>();
    case BlendMode::Add:
        return CreateBlendDescByBlendMode<BlendMode::Add>();
    case BlendMode::Subtract:
        return CreateBlendDescByBlendMode<BlendMode::Subtract>();
    case BlendMode::Multiply:
        return CreateBlendDescByBlendMode<BlendMode::Multiply>();
    case BlendMode::Screen:
        return CreateBlendDescByBlendMode<BlendMode::Screen>();
    default:
        return D3D12_BLEND_DESC{};
    }
}

std::string std::to_string(const BlendMode& mode) {
    return blendModeStr[static_cast<size_t>(mode)];
};
