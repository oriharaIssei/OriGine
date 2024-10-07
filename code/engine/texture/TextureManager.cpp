#include "texture/TextureManager.h"

#include <cassert>
#include <combaseapi.h>

#include <iostream>

#include "System.h"

#include "directX12/dxFunctionHelper/DxFunctionHelper.h"
#include "directX12/dxHeap/DxHeap.h"
#include "directX12/dxResource/srv/DxSrvArrayManager.h"
#include "directX12/resourceBarrierManager/ResourceBarrierManager.h"
#include "directX12/ShaderCompiler.h"

#include "logger/Logger.h"

uint64_t TextureManager::cpuDescriptorHandleStart_;
uint64_t TextureManager::gpuDescriptorHandleStart_;
uint32_t TextureManager::handleIncrementSize_;

const uint32_t TextureManager::maxTextureSize_;
std::shared_ptr<DxSrvArray> TextureManager::dxSrvArray_;
std::array<std::unique_ptr<TextureManager::Texture>,TextureManager::maxTextureSize_> TextureManager::textures_;

bool TextureManager::stopLoadingThread_;
std::thread TextureManager::loadingThread_;
std::queue<std::tuple<TextureManager::Texture*,std::string,uint32_t>> TextureManager::loadingQueue_;
std::mutex TextureManager::queueMutex_;
std::condition_variable TextureManager::queueCondition_;

std::unique_ptr<DxCommand> TextureManager::dxCommand_;

#pragma region Texture
void TextureManager::Texture::Init(const std::string& filePath,std::shared_ptr<DxSrvArray> srvArray,int textureIndex)
{
	loadState = LoadState::Loading;
	path_ = filePath;
	Microsoft::WRL::ComPtr<ID3D12Resource> resource;
	//==================================================
	// Textureを読み込んで転送する
	//==================================================
	DirectX::ScratchImage mipImages = Load(filePath);
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	resource = CreateTextureResource(System::getInstance()->getDxDevice()->getDevice(),metadata);
	UploadTextureData(mipImages,resource);

	//==================================================
	// ShaderResourceView を作成
	//==================================================
	/// metadataを基に SRV の設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	/// SRV を作成する  の場所を決める
	/// 先頭は ImGui が使用しているので その次を使う
	srvHandleCPU = D3D12_CPU_DESCRIPTOR_HANDLE(cpuDescriptorHandleStart_ + (handleIncrementSize_ * textureIndex));

	srvHandleGPU = D3D12_GPU_DESCRIPTOR_HANDLE(gpuDescriptorHandleStart_ + (handleIncrementSize_ * textureIndex));

	/// SRV の作成
	auto device = System::getInstance()->getDxDevice()->getDevice();
	resourceIndex = srvArray->CreateView(device,srvDesc,resource);
	loadState = LoadState::Loaded;
}

void TextureManager::Texture::Finalize()
{
	dxSrvArray_->DestroyView(resourceIndex);
}

DirectX::ScratchImage TextureManager::Texture::Load(const std::string& filePath)
{
	DirectX::ScratchImage image{};

// テクスチャファイルを読み込む
	std::wstring filePathW = Logger::ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(
		filePathW.c_str(),
		DirectX::WIC_FLAGS_FORCE_SRGB,
		nullptr,
		image
	);
	if(FAILED(hr))
	{
		std::cerr << "Failed to load texture file: " << filePath << std::endl;
		assert(SUCCEEDED(hr));
	}

	DirectX::ScratchImage mipImages{};

	// ミップマップの作成は画像が1x1以上の場合のみ行う
	if(image.GetMetadata().width > 1 && image.GetMetadata().height > 1)
	{
		hr = DirectX::GenerateMipMaps(
			image.GetImages(),
			image.GetImageCount(),
			image.GetMetadata(),
			DirectX::TEX_FILTER_SRGB,
			0,
			mipImages
		);
		if(FAILED(hr))
		{
			std::cerr << "Failed to generate mipmaps for: " << filePath << std::endl;
			assert(SUCCEEDED(hr));
		}
	} else
	{
		mipImages = std::move(image);  // 1x1の場合、MipMap生成をスキップ
	}

	return mipImages;
}

void TextureManager::Texture::UploadTextureData(DirectX::ScratchImage& mipImg,Microsoft::WRL::ComPtr<ID3D12Resource>& resource)
{
	std::vector<D3D12_SUBRESOURCE_DATA> subResources;
	auto dxDevice = System::getInstance()->getDxDevice();
	DirectX::PrepareUpload(
		dxDevice->getDevice(),
		mipImg.GetImages(),
		mipImg.GetImageCount(),
		mipImg.GetMetadata(),
		subResources
	);

	uint64_t intermediateSize = GetRequiredIntermediateSize(
		resource.Get(),
		0,
		UINT(subResources.size())
	);

	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = nullptr;
	DxFH::CreateBufferResource(dxDevice,intermediateResource,intermediateSize);

	UpdateSubresources(
		dxCommand_->getCommandList(),
		resource.Get(),
		intermediateResource.Get(),
		0,
		0,
		UINT(subResources.size()),
		subResources.data()
	);
	ExecuteCommand(resource);
}
void TextureManager::Texture::ExecuteCommand(Microsoft::WRL::ComPtr<ID3D12Resource>& resource)
{
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = resource.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	dxCommand_->ResourceBarrier(1,&barrier);

	dxCommand_->Close();

	dxCommand_->ExecuteCommand();

	// フェンスを使ってGPUが完了するのを待つ
	DxFence fence;
	fence.Init(System::getInstance()->getDxDevice()->getDevice());

	fence.Signal(dxCommand_->getCommandQueue());
	fence.WaitForFence();
	dxCommand_->CommandReset();
}

Microsoft::WRL::ComPtr<ID3D12Resource> TextureManager::Texture::CreateTextureResource(ID3D12Device* device,const DirectX::TexMetadata& metadata)
{
	Microsoft::WRL::ComPtr<ID3D12Resource> resource;
	//================================================
	// 1. metadata を基に Resource を設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metadata.width);
	resourceDesc.Height = UINT(metadata.height);
	resourceDesc.MipLevels = UINT16(metadata.mipLevels);// mipMap の数
	resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize); // 奥行 or Texture[]の配列数
	resourceDesc.Format = metadata.format; //texture の Format
	resourceDesc.SampleDesc.Count = 1; // サンプリングカウント 1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);

	//================================================
	// 2. 利用する Heap の設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	//================================================
	// 3. Resource の作成
	HRESULT hr;
	hr = device->CreateCommittedResource(
		&heapProperties,// heap の設定
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,// Clear最適値
		IID_PPV_ARGS(&resource)
	);
	assert(SUCCEEDED(hr));

	return resource;
}

#pragma endregion

#pragma region "Manager"
void TextureManager::Init()
{
	CoInitializeEx(0,COINIT_MULTITHREADED);

	DxHeap* heap = DxHeap::getInstance();
	auto* device = System::getInstance()->getDxDevice()->getDevice();

	cpuDescriptorHandleStart_ = heap->getSrvHeap()->GetCPUDescriptorHandleForHeapStart().ptr;
	gpuDescriptorHandleStart_ = heap->getSrvHeap()->GetGPUDescriptorHandleForHeapStart().ptr;
	handleIncrementSize_ = device->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
	);

	dxSrvArray_ = DxSrvArrayManager::getInstance()->Create(maxTextureSize_);

	stopLoadingThread_ = false;
	loadingThread_ = std::thread(&TextureManager::LoadLoop);

	// コマンドキュー、コマンドアロケーター、コマンドリストの初期化
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.NodeMask = 0;

	dxCommand_ = std::make_unique<DxCommand>();
	dxCommand_->Init(device,"TextureManager","TextureManager");
	// load中のテクスチャにはこれをはっつける
	LoadTexture("resource/white1x1.png");
}

void TextureManager::Finalize()
{
	{
		std::unique_lock<std::mutex> lock(queueMutex_);
		stopLoadingThread_ = true;
	}
	queueCondition_.notify_all();
	if(loadingThread_.joinable())
	{
		loadingThread_.join();
	}

	CoUninitialize();

	dxCommand_->Finalize();

	for(auto& texture : textures_)
	{
		if(texture != nullptr)
		{
			texture->Finalize();
		}
	}
}

uint32_t TextureManager::LoadTexture(const std::string& filePath)
{
	uint32_t index = 0;
	for(index = 0; index < textures_.size(); ++index)
	{
		if(textures_[index] == nullptr)
		{
			textures_[index] = std::make_unique<Texture>();
			break;
		} else if(filePath == textures_[index]->path_)
		{
			return index;
		}
	}

	{
		std::unique_lock<std::mutex> lock(queueMutex_);// {}を抜けるまでロック
		loadingQueue_.emplace(textures_[index].get(),filePath,index);// loadingQueue_に追加
	}
	queueCondition_.notify_one();//threadに通知

	return index;
}


void TextureManager::LoadLoop()
{
	while(true)
	{
		std::tuple<Texture*,std::string,uint32_t> task;
		{
			std::unique_lock<std::mutex> lock(queueMutex_);
			queueCondition_.wait(lock,[] { return !loadingQueue_.empty() || stopLoadingThread_; });

			if(stopLoadingThread_ && loadingQueue_.empty())
			{
				return;
			}

			task = loadingQueue_.front();
			loadingQueue_.pop();
		}
		{
			std::unique_lock<std::mutex> lock(queueMutex_);
			std::weak_ptr<DxSrvArray> dxSrvArray = dxSrvArray_;
			std::get<0>(task)->Init(std::get<1>(task),dxSrvArray.lock(),std::get<2>(task) + 1);
		}
	}
}

void TextureManager::UnloadTexture(uint32_t id)
{
	dxSrvArray_->DestroyView(textures_[id]->resourceIndex);
	textures_[id]->Finalize();
	textures_[id].reset();
}
#pragma endregion