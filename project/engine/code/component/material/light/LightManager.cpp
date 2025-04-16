#include "LightManager.h"

/// engine
#include "Engine.h"
// directX12 Object
#include "directX12/DxSrvArrayManager.h"

/// lib
#include "globalVariables/GlobalVariables.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

LightManager::LightManager()
    : IModule() {}
LightManager::~LightManager() {}

void LightManager::Initialize() {
    ID3D12Device* device = Engine::getInstance()->getDxDevice()->getDevice();

    ///========================================
    /// 作成個数を決める
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
    directionalLights_.CreateBuffer(device, srvArray_.get(), directionalLightSize_);

    /// Point Light
    pointLights_.CreateBuffer(device, srvArray_.get(), pointLightSize_);

    /// Spot Light
    spotLights_.CreateBuffer(device, srvArray_.get(), spotLightSize_);
}

void LightManager::Update() {
    lightCounts_->directionalLightNum = static_cast<int32_t>(directionalLights_.openData_.size());
    directionalLights_.ConvertToBuffer();

    lightCounts_->pointLightNum = static_cast<int32_t>(pointLights_.openData_.size());
    pointLights_.ConvertToBuffer();

    lightCounts_->spotLightNum = static_cast<int32_t>(spotLights_.openData_.size());
    spotLights_.ConvertToBuffer();

    lightCounts_.ConvertToBuffer();
}

void LightManager::Finalize() {
    lightCounts_.Finalize();
    directionalLights_.Finalize();
    pointLights_.Finalize();
    spotLights_.Finalize();

    srvArray_->Finalize();
}

void LightManager::SetForRootParameter(ID3D12GraphicsCommandList* cmdList) {
    lightCounts_.SetForRootParameter(cmdList, 6);

    ID3D12DescriptorHeap* ppHeaps[] = {DxHeap::getInstance()->getSrvHeap()};
    cmdList->SetDescriptorHeaps(1, ppHeaps);

    directionalLights_.SetForRootParameter(cmdList, 3);
    pointLights_.SetForRootParameter(cmdList, 4);
    spotLights_.SetForRootParameter(cmdList, 5);
}

void LightManager::pushDirectionalLight(const DirectionalLight& light) {
    if (directionalLights_.openData_.size() >= directionalLights_.capacity()) {
        return;
    }
    directionalLights_.openData_.push_back(light);
}

void LightManager::pushPointLight(const PointLight& light) {
    if (pointLights_.openData_.size() >= pointLights_.capacity()) {
        return;
    }
    pointLights_.openData_.push_back(light);
}

void LightManager::pushSpotLight(const SpotLight& light) {
    if (spotLights_.openData_.size() >= spotLights_.capacity()) {
        return;
    }
    spotLights_.openData_.push_back(light);
}
