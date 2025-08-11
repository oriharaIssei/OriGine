#pragma once

/// stl
// memory
#include <memory>
// container
#include <queue>
#include <unordered_map>
// basic
#include <stdint.h>
#include <string>

/// microsoft
#include <wrl.h>

/// engine
#include "module/IModule.h"
// dx12Object
#include "directX12/DxCommand.h"
#include "directX12/DxDescriptor.h"
#include "directX12/DxFence.h"
#include "directX12/DxResource.h"
#include "directX12/PipelineStateObj.h"
// lib
#include "Thread/Thread.h"

struct Texture {
    void Initialize(const std::string& filePath);
    void Finalize();

    mutable std::mutex mutex;

    std::string path;
    DirectX::TexMetadata metaData;
    DxResource resource;
    std::shared_ptr<DxSrvDescriptor> srv;

    LoadState loadState = LoadState::Unloaded;

private:
    DirectX::ScratchImage Load(const std::string& filePath);
    void UploadTextureData(DirectX::ScratchImage& mipImg, Microsoft::WRL::ComPtr<ID3D12Resource> reosurce);
    void ExecuteCommand(Microsoft::WRL::ComPtr<ID3D12Resource> resource);
};

class TextureManager
    : public IModule {
    friend struct Texture;

public:
    TextureManager();
    ~TextureManager() override;

    static void Initialize();
    static void Finalize();

    static uint32_t LoadTexture(const std::string& filePath, std::function<void(uint32_t)> callBack = nullptr);
    static void UnloadTexture(uint32_t id);

public:
    static const uint32_t maxTextureSize_ = 192;

    struct LoadTask {
        std::string filePath;
        uint32_t textureIndex            = 0;
        std::shared_ptr<Texture> texture = nullptr;

        std::function<void(uint32_t)> callBack;
        void Update();
    };

private:
    static std::array<std::shared_ptr<Texture>, maxTextureSize_> textures_;
    static std::unordered_map<std::string, uint32_t> textureFileNameToIndexMap_;
    static uint32_t dummyTextureIndex_;

    // static std::unique_ptr<TaskThread<TextureManager::LoadTask>> loadThread_;

    static std::mutex texturesMutex_;

    // バックグラウンドスレッド用
    static std::unique_ptr<DxCommand> dxCommand_;

public:
    static D3D12_GPU_DESCRIPTOR_HANDLE getDescriptorGpuHandle(uint32_t handleId);

    static const DirectX::TexMetadata& getTexMetadata(uint32_t handleId) { return textures_[handleId]->metaData; }
};

// engine/resource/ は含まれない
const static std::string kDefaultWhiteTextureLocalPath = "/Texture/white1x1.png";
