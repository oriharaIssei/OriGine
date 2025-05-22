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
#include "directX12/DxFunctionHelper.h"
#include "directX12/DxHeap.h"
#include "directX12/DxSrvArrayManager.h"
#include "directX12/ResourceStateTracker.h"
#include "directX12/ShaderCompiler.h"

// lib
#include "logger/Logger.h"

// util
#include "util/ConvertString.h"

const uint32_t TextureManager::maxTextureSize_;
std::shared_ptr<DxSrvArray> TextureManager::dxSrvArray_;
std::array<std::unique_ptr<Texture>, TextureManager::maxTextureSize_> TextureManager::textures_;

std::unique_ptr<TaskThread<TextureManager::LoadTask>> TextureManager::loadThread_;
std::unique_ptr<DxCommand> TextureManager::dxCommand_;

#pragma region Texture
void Texture::Initialize(const std::string& filePath, std::shared_ptr<DxSrvArray> srvArray) {
    loadState = LoadState::Loaded;
    path      = filePath;
    DxResource resource;
    //==================================================
    // Textureを読み込んで転送する
    //==================================================
    DirectX::ScratchImage mipImages = Load(filePath);
    metaData                        = mipImages.GetMetadata();
    resource.CreateTextureResource(Engine::getInstance()->getDxDevice()->getDevice(), metaData);

    // ファイル名をリソース名にセット（デバッグ用）
    std::wstring wname = ConvertString(filePath);
    resource.getResource()->SetName(wname.c_str());

    UploadTextureData(mipImages, resource.getResource());

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
    auto device   = Engine::getInstance()->getDxDevice()->getDevice();
    resourceIndex = srvArray->CreateView(device, srvDesc, resource.getResource());
    loadState     = LoadState::Loaded;
}

void Texture::Finalize() {
    TextureManager::dxSrvArray_->DestroyView(resourceIndex);
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
            DirectX::WIC_FLAGS_FORCE_SRGB,
            nullptr,
            image);
    }

    if (FAILED(hr)) {
        std::string errorMassage;
        std::wcerr << L"Failed to load texture file: " << filePathW << L" hr=0x" << std::hex << hr << std::endl;
        LOG_CRITICAL("Failed to load texture file: " + filePath + " hr=0x" + std::to_string(hr));
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

void Texture::UploadTextureData(DirectX::ScratchImage& mipImg, ID3D12Resource* resource) {
    std::vector<D3D12_SUBRESOURCE_DATA> subResources;
    auto dxDevice = Engine::getInstance()->getDxDevice();

    DirectX::PrepareUpload(
        dxDevice->getDevice(),
        mipImg.GetImages(),
        mipImg.GetImageCount(),
        mipImg.GetMetadata(),
        subResources);

    uint64_t intermediateSize = GetRequiredIntermediateSize(
        resource,
        0,
        UINT(subResources.size()));

    std::unique_ptr<DxResource> intermediateResource = std::make_unique<DxResource>();
    intermediateResource->CreateBufferResource(dxDevice->getDevice(), intermediateSize);

    UpdateSubresources(
        TextureManager::dxCommand_->getCommandList(),
        resource,
        intermediateResource->getResource(),
        0,
        0,
        UINT(subResources.size()),
        subResources.data());
    ExecuteCommand(resource);
}
void Texture::ExecuteCommand(ID3D12Resource* resource) {
    D3D12_RESOURCE_BARRIER barrier{};

    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource   = resource;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_GENERIC_READ;
    TextureManager::dxCommand_->getResourceStateTracker()->DirectBarrier(TextureManager::dxCommand_->getCommandList(), resource, barrier);

    TextureManager::dxCommand_->Close();

    TextureManager::dxCommand_->ExecuteCommand();

    // フェンスを使ってGPUが完了するのを待つ
    DxFence fence;
    fence.Initialize(Engine::getInstance()->getDxDevice()->getDevice());

    fence.Signal(TextureManager::dxCommand_->getCommandQueue());
    fence.WaitForFence();
    TextureManager::dxCommand_->CommandReset();
}

#pragma endregion

#pragma region "Manager"
void TextureManager::Initialize() {
    CoInitializeEx(0, COINIT_MULTITHREADED);
    dxSrvArray_ = DxSrvArrayManager::getInstance()->Create(maxTextureSize_);

    loadThread_ = std::make_unique<TaskThread<LoadTask>>();
    loadThread_->Initialize(1);

    // コマンドキュー、コマンドアロケーター、コマンドリストの初期化
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type                     = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Priority                 = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    queueDesc.Flags                    = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.NodeMask                 = 0;

    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("TextureManager", "TextureManager");
    // load中のテクスチャにはこれをはっつける
    LoadTexture(kEngineResourceDirectory + "/Texture/white1x1.png");
}

void TextureManager::Finalize() {
    loadThread_->Finalize();

    CoUninitialize();

    dxCommand_->Finalize();

    for (auto& texture : textures_) {
        if (texture != nullptr) {
            texture->Finalize();
        }
    }
}

uint32_t TextureManager::LoadTexture(const std::string& filePath, std::function<void(uint32_t loadedIndex)> callBack) {
    LOG_TRACE("Load Texture \n Path : " + filePath);

    uint32_t index = 0;
    for (index = 0; index < textures_.size(); ++index) {
        if (textures_[index] == nullptr) {
            textures_[index] = std::make_unique<Texture>();
            break;
        } else if (filePath == textures_[index]->path) {
            LOG_TRACE("Already loaded texture: " + filePath);
            if (callBack) {
                callBack(index);
            }
            return index;
        }
    }

#ifdef _DEBUG
    loadThread_->pushTask(
        {.filePath        = filePath,
            .textureIndex = index,
            .texture      = textures_[index].get(),
            .callBack     = callBack});
#else
    LoadTask task;
    task.filePath     = filePath;
    task.texture      = textures_[index].get();
    task.textureIndex = index;
    task.callBack     = callBack;

    task.Update();
#endif // DEBUG

    return index;
}

void TextureManager::UnloadTexture(uint32_t id) {
    dxSrvArray_->DestroyView(textures_[id]->resourceIndex);
    textures_[id]->Finalize();
    textures_[id].reset();
}
#pragma endregion

#pragma region "LoadTask"
void TextureManager::LoadTask::Update() {
    DeltaTime timer;
    timer.Initialize();

    std::lock_guard<std::mutex> lock(texture->mutex);
    std::weak_ptr<DxSrvArray> dxSrvArray = dxSrvArray_;
    texture->Initialize(filePath, dxSrvArray.lock());

    if (callBack) {
        callBack(textureIndex);
    }
    timer.Update();

    LOG_TRACE("LoadedTexture \n Path        : " + filePath + "\n Lading Time : " + std::to_string(timer.getDeltaTime()));
}
#pragma endregion
