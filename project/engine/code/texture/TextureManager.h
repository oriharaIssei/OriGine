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

// dx12Object
#include "directX12/DxCommand.h"
#include "directX12/DxDescriptor.h"
#include "directX12/DxFence.h"
#include "directX12/DxResource.h"
#include "directX12/PipelineStateObj.h"

struct Texture {
    void Initialize(const std::string& filePath);
    void Finalize();

    std::string path;
    DirectX::TexMetadata metaData;
    DxResource resource;
    DxSrvDescriptor srv;

private:
    DirectX::ScratchImage Load(const std::string& filePath);
    void UploadTextureData(DirectX::ScratchImage& mipImg, Microsoft::WRL::ComPtr<ID3D12Resource> reosurce);
    void ExecuteCommand(Microsoft::WRL::ComPtr<ID3D12Resource> resource);
};

class TextureManager {
    friend struct Texture;

public:
    TextureManager();
    ~TextureManager();

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

    // バックグラウンドスレッド用
    static std::unique_ptr<DxCommand> dxCommand_;

public:
    static D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorGpuHandle(uint32_t handleId);

    static const DirectX::TexMetadata& GetTexMetadata(uint32_t handleId) { return textures_[handleId]->metaData; }
};

// engine/resource/ は含まれない
const static std::string kDefaultWhiteTextureLocalPath = "/Texture/white1x1.png";
