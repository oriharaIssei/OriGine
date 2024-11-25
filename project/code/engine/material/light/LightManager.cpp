#include "LightManager.h"

#include "directX12/DxSrvArrayManager.h"
#include "globalVariables/GlobalVariables.h"
#include "System.h"

void LightManager::Init(){
	GlobalVariables* variables = GlobalVariables::getInstance();
	ID3D12Device* device = System::getInstance()->getDxDevice()->getDevice();

	///========================================
	///作成個数を決める
	///========================================

	lightCounts_.CreateBuffer(device);

	variables->addValue("LightManager","LightCounts","directionalLightNum",lightCounts_.openData_.directionalLightNum);
	variables->addValue("LightManager","LightCounts","spotLightNum",lightCounts_.openData_.spotLightNum);
	variables->addValue("LightManager","LightCounts","pointLightNum",lightCounts_.openData_.pointLightNum);

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
	int32_t index = 0;
	for(auto& directionalLight : directionalLights_.openData_){
		directionalLight.Init("LightManager",index);
		index++;
	}

	/// Point Light
	pointLights_.CreateBuffer(device,srvArray_.get(),lightCounts_.openData_.pointLightNum);
	index = 0;
	for(auto& pointLight : pointLights_.openData_){
		pointLight.Init("LightManager",index);
		index++;
	}

	/// Spot Light
	spotLights_.CreateBuffer(device,srvArray_.get(),lightCounts_.openData_.spotLightNum);
	index = 0;
	for(auto& spotLight : spotLights_.openData_){
		spotLight.Init("LightManager",index);
		index++;
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