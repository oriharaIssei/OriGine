#include "TextureLoader.h"

/// directXTex
#include <DirectXTex/d3dx12.h>

/// stl
#include <cassert>

/// engine
#include "Engine.h"
// directX12
#include "directX12/DxDescriptor.h"
#include "directX12/DxDevice.h"
#include "directX12/DxFence.h"

/// util
#include "util/StringUtil.h"

using namespace OriGine;

void TextureUploadHelper::UploadToGpu(
    DxCommand* _dxCommand,
    const DirectX::ScratchImage& _image,
    TextureAsset& _outAsset) {
    auto dxDevice = Engine::GetInstance()->GetDxDevice();

    // Resource 作成
    _outAsset.resource.CreateTextureResource(
        dxDevice->device_,
        _image.GetMetadata());

    std::vector<D3D12_SUBRESOURCE_DATA> subResources;
    DirectX::PrepareUpload(
        dxDevice->device_.Get(),
        _image.GetImages(),
        _image.GetImageCount(),
        _image.GetMetadata(),
        subResources);

    uint64_t uploadSize = GetRequiredIntermediateSize(
        _outAsset.resource.GetResource().Get(),
        0,
        UINT(subResources.size()));

    DxResource uploadBuffer;
    uploadBuffer.CreateBufferResource(dxDevice->device_, uploadSize);

    UpdateSubresources(
        _dxCommand->GetCommandList().Get(),
        _outAsset.resource.GetResource().Get(),
        uploadBuffer.GetResource().Get(),
        0,
        0,
        UINT(subResources.size()),
        subResources.data());

    // Barrier & Execute
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource   = _outAsset.resource.GetResource().Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_GENERIC_READ;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    _dxCommand->GetResourceStateTracker()
        ->DirectBarrier(_dxCommand->GetCommandList(),
            _outAsset.resource.GetResource().Get(),
            barrier);

    _dxCommand->Close();
    _dxCommand->ExecuteCommand();

    DxFence* fence = Engine::GetInstance()->GetDxFence();
    UINT64 val     = fence->Signal(_dxCommand->GetCommandQueue());
    fence->WaitForFence(val);

    _dxCommand->CommandReset();

    _outAsset.metaData = _image.GetMetadata();

    // SRV の作成
    /// metadataを基に SRV の設定
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format                  = _outAsset.metaData.format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    if (_outAsset.metaData.IsCubemap()) {
        srvDesc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.TextureCube.MostDetailedMip     = 0;
        srvDesc.TextureCube.MipLevels           = UINT_MAX;
        srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
    } else {
        srvDesc.ViewDimension       = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = UINT(_outAsset.metaData.mipLevels);
    }
    SRVEntry srvEntry(&_outAsset.resource, srvDesc);
    _outAsset.srv = Engine::GetInstance()->GetSrvHeap()->CreateDescriptor(&srvEntry);
}

void OriGine::TextureWicLoader::Initialize() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");
}

void OriGine::TextureWicLoader::Finalize() {
    dxCommand_->Finalize();
    dxCommand_.reset();
}

TextureAsset TextureWicLoader::LoadAsset(const std::string& _assetPath) {
    TextureAsset asset{};

    DirectX::ScratchImage _image{};
    DirectX::ScratchImage mipImages{};

    std::wstring pathW = ConvertString(_assetPath);

    HRESULT hr = DirectX::LoadFromWICFile(
        pathW.c_str(),
        DirectX::WIC_FLAGS_FORCE_SRGB | DirectX::WIC_FLAGS_DEFAULT_SRGB,
        &asset.metaData,
        _image);
    assert(SUCCEEDED(hr));

    // mipmap 生成
    if (_image.GetMetadata().width > 1 && _image.GetMetadata().height > 1) {
        hr = DirectX::GenerateMipMaps(
            _image.GetImages(),
            _image.GetImageCount(),
            _image.GetMetadata(),
            DirectX::TEX_FILTER_SRGB,
            0,
            mipImages);
        assert(SUCCEEDED(hr));
    } else {
        mipImages = std::move(_image);
    }

    asset.metaData = mipImages.GetMetadata();

    TextureUploadHelper::UploadToGpu(dxCommand_.get(), mipImages, asset);
    return asset;
}

void OriGine::TextureDdsLoader::Initialize() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");
}

void OriGine::TextureDdsLoader::Finalize() {
    dxCommand_->Finalize();
    dxCommand_.reset();
}

TextureAsset TextureDdsLoader::LoadAsset(const std::string& _assetPath) {
    TextureAsset asset{};

    DirectX::ScratchImage _image{};
    std::wstring pathW = ConvertString(_assetPath);

    HRESULT hr = DirectX::LoadFromDDSFile(
        pathW.c_str(),
        DirectX::DDS_FLAGS_NONE,
        &asset.metaData,
        _image);
    assert(SUCCEEDED(hr));

    // DDS は mipmap 済み前提
    TextureUploadHelper::UploadToGpu(dxCommand_.get(), _image, asset);
    return asset;
}
