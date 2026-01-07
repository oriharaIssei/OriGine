#include "TextureManager.h"

/// stl
// io
#include <iostream>
// assert
#include <cassert>
// etc
#include <combaseapi.h>

/// externals
// dx12
#include "d3d12.h"
#include "DirectXTex/d3dx12.h"
#include "DirectXTex/DirectXTex.h"

/// engine
#include "Engine.h"
#define RESOURCE_DIRECTORY
#include "EngineInclude.h"
// dx12Object
#include "directX12/DxDevice.h"
#include "directX12/DxFunctionHelper.h"
#include "directX12/ResourceStateTracker.h"
#include "directX12/ShaderCompiler.h"

/// externals
#include "logger/Logger.h"

// util
#include "util/StringUtil.h"

using namespace OriGine;

const uint32_t TextureManager::maxTextureSize_;
std::array<std::shared_ptr<Texture>, TextureManager::maxTextureSize_> TextureManager::textures_;
std::unordered_map<std::string, uint32_t> TextureManager::textureFileNameToIndexMap_;
std::unique_ptr<DxCommand> TextureManager::dxCommand_;
uint32_t TextureManager::dummyTextureIndex_;

#pragma region Texture
void Texture::Initialize(const std::string& filePath) {
    path = filePath;

    //==================================================
    // Textureを読み込んで転送する
    //==================================================
    DirectX::ScratchImage mipImages = Load(filePath);
    metaData                        = mipImages.GetMetadata();
    resource.CreateTextureResource(Engine::GetInstance()->GetDxDevice()->device_, metaData);

    // ファイル名をリソース名にセット（デバッグ用）
    std::wstring wname = ConvertString(filePath);
    resource.GetResource()->SetName(wname.c_str());

    UploadTextureData(mipImages, resource.GetResource());

    //==================================================
    // ShaderResourceView を作成
    //==================================================
    /// metadataを基に SRV の設定
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format                  = metaData.format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    if (metaData.IsCubemap()) {
        srvDesc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.TextureCube.MostDetailedMip     = 0;
        srvDesc.TextureCube.MipLevels           = UINT_MAX;
        srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
    } else {
        srvDesc.ViewDimension       = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = UINT(metaData.mipLevels);
    }

    /// SRV の作成
    SRVEntry srvEntry(&resource, srvDesc);
    srv = Engine::GetInstance()->GetSrvHeap()->CreateDescriptor(&srvEntry);
}

void Texture::Finalize() {
    Engine::GetInstance()->GetSrvHeap()->ReleaseDescriptor(srv);
    resource.Finalize();
}

DirectX::ScratchImage Texture::Load(const std::string& filePath) {
    DirectX::ScratchImage image{};

    // テクスチャファイルを読み込む
    std::wstring filePathW = ConvertString(filePath);
    HRESULT hr             = 0;

    if (filePathW.ends_with(L".dds")) {
        hr = DirectX::LoadFromDDSFile(
            filePathW.c_str(),
            DirectX::DDS_FLAGS_NONE,
            nullptr,
            image);
    } else {
        hr = DirectX::LoadFromWICFile(
            filePathW.c_str(),
            DirectX::WIC_FLAGS_FORCE_SRGB | DirectX::WIC_FLAGS_DEFAULT_SRGB,
            nullptr,
            image);
    }

    if (FAILED(hr)) {
        std::string errorMassage;
        std::wcerr << L"Failed to load texture file: " << filePathW << L" hr=0x" << std::hex << hr << std::endl;
        LOG_CRITICAL("Failed to load texture file: {} \n massage {}", filePath, std::to_string(hr));
        assert(SUCCEEDED(hr));
    }

    DirectX::ScratchImage mipImages{};

    if (DirectX::IsCompressed(image.GetMetadata().format)) {
        // 圧縮テクスチャの場合、MipMapを生成しない
        mipImages = std::move(image);
        return mipImages;
    }
    if (image.GetMetadata().width > 1 && image.GetMetadata().height > 1) {
        hr = DirectX::GenerateMipMaps(
            image.GetImages(),
            image.GetImageCount(),
            image.GetMetadata(),
            DirectX::TEX_FILTER_SRGB,
            0,
            mipImages);

        if (FAILED(hr)) {
            std::cerr << "Failed to generate mipmaps for: " << filePath << std::endl;
            assert(SUCCEEDED(hr));
        }
    } else {
        mipImages = std::move(image); // 1x1の場合、MipMap生成をスキップ
    }

    return mipImages;
}

void Texture::UploadTextureData(DirectX::ScratchImage& mipImg, Microsoft::WRL::ComPtr<ID3D12Resource> _resource) {
    std::vector<D3D12_SUBRESOURCE_DATA> subResources;
    auto dxDevice = Engine::GetInstance()->GetDxDevice();

    DirectX::PrepareUpload(
        dxDevice->device_.Get(),
        mipImg.GetImages(),
        mipImg.GetImageCount(),
        mipImg.GetMetadata(),
        subResources);

    uint64_t intermediateSize = GetRequiredIntermediateSize(
        _resource.Get(),
        0,
        UINT(subResources.size()));

    std::unique_ptr<DxResource> intermediateResource = std::make_unique<DxResource>();
    intermediateResource->CreateBufferResource(dxDevice->device_, intermediateSize);

    UpdateSubresources(
        TextureManager::dxCommand_->GetCommandList().Get(),
        _resource.Get(),
        intermediateResource->GetResource().Get(),
        0,
        0,
        UINT(subResources.size()),
        subResources.data());

    ExecuteCommand(_resource);

    intermediateResource->Finalize();
}

void Texture::ExecuteCommand(Microsoft::WRL::ComPtr<ID3D12Resource> _resource) {
    D3D12_RESOURCE_BARRIER barrier{};

    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource   = _resource.Get();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_GENERIC_READ;
    TextureManager::dxCommand_->GetResourceStateTracker()->DirectBarrier(TextureManager::dxCommand_->GetCommandList(), _resource.Get(), barrier);

    HRESULT hr = TextureManager::dxCommand_->Close();
    if (FAILED(hr)) {
        LOG_ERROR("Failed to close command list. HRESULT: {}", std::to_string(hr));
        assert(false);
    }

    TextureManager::dxCommand_->ExecuteCommand();

    DxFence* fence  = Engine::GetInstance()->GetDxFence();
    UINT64 fenceVal = fence->Signal(TextureManager::dxCommand_->GetCommandQueue());
    fence->WaitForFence(fenceVal);

    TextureManager::dxCommand_->CommandReset();

    /*TextureManager::dxFence_->Signal(TextureManager::dxCommand_->GetCommandQueue());
    TextureManager::dxFence_->WaitForFence();
    TextureManager::dxCommand_->CommandReset();*/
}

#pragma endregion

#pragma region "Manager"
TextureManager::TextureManager() {}

TextureManager::~TextureManager() {}

void TextureManager::Initialize() {
    CoInitializeEx(0, COINIT_MULTITHREADED);

#ifdef _DEBUG
    // loadThread_ = std::make_unique<TaskThread<LoadTask>>();
    // loadThread_->Initialize(1);
#endif // DEBUG

    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");

    // load中のテクスチャにはこれをはっつける
    dummyTextureIndex_ = LoadTexture(kEngineResourceDirectory + kDefaultWhiteTextureLocalPath);
}

void TextureManager::Finalize() {
#ifdef _DEBUG
    // loadThread_->Finalize();
#endif // DEBUG

    CoUninitialize();

    dxCommand_->Finalize();

    for (auto& texture : textures_) {
        if (texture != nullptr) {
            texture->Finalize();
            texture.reset();
        }
    }
}

uint32_t TextureManager::LoadTexture(const std::string& filePath, std::function<void(uint32_t loadedIndex)> callBack) {
    std::string normalizedPath = NormalizeString(filePath);
    LOG_TRACE("Load Texture \n Path : {}", filePath);

    uint32_t index = 0;
    {
        auto itr = textureFileNameToIndexMap_.find(normalizedPath);
        if (itr != textureFileNameToIndexMap_.end()) {
            // 既にロード済みのテクスチャがある場合
            index = itr->second;
            if (callBack) {
                callBack(index);
            }
            return index;
        }

        // 新規ロードの場合、マップに追加
        for (auto& texture : textures_) {
            if (texture == nullptr) {
                break;
            }
            ++index;
        }
        textureFileNameToIndexMap_[normalizedPath] = index;

        // 空きスロットがなければエラー
        if (textureFileNameToIndexMap_.size() >= maxTextureSize_) {
            LOG_CRITICAL("Texture index exceeds maxTextureSize_ limit.");
            assert(false);
        }

        // 新しいテクスチャを作成
        textures_[index] = std::make_shared<Texture>();
    }

#ifdef _DEBUG
    LoadTask task;
    task.filePath     = normalizedPath;
    task.texture      = textures_[index];
    task.textureIndex = index;
    task.callBack     = callBack;
    task.Update();

    /* loadThread_->pushTask(
         {.filePath        = normalizedPath,
             .textureIndex = index,
             .texture      = textures_[index],
             .callBack     = callBack});*/

#else
    LoadTask task;
    task.filePath     = normalizedPath;
    task.texture      = textures_[index];
    task.textureIndex = index;
    task.callBack     = callBack;

    task.Update();
#endif // DEBUG

    return index;
}

void TextureManager::UnloadTexture(uint32_t id) {
    Engine::GetInstance()->GetSrvHeap()->ReleaseDescriptor(textures_[id]->srv);
    textures_[id]->Finalize();
    textures_[id].reset();
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetDescriptorGpuHandle(uint32_t handleId) {
    uint32_t locate = dummyTextureIndex_;

    locate = handleId;

    // ロード中や未ロードの場合は必ずダミー（0番）を返す
    return textures_[locate]->srv.GetGpuHandle();
}
#pragma endregion

#pragma region "LoadTask"
void TextureManager::LoadTask::Update() {
    DeltaTime timer;
    timer.Initialize();

    texture->Initialize(filePath);

    if (callBack) {
        callBack(textureIndex);
    }
    timer.Update();

    LOG_TRACE("LoadedTexture \n Path        : {} \n Lading Time : ", filePath, std::to_string(timer.GetDeltaTime()));
}
#pragma endregion
