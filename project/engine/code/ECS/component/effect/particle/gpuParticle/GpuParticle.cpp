#include "GpuParticle.h"

/// engine
#include "Engine.h"
#include "texture/TextureManager.h"
// directX12 Object
#include "directX12/DxDescriptor.h"
#include "directX12/DxDevice.h"

/// lib
#include "lib/myFileSystem/MyFileSystem.h"
#ifdef _DEBUG
#include "myGui/MyGui.h"
#endif // _DEBUG

void GpuParticleEmitter::Initialize(GameEntity* /*_entity*/) {
    if (!texturePath_.empty()) {
        textureIndex_ = TextureManager::LoadTexture(texturePath_);
    }

    if (isActive_) {
        CreateBuffer();
    }
}

void GpuParticleEmitter::Edit(Scene* /*_scene*/, GameEntity* /*_entity*/, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    if (CheckBoxCommand("IsActive##" + _parentLabel, isActive_)) {
        // CheckBoxCommand は 1フレーム後に 値が変更されるため, 本来と逆の bool で if文を書く
        if (!isActive_) {
            CreateBuffer();
        }
    }

    ImGui::Spacing();

    std::string label = "Material##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {

        materialBuffer_->DebugGui(_parentLabel);

        ImGui::TreePop();
    }

    auto askLoadTexture = [this](const std::string& _parentLabel) {
        bool ask          = false;
        std::string label = "Load Texture##" + _parentLabel;
        ask               = ImGui::Button(label.c_str());
        ask               = ImGui::ImageButton(
            ImTextureID(TextureManager::getDescriptorGpuHandle(textureIndex_).ptr),
            ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), 4, ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1));

        return ask;
    };

    ImGui::Text("Texture Path: %s", texturePath_.c_str());
    if (askLoadTexture(_parentLabel)) {
        std::string directory;
        std::string fileName;
        if (myfs::selectFileDialog(kApplicationResourceDirectory, directory, fileName, {"png"})) {
            auto setPath = std::make_unique<SetterCommand<std::string>>(&texturePath_, kApplicationResourceDirectory + "/" + directory + "/" + fileName);
            CommandCombo commandCombo;
            commandCombo.addCommand(std::move(setPath));
            commandCombo.setFuncOnAfterCommand([this]() {
                textureIndex_ = TextureManager::LoadTexture(texturePath_);
            },
                true);
            EditorController::getInstance()->pushCommand(std::make_unique<CommandCombo>(commandCombo));
        }
    };

#endif // _DEBUG
}

void GpuParticleEmitter::Finalize() {

    dxResource_.Finalize();

    if (uavDescriptor_) {
        // cbv_srv_uav heap
        DxDescriptorHeap<DxDescriptorHeapType::CBV_SRV_UAV>* uavHeap = Engine::getInstance()->getSrvHeap();
        uavHeap->ReleaseDescriptor(uavDescriptor_); // UAVディスクリプタを解放
        uavDescriptor_ = nullptr; // UAVディスクリプタを解放
    }

    texturePath_.clear();
    textureIndex_ = 0;
    isActive_     = false;
}

void GpuParticleEmitter::CreateBuffer() {
    DxDevice* dxDevice = Engine::getInstance()->getDxDevice();

    // materialBuffer が未作成の場合は、バッファを生成
    if (!materialBuffer_.getResource().getResource()) {
        materialBuffer_.CreateBuffer(dxDevice->getDevice());
        materialBuffer_.ConvertToBuffer();
    }

    // particleResource が未作成の場合は、UAVバッファを生成
    if (!dxResource_.getResource()) {
        dxResource_.CreateUAVBuffer(
            dxDevice->getDevice(),
            sizeof(GpuParticleData::ConstantBuffer) * particleSize_,
            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
            D3D12_HEAP_TYPE_DEFAULT);
    }

    // Srv が 未作成の場合は、バッファを生成
    if (!srvDescriptor_) {
        // cbv_srv_uav heap
        DxDescriptorHeap<DxDescriptorHeapType::CBV_SRV_UAV>* srvHeap = Engine::getInstance()->getSrvHeap();
        // SRVディスクリプタ 生成
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format                     = DXGI_FORMAT_UNKNOWN; // SRVはフォーマットを持たない
        srvDesc.Shader4ComponentMapping    = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.ViewDimension              = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Buffer.FirstElement        = 0;
        srvDesc.Buffer.NumElements         = particleSize_;
        srvDesc.Buffer.StructureByteStride = sizeof(GpuParticleData::ConstantBuffer);
        srvDescriptor_                     = srvHeap->CreateDescriptor(srvDesc, &dxResource_);
    }

    // UAVが未作成の場合は、UAVを生成
    if (uavDescriptor_) {
        return;
    }

    // cbv_srv_uav heap
    DxDescriptorHeap<DxDescriptorHeapType::CBV_SRV_UAV>* uavHeap = Engine::getInstance()->getSrvHeap();

    // UAVディスクリプタ 生成
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
    uavDesc.Format                      = DXGI_FORMAT_UNKNOWN; // UAVはフォーマットを持たない
    uavDesc.ViewDimension               = D3D12_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.FirstElement         = 0;
    uavDesc.Buffer.NumElements          = particleSize_;
    uavDesc.Buffer.CounterOffsetInBytes = 0; // カウンターオフセットは0
    uavDesc.Buffer.Flags                = D3D12_BUFFER_UAV_FLAG_NONE; // 特にフラグは必要ない
    uavDesc.Buffer.StructureByteStride  = sizeof(GpuParticleData::ConstantBuffer);
    uavDescriptor_                      = uavHeap->CreateDescriptor(uavDesc, &dxResource_);
}

void GpuParticleEmitter::LoadTexture(const std::string& _path) {
    if (_path.empty()) {
        return;
    }
    texturePath_  = _path;
    textureIndex_ = TextureManager::LoadTexture(texturePath_);
}

void to_json(nlohmann::json& j, const GpuParticleEmitter& p) {
    j = nlohmann::json{
        {"isActive", p.isActive_},
        {"particleSize", p.particleSize_},
        {"texturePath", p.texturePath_},
        {"Material", p.materialBuffer_.openData_}};
}

void from_json(const nlohmann::json& j, GpuParticleEmitter& p) {
    j.at("isActive").get_to(p.isActive_);
    j.at("particleSize").get_to(p.particleSize_);
    j.at("texturePath").get_to(p.texturePath_);
    j.at("Material").get_to(p.materialBuffer_.openData_);
}
