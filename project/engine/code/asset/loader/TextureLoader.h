#pragma once

#include "asset/TextureAsset.h"
#include "IAssetLoader.h"

/// stl
#include <memory>

/// engine
// directX12
#include "directX12/DxCommand.h"
#include "directX12/DxResource.h"

namespace OriGine {

namespace TextureUploadHelper {
/// <summary>
/// GPUへテクスチャをアップロードする.
/// </summary>
/// <param name="_image"></param>
/// <param name="_outAsset"></param>
void UploadToGpu(
    DxCommand* _dxCommand,
    const DirectX::ScratchImage& _image,
    TextureAsset& _outAsset);
}

/// <summary>
/// WIC (Windows Imaging Component) 対応フォーマット
/// </summary>
class TextureWicLoader
    : public IAssetLoader<TextureAsset> {
public:
    void Initialize() override;
    void Finalize() override;

    TextureAsset LoadAsset(const std::string& _assetPath) override;

private:
    std::unique_ptr<DxCommand> dxCommand_;
};

/// <summary>
/// DDS フォーマット対応ローダー
/// </summary>
class TextureDdsLoader
    : public IAssetLoader<TextureAsset> {
public:
    void Initialize() override;
    void Finalize() override;

    TextureAsset LoadAsset(const std::string& _assetPath) override;

private:
    std::unique_ptr<DxCommand> dxCommand_;
};

} // namespace OriGine
