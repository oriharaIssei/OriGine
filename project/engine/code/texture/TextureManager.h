#pragma once

/// stl
// memory
#include <memory>
// container
#include <array>
#include <queue>
// basic
#include <stdint.h>
#include <string>

/// microsoft
#include <wrl.h>

/// engine
#include "assets/IAsset.h"
#include "module/IModule.h"
// dx12Object
#include "directX12/DxCommand.h"
#include "directX12/DxResource.h"
#include "directX12/DxSrvArray.h"
#include "directX12/PipelineStateObj.h"
// lib
#include "Thread/Thread.h"

struct Texture
    : IAsset {
    void Initialize(const std::string& filePath, std::shared_ptr<DxSrvArray> srvArray);
    void Finalize();

    std::string path;
    DirectX::TexMetadata metaData;
    uint32_t resourceIndex;

    LoadState loadState = LoadState::Unloaded;

private:
    DirectX::ScratchImage Load(const std::string& filePath);
    void UploadTextureData(DirectX::ScratchImage& mipImg, ID3D12Resource* reosurce);
    void ExecuteCommand(ID3D12Resource* resource);
};

class TextureManager
    : public IModule {
    friend struct Texture;

public:
    static void Initialize();
    static void Finalize();

    static uint32_t LoadTexture(const std::string& filePath, std::function<void(uint32_t)> callBack = nullptr);
    static void UnloadTexture(uint32_t id);

public:
    static const uint32_t maxTextureSize_ = 128;

    struct LoadTask {
        std::string filePath;
        uint32_t textureIndex = 0;
        Texture* texture      = nullptr;

        std::function<void(uint32_t)> callBack;
        void Update();
    };

private:
    static std::shared_ptr<DxSrvArray> dxSrvArray_;
    static std::array<std::unique_ptr<Texture>, maxTextureSize_> textures_;

    static std::unique_ptr<TaskThread<TextureManager::LoadTask>> loadThread_;

    // バックグラウンドスレッド用
    static std::unique_ptr<DxCommand> dxCommand_;

public:
    static D3D12_GPU_DESCRIPTOR_HANDLE getDescriptorGpuHandle(uint32_t handleId) {
        DxHeap* heap   = DxHeap::getInstance();
        int32_t locate = 0;
        if (textures_[handleId] && textures_[handleId]->loadState == LoadState::Loaded) {
            locate = textures_[handleId]->resourceIndex;
        }
        return heap->getSrvGpuHandle(dxSrvArray_->getLocationOnHeap(textures_[locate]->resourceIndex));
    }

    static const DirectX::TexMetadata& getTexMetadata(uint32_t handleId) { return textures_[handleId]->metaData; }
};
