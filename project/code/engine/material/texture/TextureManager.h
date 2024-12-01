#pragma once

#include "d3d12.h"
#include "DirectXTex/d3dx12.h"
#include "DirectXTex/DirectXTex.h"

#include <thread>

#include <stdint.h>

#include <string>

#include <array>
#include <queue>

#include <memory>
#include <wrl.h>

#include "assets/IAsset.h"
#include "directX12/DxCommand.h"
#include "directX12/DxCommand.h"
#include "directX12/DxResource.h"
#include "directX12/DxSrvArray.h"
#include "directX12/PipelineStateObj.h"
#include "module/IModule.h"

#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>

struct Texture
	: IAsset{
	enum class LoadState{
		Loading,
		Loaded,
		Error
	};
	void Init(const std::string& filePath,std::shared_ptr<DxSrvArray> srvArray,int textureIndex);
	void Finalize();

	std::string path;
	DirectX::TexMetadata metaData;
	uint32_t resourceIndex;

	LoadState loadState;
private:
	DirectX::ScratchImage Load(const std::string& filePath);
	void UploadTextureData(DirectX::ScratchImage& mipImg,ID3D12Resource* reosurce);
	void ExecuteCommand(ID3D12Resource* resource);
};

class TextureManager
:public IModule{
	friend struct Texture;
public:
	static void Init();
	static void Finalize();

	static uint32_t LoadTexture(const std::string &filePath);
	static void UnloadTexture(uint32_t id);

	static void LoadLoop();
public:
	static const uint32_t maxTextureSize_ = 128;	
private:
	static std::shared_ptr<DxSrvArray> dxSrvArray_;
	static std::array<std::unique_ptr<Texture>,maxTextureSize_> textures_;

	static std::thread loadingThread_;
	static std::queue<std::tuple<Texture*,std::string,uint32_t>> loadingQueue_;
	static std::mutex queueMutex_;
	static std::condition_variable queueCondition_;
	static bool stopLoadingThread_;

	// バックグラウンドスレッド用
	static std::unique_ptr<DxCommand> dxCommand_;
public:
	static D3D12_GPU_DESCRIPTOR_HANDLE getDescriptorGpuHandle(uint32_t handleId){
		DxHeap* heap = DxHeap::getInstance();
		int32_t locate = 0;
		if(textures_[handleId]->loadState == Texture::LoadState::Loaded){
			locate = textures_[handleId]->resourceIndex;
		}
		return  heap->getSrvGpuHandle(dxSrvArray_->getLocationOnHeap(textures_[locate]->resourceIndex));
	}

	static const DirectX::TexMetadata& getTexMetadata(uint32_t handleId){return textures_[handleId]->metaData; }
};