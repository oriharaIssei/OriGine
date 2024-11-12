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
	/// light 作成
	///========================================
	/// Directional Light
	directionalLights_.CreateBuffer(device,lightCounts_.openData_.directionalLightNum);
	int32_t index = 0;
	for(auto& directionalLight : directionalLights_.openData_){
		directionalLight.Init(index);
		index++;
	}

	/// Point Light
	pointLights_.CreateBuffer(device,lightCounts_.openData_.pointLightNum);
	index = 0;
	for(auto& pointLight : pointLights_.openData_){
		pointLight.Init(index);
		index++;
	}

	/// Spot Light
	spotLights_.CreateBuffer(device,lightCounts_.openData_.spotLightNum);
	index = 0;
	for(auto& spotLight : spotLights_.openData_){
		spotLight.Init(index);
		index++;
	}
	///========================================
	/// srvArray 作成
	///========================================
	srvArray_ = DxSrvArrayManager::getInstance()->Create(3);

	D3D12_SHADER_RESOURCE_VIEW_DESC lightViewDesc{};
	lightViewDesc.Format = DXGI_FORMAT_UNKNOWN;
	lightViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	lightViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	lightViewDesc.Buffer.FirstElement = 0;
	lightViewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	lightViewDesc.Buffer.NumElements = lightCounts_.openData_.directionalLightNum;
	lightViewDesc.Buffer.StructureByteStride = sizeof(DirectionalLight::ConstantBuffer);

	srvArray_->CreateView(device,lightViewDesc,directionalLights_.getResource().getResource());

	lightViewDesc.Buffer.NumElements = lightCounts_.openData_.pointLightNum;
	lightViewDesc.Buffer.StructureByteStride = sizeof(PointLight::ConstantBuffer);
	srvArray_->CreateView(device,lightViewDesc,pointLights_.getResource().getResource());

	lightViewDesc.Buffer.NumElements = lightCounts_.openData_.spotLightNum;
	lightViewDesc.Buffer.StructureByteStride = sizeof(SpotLight::ConstantBuffer);
	srvArray_->CreateView(device,lightViewDesc,spotLights_.getResource().getResource());
}

void LightManager::Update(){
#ifdef _DEBUG
	for(auto& directionalLight : directionalLights_.openData_){
		directionalLight.DebugUpdate();
	}
	for(auto& spotLight : spotLights_.openData_){
		spotLight.DebugUpdate();
	}
	for(auto& pointLight : pointLights_.openData_){
		pointLight.DebugUpdate();
	}
#endif // _DEBUG

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
	lightCounts_.SetForRootParameter(cmdList,7);

	ID3D12DescriptorHeap* ppHeaps[] = {DxHeap::getInstance()->getSrvHeap()};
	cmdList->SetDescriptorHeaps(1,ppHeaps);

	cmdList->SetGraphicsRootDescriptorTable(3,DxHeap::getInstance()->getSrvGpuHandle(srvArray_->getLocationOnHeap(0)));
	cmdList->SetGraphicsRootDescriptorTable(4,DxHeap::getInstance()->getSrvGpuHandle(srvArray_->getLocationOnHeap(1)));
	cmdList->SetGraphicsRootDescriptorTable(5,DxHeap::getInstance()->getSrvGpuHandle(srvArray_->getLocationOnHeap(2)));
}