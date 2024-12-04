#include "LightManager.h"

#include "directX12/DxSrvArrayManager.h"
#include "globalVariables/GlobalVariables.h"
#include "Engine.h"

void LightManager::Init(){
	GlobalVariables* variables = GlobalVariables::getInstance();
	ID3D12Device* device = Engine::getInstance()->getDxDevice()->getDevice();

	///========================================
	///作成個数を決める
	///========================================

	lightCounts_.CreateBuffer(device);

	lightCounts_.ConvertToBuffer();

	///========================================
	/// srvArray 作成
	///========================================
	srvArray_ = DxSrvArrayManager::getInstance()->Create(3);

	///========================================
	/// light 作成
	///========================================
	/// Directional Light
	directionalLights_.CreateBuffer(device,srvArray_.get(),lightCounts_.openData_.directionalLightNum);
	for(int32_t i = 0; i < lightCounts_.openData_.directionalLightNum; i++){
		directionalLights_.openData_.push_back({"LightManager",i});
	}

	/// Point Light
	pointLights_.CreateBuffer(device,srvArray_.get(),lightCounts_.openData_.pointLightNum);
	for(int32_t i = 0; i < lightCounts_.openData_.pointLightNum; i++){
		pointLights_.openData_.push_back({"LightManager",i});
	}

	/// Spot Light
	spotLights_.CreateBuffer(device,srvArray_.get(),lightCounts_.openData_.spotLightNum);
	for(int32_t i = 0; i < lightCounts_.openData_.spotLightNum; i++){
		spotLights_.openData_.push_back({"LightManager",i});
	}
}

void LightManager::Update(){
	directionalLights_.ConvertToBuffer();
	pointLights_.ConvertToBuffer();
	spotLights_.ConvertToBuffer();
}

void LightManager::Finalize(){
	lightCounts_.Finalize();
	directionalLights_.Finalize();
	pointLights_.Finalize();
	spotLights_.Finalize();

	srvArray_->Finalize();
}

void LightManager::SetForRootParameter(ID3D12GraphicsCommandList* cmdList){
	lightCounts_.SetForRootParameter(cmdList,6);

	ID3D12DescriptorHeap* ppHeaps[] = {DxHeap::getInstance()->getSrvHeap()};
	cmdList->SetDescriptorHeaps(1,ppHeaps);

	directionalLights_.SetForRootParameter(cmdList,3);
	pointLights_.SetForRootParameter(cmdList,4);
	spotLights_.SetForRootParameter(cmdList,5);
}