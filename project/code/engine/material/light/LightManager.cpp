#include "LightManager.h"

#include "Engine.h"
#include "directX12/DxSrvArrayManager.h"
#include "globalVariables/GlobalVariables.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

LightManager::LightManager()
    : IModule() {}
LightManager::~LightManager() {}

void LightManager::Init() {
    GlobalVariables* variables = GlobalVariables::getInstance();
    ID3D12Device* device       = Engine::getInstance()->getDxDevice()->getDevice();

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
    directionalLights_.CreateBuffer(device, srvArray_.get(), lightCounts_.openData_.directionalLightNum);
    for (int32_t i = 0; i < lightCounts_.openData_.directionalLightNum; i++) {
        directionalLights_.openData_.push_back({"LightManager", i});
    }

    /// Point Light
    pointLights_.CreateBuffer(device, srvArray_.get(), lightCounts_.openData_.pointLightNum);
    for (int32_t i = 0; i < lightCounts_.openData_.pointLightNum; i++) {
        pointLights_.openData_.push_back({"LightManager", i});
    }

    /// Spot Light
    spotLights_.CreateBuffer(device, srvArray_.get(), lightCounts_.openData_.spotLightNum);
    for (int32_t i = 0; i < lightCounts_.openData_.spotLightNum; i++) {
        spotLights_.openData_.push_back({"LightManager", i});
    }
}

void LightManager::Update() {
    directionalLights_.ConvertToBuffer();
    pointLights_.ConvertToBuffer();
    spotLights_.ConvertToBuffer();
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

LightEditor::LightEditor()
    : IEditor() {}
LightEditor::~LightEditor() {}

void LightEditor::Init() {
    lightManager_ = LightManager::getInstance();
}
void LightEditor::Update() {
#ifdef _DEBUG
    int32_t lightIndex = 0;
    std::string label  = "NULL";

    for (auto& directionalLight : lightManager_->directionalLights_.openData_) {
        label = "DirectionalLight_" + std::to_string(lightIndex);
        ImGui::Begin(label.c_str());
        ImGui::ColorEdit3("color", reinterpret_cast<float*>(directionalLight.color.operator Vector3*()));
        ImGui::SliderFloat("intensity", directionalLight.intensity, 0.0f, 1.0f);
        ImGui::SliderFloat3("direction", reinterpret_cast<float*>(directionalLight.direction.operator Vector3*()), -1.0f, 1.0f);
        directionalLight.direction.setValue(directionalLight.direction->normalize());
        ImGui::End();
        ++lightIndex;
    }

    lightIndex = 0;
    label      = "NULL";
    for (auto& pointLight : lightManager_->pointLights_.openData_) {
        label = "PointLight" + std::to_string(lightIndex);
        ImGui::Begin(label.c_str());
        ImGui::ColorEdit3("color", reinterpret_cast<float*>(pointLight.color.operator Vector3*()));
        ImGui::SliderFloat("intensity", pointLight.intensity, 0.0f, 1.0f);
        ImGui::DragFloat("decay", pointLight.decay, 0.1f, 0.0f);
        ImGui::DragFloat3("pos", reinterpret_cast<float*>(pointLight.pos.operator Vector3*()), 0.1f);
        ImGui::DragFloat("radius", pointLight.radius, 0.1f, 0.0f);
        ImGui::End();
        ++lightIndex;
    }

    for (auto& spotLight : lightManager_->spotLights_.openData_) {
        label = "SpotLight" + std::to_string(lightIndex);
        ImGui::Begin(label.c_str());
        ImGui::ColorEdit3("color", reinterpret_cast<float*>(spotLight.color.operator Vector3*()));
        ImGui::SliderFloat("intensity", spotLight.intensity, 0.0f, 1.0f);
        ImGui::DragFloat("decay", spotLight.decay, 0.1f, 0.0f);
        ImGui::DragFloat("cosFalloffStart", spotLight.cosFalloffStart, 0.1f, 0.0f);
        ImGui::DragFloat3("pos", reinterpret_cast<float*>(spotLight.pos.operator Vector3*()), 0.1f);
        ImGui::SliderFloat3("direction", reinterpret_cast<float*>(spotLight.direction.operator Vector3*()), -1.0f, 1.0f);
        spotLight.direction.setValue(spotLight.direction->normalize());
        ImGui::DragFloat("distance", spotLight.distance, 0.1f, 0.0f);
        ImGui::DragFloat("cosAngle", spotLight.cosAngle, 0.1f, 0.0f);
        ImGui::End();
        ++lightIndex;
    }
#endif // _DEBUG
}
