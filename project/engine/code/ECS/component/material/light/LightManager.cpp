#include "LightManager.h"

/// engine
#include "Engine.h"
// directX12
#include "directX12/DxDevice.h"

namespace OriGine {

LightManager::LightManager() {}
LightManager::~LightManager() {}

void LightManager::Initialize() {
    Microsoft::WRL::ComPtr<ID3D12Device> device = Engine::GetInstance()->GetDxDevice()->device_;

    ///========================================
    /// 作成個数を決める
    ///========================================

    lightCounts_.CreateBuffer(device);

    lightCounts_.ConvertToBuffer();

    ///========================================
    /// light 作成
    ///========================================
    /// Directional Light
    directionalLights_.CreateBuffer(device, directionalLightSize_);

    /// Point Light
    pointLights_.CreateBuffer(device, pointLightSize_);

    /// Spot Light
    spotLights_.CreateBuffer(device, spotLightSize_);
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
}

void LightManager::SetForRootParameter(
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _cmdList,
    int32_t _lightCountIndex,
    int32_t _directionalLightIndex,
    int32_t _pointLightIndex,
    int32_t _spotLightIndex) {
    lightCounts_.SetForRootParameter(_cmdList, _lightCountIndex);

    ID3D12DescriptorHeap* ppHeaps[] = {Engine::GetInstance()->GetSrvHeap()->GetHeap().Get()};
    _cmdList->SetDescriptorHeaps(1, ppHeaps);

    if (lightCounts_->directionalLightNum > 0) {
        directionalLights_.SetForRootParameter(_cmdList, _directionalLightIndex);
    }
    if (lightCounts_->pointLightNum > 0) {
        pointLights_.SetForRootParameter(_cmdList, _pointLightIndex);
    }
    if (lightCounts_->spotLightNum > 0) {
        spotLights_.SetForRootParameter(_cmdList, _spotLightIndex);
    }
}

void LightManager::PushDirectionalLight(const DirectionalLight& _light) {
    if (directionalLights_.openData_.size() >= directionalLights_.Capacity()) {
        return;
    }
    directionalLights_.openData_.push_back(_light);
}

void LightManager::PushPointLight(const PointLight& _light) {
    if (pointLights_.openData_.size() >= pointLights_.Capacity()) {
        return;
    }
    pointLights_.openData_.push_back(_light);
}

void LightManager::PushSpotLight(const SpotLight& _light) {
    if (spotLights_.openData_.size() >= spotLights_.Capacity()) {
        return;
    }
    spotLights_.openData_.push_back(_light);
}

} // namespace OriGine
