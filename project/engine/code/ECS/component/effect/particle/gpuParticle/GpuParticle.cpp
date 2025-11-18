#include "GpuParticle.h"

/// engine
#include "Engine.h"
#include "texture/TextureManager.h"
// directX12 Object
#include "directX12/DxDescriptor.h"
#include "directX12/DxDevice.h"

// component
#include "component/renderer/primitive/shape/Plane.h"

#include "util/myFileSystem/MyFileSystem.h"
#ifdef _DEBUG
#include "myGui/MyGui.h"
#endif // _DEBUG

void GpuParticleEmitter::Initialize(Entity* /*_entity*/) {
    Primitive::Plane plane;
    plane.CreateMesh(&mesh_);

    if (!texturePath_.empty()) {
        textureIndex_ = TextureManager::LoadTexture(texturePath_);
    }

    if (isActive_) {
        CreateBuffer();
    }
}

void GpuParticleEmitter::Edit(Scene* /*_scene*/, Entity* /*_entity*/, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    if (CheckBoxCommand("IsActive##" + _parentLabel, isActive_)) {
        // CheckBoxCommand は 1フレーム後に 値が変更されるため, 本来と逆の bool で if文を書く
        if (!isActive_) {
            CreateBuffer();
        }
    }

    DragGuiCommand<uint32_t>(
        "ParticleSize##" + _parentLabel,
        shapeBuffer_->particleSize,
        1.f,
        1, 0,
        "%6d");

    ImGui::Spacing();

    std::string label = "Material##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {

        ImGui::Text("Color");
        ColorEditGuiCommand("##color" + _parentLabel, materialBuffer_->color_);
        ImGui::Text("uvScale");
        DragGuiVectorCommand<2, float>("##uvScale" + _parentLabel, materialBuffer_->uvTransform_.scale_, 0.01f);
        ImGui::Text("uvRotate");
        DragGuiCommand<float>("##uvRotate" + _parentLabel, materialBuffer_->uvTransform_.rotate_, 0.01f);
        ImGui::Text("uvTranslate");
        DragGuiVectorCommand<2, float>("##uvTranslate" + _parentLabel, materialBuffer_->uvTransform_.translate_, 0.01f);

        ImGui::Spacing();

        auto askLoadTexture = [this](const std::string& _parentLabel) {
            bool ask          = false;
            std::string label = "Load Texture##" + _parentLabel;
            ask               = ImGui::Button(label.c_str());
            ask               = ImGui::ImageButton(
                ImTextureID(TextureManager::GetDescriptorGpuHandle(textureIndex_).ptr),
                ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), 4, ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1));

            return ask;
        };

        ImGui::Text("Texture Path: %s", texturePath_.c_str());
        if (askLoadTexture(_parentLabel)) {
            std::string directory;
            std::string fileName;
            if (myfs::selectFileDialog(kApplicationResourceDirectory, directory, fileName, {"png"})) {
                auto SetPath = std::make_unique<SetterCommand<std::string>>(&texturePath_, kApplicationResourceDirectory + "/" + directory + "/" + fileName);
                CommandCombo commandCombo;
                commandCombo.AddCommand(std::move(SetPath));
                commandCombo.SetFuncOnAfterCommand([this]() {
                    textureIndex_ = TextureManager::LoadTexture(texturePath_);
                },
                    true);
                EditorController::GetInstance()->PushCommand(std::make_unique<CommandCombo>(commandCombo));
            }
        };

        ImGui::TreePop();
    }

    label = " Emitter##" + _parentLabel;

    if (ImGui::TreeNode(label.c_str())) {
        ImGui::SeparatorText("Emitter Shape");
        DragGuiVectorCommand("Center##" + _parentLabel, shapeBuffer_->center);
        DragGuiVectorCommand("Size##" + _parentLabel, shapeBuffer_->size, 0.01f, 0.001f);

        label         = "Sphere##" + _parentLabel;
        int shapeType = shapeBuffer_->isBox ? 1 : 0; // 0:球形, 1:立方体
        ImGui::RadioButton(label.c_str(), &shapeType, 0);
        label = "Box##" + _parentLabel;
        ImGui::RadioButton(label.c_str(), &shapeType, 1);

        if (shapeType != (int)shapeBuffer_->isBox) {
            auto command = std::make_unique<SetterCommand<uint32_t>>(&shapeBuffer_->isBox, (uint32_t)shapeType);
            EditorController::GetInstance()->PushCommand(std::move(command));
        }

        ImGui::Spacing();

        label           = "isEmitEdge##" + _parentLabel;
        bool isEmitEdge = shapeBuffer_->isEmitEdge != 0;
        if (ImGui::Checkbox(label.c_str(), &isEmitEdge)) {
            auto command = std::make_unique<SetterCommand<uint32_t>>(&shapeBuffer_->isEmitEdge, (uint32_t)isEmitEdge);
            EditorController::GetInstance()->PushCommand(std::move(command));
        }
        ImGui::Spacing();

        label = "Emit##" + _parentLabel;
        if (ImGui::Button(label.c_str())) {
            shapeBuffer_->frequency = 0.f; // Emitボタンを押したら、frequencyをリセット
        }

        ImGui::Spacing();

        ImGui::SeparatorText("Particle Data");

        ImGui::Spacing();

        DragGuiCommand("FrequencyTime##" + _parentLabel, shapeBuffer_->frequencyTime, 0.01f, 0.001f);

        ImGui::Spacing();

        DragGuiCommand<uint32_t>("MinEmitParticleCount##" + _parentLabel, shapeBuffer_->minParticleCount, 1.f, 0, shapeBuffer_->particleSize, "%3d");
        DragGuiCommand<uint32_t>("MaxEmitParticleCount##" + _parentLabel, shapeBuffer_->maxParticleCount, 1.f, 0, shapeBuffer_->particleSize, "%3d");

        shapeBuffer_->maxParticleCount = (std::max)(shapeBuffer_->minParticleCount, shapeBuffer_->maxParticleCount);
        shapeBuffer_->minParticleCount = (std::min)(shapeBuffer_->minParticleCount, shapeBuffer_->maxParticleCount);

        ImGui::Spacing();

        DragGuiCommand("MinLifeTime##" + _parentLabel, shapeBuffer_->minLifeParticleTime, 0.01f, 0.001f);
        DragGuiCommand("MaxLifeTime##" + _parentLabel, shapeBuffer_->maxLifeParticleTime, 0.01f, 0.001f);

        shapeBuffer_->maxLifeParticleTime = (std::max)(shapeBuffer_->minLifeParticleTime, shapeBuffer_->maxLifeParticleTime);
        shapeBuffer_->minLifeParticleTime = (std::min)(shapeBuffer_->minLifeParticleTime, shapeBuffer_->maxLifeParticleTime);

        ImGui::Spacing();

        DragGuiVectorCommand("MinVelocity##" + _parentLabel, shapeBuffer_->minVelocity, 0.01f);
        DragGuiVectorCommand("MaxVelocity##" + _parentLabel, shapeBuffer_->maxVelocity, 0.01f);

        shapeBuffer_->minVelocity = MinElement(shapeBuffer_->minVelocity, shapeBuffer_->maxVelocity);
        shapeBuffer_->maxVelocity = MaxElement(shapeBuffer_->minVelocity, shapeBuffer_->maxVelocity);

        ImGui::Spacing();

        DragGuiVectorCommand("MinScale##" + _parentLabel, shapeBuffer_->minScale, 0.01f);
        DragGuiVectorCommand("MaxScale##" + _parentLabel, shapeBuffer_->maxScale, 0.01f);
        shapeBuffer_->minScale = MinElement(shapeBuffer_->minScale, shapeBuffer_->maxScale);
        shapeBuffer_->maxScale = MaxElement(shapeBuffer_->minScale, shapeBuffer_->maxScale);

        ImGui::Spacing();

        DragGuiVectorCommand<3, float>("MinColor##" + _parentLabel, shapeBuffer_->minColor, 0.01f, 0.f, 1.f);
        DragGuiVectorCommand<3, float>("MaxColor##" + _parentLabel, shapeBuffer_->maxColor, 0.01f, 0.f, 1.f);

        shapeBuffer_->minColor = MinElement(shapeBuffer_->minColor, shapeBuffer_->maxColor);
        shapeBuffer_->maxColor = MaxElement(shapeBuffer_->minColor, shapeBuffer_->maxColor);

        ImGui::TreePop();
    }
#endif // _DEBUG
}

void GpuParticleEmitter::Finalize() {
    DxDescriptorHeap<DxDescriptorHeapType::CBV_SRV_UAV>* srvuavHeap = Engine::GetInstance()->GetSrvHeap();

    particleResource_.Finalize();

    srvuavHeap->ReleaseDescriptor(particleUavDescriptor_); // UAVディスクリプタを解放
    // cbv_srv_uav heap
    srvuavHeap->ReleaseDescriptor(particleSrvDescriptor_); // SRVディスクリプタを解放

    freeIndexResource_.Finalize();
    // cbv_srv_uav heap
    srvuavHeap->ReleaseDescriptor(freeIndexUavDescriptor_); // UAVディスクリプタを解放

    freeListResource_.Finalize();
    // cbv_srv_uav heap
    srvuavHeap->ReleaseDescriptor(freeListUavDescriptor_); // UAVディスクリプタを解放

    if (materialBuffer_.GetResource().GetResource()) {
        materialBuffer_.Finalize();
    }
    if (shapeBuffer_.GetResource().GetResource()) {
        shapeBuffer_.Finalize();
    }

    mesh_.Finalize();

    texturePath_.clear();
    textureIndex_ = 0;
    isActive_     = false;
}

void GpuParticleEmitter::CreateBuffer() {
    DxDevice* dxDevice = Engine::GetInstance()->GetDxDevice();
    // cbv_srv_uav heap
    DxDescriptorHeap<DxDescriptorHeapType::CBV_SRV_UAV>* srvuavHeap = Engine::GetInstance()->GetSrvHeap();

    // materialBuffer が未作成の場合は、バッファを生成
    if (!materialBuffer_.GetResource().GetResource()) {
        materialBuffer_.CreateBuffer(dxDevice->device_);
        materialBuffer_.ConvertToBuffer();
    }

    if (!shapeBuffer_.GetResource().GetResource()) {
        shapeBuffer_.CreateBuffer(dxDevice->device_);
    }

    // particleResource が未作成の場合は、UAVバッファを生成
    if (!particleResource_.GetResource()) {
        particleResource_.CreateUAVBuffer(
            dxDevice->device_,
            sizeof(GpuParticleData::ConstantBuffer) * shapeBuffer_->particleSize,
            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
            D3D12_HEAP_TYPE_DEFAULT);
    }

    // srv が 未作成の場合は、バッファを生成
    if (particleSrvDescriptor_.GetIndex() < 0) {
        // SRVディスクリプタ 生成
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format                     = DXGI_FORMAT_UNKNOWN; // SRVはフォーマットを持たない
        srvDesc.Shader4ComponentMapping    = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.ViewDimension              = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Buffer.FirstElement        = 0;
        srvDesc.Buffer.NumElements         = shapeBuffer_->particleSize;
        srvDesc.Buffer.StructureByteStride = sizeof(GpuParticleData::ConstantBuffer);
        particleSrvDescriptor_             = srvuavHeap->CreateDescriptor(srvDesc, &particleResource_);
    }

    // UAVが未作成の場合は、UAVを生成
    if (particleUavDescriptor_.GetIndex() < 0) {

        // UAVディスクリプタ 生成
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
        uavDesc.Format                      = DXGI_FORMAT_UNKNOWN; // UAVはフォーマットを持たない
        uavDesc.ViewDimension               = D3D12_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.FirstElement         = 0;
        uavDesc.Buffer.NumElements          = shapeBuffer_->particleSize;
        uavDesc.Buffer.CounterOffsetInBytes = 0; // カウンターオフセットは0
        uavDesc.Buffer.Flags                = D3D12_BUFFER_UAV_FLAG_NONE; // 特にフラグは必要ない
        uavDesc.Buffer.StructureByteStride  = sizeof(GpuParticleData::ConstantBuffer);
        particleUavDescriptor_              = srvuavHeap->CreateDescriptor(uavDesc, &particleResource_);
    }

    // freeIndexResource が未作成の場合は、UAVバッファを生成
    if (!freeIndexResource_.GetResource()) {
        freeIndexResource_.CreateUAVBuffer(
            dxDevice->device_,
            sizeof(int) * shapeBuffer_->particleSize,
            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
            D3D12_HEAP_TYPE_DEFAULT);
    }
    // freeIndexUavDescriptor_ が未作成の場合は、UAVを生成
    if (freeIndexUavDescriptor_.GetIndex() < 0) {
        // UAVディスクリプタ 生成
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
        uavDesc.Format                      = DXGI_FORMAT_UNKNOWN; // UAVはフォーマットを持たない
        uavDesc.ViewDimension               = D3D12_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.FirstElement         = 0;
        uavDesc.Buffer.NumElements          = shapeBuffer_->particleSize;
        uavDesc.Buffer.CounterOffsetInBytes = 0; // カウンターオフセットは0
        uavDesc.Buffer.Flags                = D3D12_BUFFER_UAV_FLAG_NONE; // 特にフラグは必要ない
        uavDesc.Buffer.StructureByteStride  = sizeof(int);
        freeIndexUavDescriptor_             = srvuavHeap->CreateDescriptor(uavDesc, &freeIndexResource_);
    }

    // freeListResource が未作成の場合は、UAVバッファを生成
    if (!freeListResource_.GetResource()) {
        freeListResource_.CreateUAVBuffer(
            dxDevice->device_,
            sizeof(int) * shapeBuffer_->particleSize,
            D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
            D3D12_HEAP_TYPE_DEFAULT);
    }
    // freeListUavDescriptor_ が未作成の場合は、UAVを生成
    if (freeListUavDescriptor_.GetIndex() < 0) {
        // UAVディスクリプタ 生成
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
        uavDesc.Format                      = DXGI_FORMAT_UNKNOWN; // UAVはフォーマットを持たない
        uavDesc.ViewDimension               = D3D12_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.FirstElement         = 0;
        uavDesc.Buffer.NumElements          = shapeBuffer_->particleSize;
        uavDesc.Buffer.CounterOffsetInBytes = 0; // カウンターオフセットは0
        uavDesc.Buffer.Flags                = D3D12_BUFFER_UAV_FLAG_NONE; // 特にフラグは必要ない
        uavDesc.Buffer.StructureByteStride  = sizeof(int);
        freeListUavDescriptor_              = srvuavHeap->CreateDescriptor(uavDesc, &freeListResource_);
    }
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
        {"particleSize", p.shapeBuffer_->particleSize},
        {"texturePath", p.texturePath_},
        {"Material", p.materialBuffer_.openData_},
        {"center", p.shapeBuffer_->center},
        {"size", p.shapeBuffer_->size},
        {"frequencyTime", p.shapeBuffer_->frequencyTime},
        {"emitMinParticleCount", p.shapeBuffer_->minParticleCount},
        {"emitMaxParticleCount", p.shapeBuffer_->maxParticleCount},
        {"minLifeParticleTime", p.shapeBuffer_->minLifeParticleTime},
        {"maxLifeParticleTime", p.shapeBuffer_->maxLifeParticleTime},
        {"minVelocity", p.shapeBuffer_->minVelocity},
        {"maxVelocity", p.shapeBuffer_->maxVelocity},
        {"minScale", p.shapeBuffer_->minScale},
        {"maxScale", p.shapeBuffer_->maxScale},
        {"minColor", p.shapeBuffer_->minColor},
        {"maxColor", p.shapeBuffer_->maxColor},
        {"isBox", p.shapeBuffer_->isBox},
        {"isEmitEdge", p.shapeBuffer_->isEmitEdge},
        {"blendMode", static_cast<int>(p.blendMode_)},
    };
}

void from_json(const nlohmann::json& j, GpuParticleEmitter& p) {
    j.at("isActive").get_to(p.isActive_);
    j.at("particleSize").get_to(p.shapeBuffer_->particleSize);
    j.at("texturePath").get_to(p.texturePath_);

    j.at("Material").get_to(p.materialBuffer_.openData_);

    j.at("center").get_to(p.shapeBuffer_->center);
    j.at("size").get_to(p.shapeBuffer_->size);
    j.at("frequencyTime").get_to(p.shapeBuffer_->frequencyTime);
    j.at("emitMinParticleCount").get_to(p.shapeBuffer_->minParticleCount);
    j.at("emitMaxParticleCount").get_to(p.shapeBuffer_->maxParticleCount);
    j.at("minLifeParticleTime").get_to(p.shapeBuffer_->minLifeParticleTime);
    j.at("maxLifeParticleTime").get_to(p.shapeBuffer_->maxLifeParticleTime);
    j.at("minVelocity").get_to(p.shapeBuffer_->minVelocity);
    j.at("maxVelocity").get_to(p.shapeBuffer_->maxVelocity);
    j.at("minScale").get_to(p.shapeBuffer_->minScale);
    j.at("maxScale").get_to(p.shapeBuffer_->maxScale);
    j.at("minColor").get_to(p.shapeBuffer_->minColor);
    j.at("maxColor").get_to(p.shapeBuffer_->maxColor);
    j.at("isBox").get_to(p.shapeBuffer_->isBox);

    int blendModeInt = 0;
    j.at("blendMode").get_to(blendModeInt);
    p.blendMode_ = static_cast<BlendMode>(blendModeInt);
}
