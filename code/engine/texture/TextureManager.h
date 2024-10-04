#pragma once

#include "d3d12.h"
#include "DirectXTex/d3dx12.h"
#include "DirectXTex.h"

#include <thread>

#include <stdint.h>

#include <string>

#include <array>
#include <queue>

#include <memory>
#include <wrl.h>

#include "directX12/dxCommand/DxCommand.h"
#include "directX12/dxResource/srv/DxSrvArray.h"
#include "directX12/PipelineStateObj.h"

#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>

class TextureManager{
public:
	static void Init();
	static void Finalize();

	static uint32_t LoadTexture(const std::string &filePath);
	static void UnloadTexture(uint32_t id);

	static void LoadLoop();
public:
	static const uint32_t maxTextureSize_ = 128;
private:
	struct Texture{
		enum class LoadState{
			Loading,
			Loaded,
			Error
		};
		void Init(const std::string &filePath,std::shared_ptr<DxSrvArray> srvArray,int textureIndex);
		void Finalize();

		std::string path_;
		uint32_t resourceIndex;
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;

		LoadState loadState;
	private:
		DirectX::ScratchImage Load(const std::string &filePath);
		Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(ID3D12Device *device,const DirectX::TexMetadata &metadata);
		void UploadTextureData(DirectX::ScratchImage &mipImg,Microsoft::WRL::ComPtr<ID3D12Resource> &reosurce);
		void ExecuteCommnad(Microsoft::WRL::ComPtr<ID3D12Resource> &resource);
	};

private:
	static uint64_t cpuDescriptorHandleStart_;
	static uint64_t gpuDescriptorHandleStart_;
	static uint32_t handleIncrementSize_;

	static std::shared_ptr<DxSrvArray> dxSrvArray_;
	static std::array<std::shared_ptr<Texture>,maxTextureSize_> textures_;

	static std::thread loadingThread_;
	static std::queue<std::tuple<std::weak_ptr<Texture>,std::string,uint32_t>> loadingQueue_;
	static std::mutex queueMutex_;
	static std::condition_variable queueCondition_;
	static bool stopLoadingThread_;

	// バックグラウンドスレッド用
	static std::unique_ptr<DxCommand> dxCommand_;

public:
	static const D3D12_GPU_DESCRIPTOR_HANDLE &getDescriptorGpuHandle(uint32_t handleId){
		if(textures_[handleId]->loadState == Texture::LoadState::Loaded){
			return textures_[handleId]->srvHandleGPU;
		}
		return textures_[0]->srvHandleGPU;
	}
};