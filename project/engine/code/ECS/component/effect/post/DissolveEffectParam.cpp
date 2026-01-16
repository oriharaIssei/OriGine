#include "DissolveEffectParam.h"

/// engine
#define ENGINE_INCLUDE
#define RESOURCE_DIRECTORY
#include "engine/EngineInclude.h"
#include "scene/Scene.h"
#include "texture/TextureManager.h"
// directX12
#include "directX12/DxDevice.h"

/// util
#include "myFileSystem/MyFileSystem.h"
/// externals
#ifdef _DEBUG
#include "myGui/MyGui.h"
#endif // _DEBUG

using namespace OriGine;

void DissolveEffectParam::Initialize([[maybe_unused]] Scene* _scene, [[maybe_unused]] EntityHandle _entity) {

    if (!textureFilePath_.empty()) {
        textureIndex_ = TextureManager::LoadTexture(textureFilePath_);
    }

    if (isActive_) {
        paramBuffer_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
        materialBuffer_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
    }
}
void DissolveEffectParam::Finalize() {
    if (isActive_) {
        paramBuffer_.Finalize();
    }
    textureFilePath_.clear();
    textureIndex_ = 0;
    isActive_     = false;
}

void DissolveEffectParam::LoadTexture(const std::string& _filePath) {
    if (_filePath.empty()) {
        return;
    }
    textureFilePath_ = _filePath;
    textureIndex_    = TextureManager::LoadTexture(textureFilePath_);
}

void DissolveEffectParam::Play() {
    isActive_ = true;
    // buff の作成
    paramBuffer_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
    materialBuffer_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
}

void DissolveEffectParam::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] EntityHandle _handle, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    if (CheckBoxCommand("Active##" + _parentLabel, isActive_)) {
        // パラメータバッファの作成
        Play();
    }

    ImGui::Spacing();

    std::string label          = "MaterialIndex##" + _parentLabel;
    auto& materials            = _scene->GetComponents<Material>(_handle);
    int32_t entityMaterialSize = static_cast<int32_t>(materials.size());

    if (entityMaterialSize <= 0) {
        ImGui::InputInt(label.c_str(), &materialIndex_, 0, 0, ImGuiInputTextFlags_ReadOnly);
    } else {
        InputGuiCommand(label, materialIndex_);
        materialIndex_ = std::clamp(materialIndex_, 0, entityMaterialSize - 1);
    }
    label = "Material##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        if (materialIndex_ >= 0 && materialIndex_ < materials.size()) {
            materials[materialIndex_].Edit(_scene, _handle, label);
        } else {
            ImGui::Text("Material is null.");
        }
        ImGui::TreePop();
    }

    ImGui::Spacing();

    DragGuiCommand("Threshold##" + _parentLabel, paramBuffer_->threshold, 0.001f, 0.0f, 1.0f, "%.4f");
    DragGuiCommand("Edge Width##" + _parentLabel, paramBuffer_->edgeWidth, 0.001f, 0.0f, 1.0f, "%.4f");
    ColorEditGuiCommand("OutLine Color##" + _parentLabel, paramBuffer_->outLineColor);

    auto askLoadTexture = [this](const std::string& _label) {
        bool ask = false;

        std::string label = "Load Texture##" + _label;
        ask               = ImGui::Button(label.c_str());
        ask               = ImGui::ImageButton(
            ImTextureID(TextureManager::GetDescriptorGpuHandle(textureIndex_).ptr),
            ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), 4, ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1));

        return ask;
    };

    ImGui::Text("Texture Directory: %s", textureFilePath_.c_str());
    if (askLoadTexture(_parentLabel)) {
        std::string directory;
        std::string fileName;
        if (myfs::SelectFileDialog(kApplicationResourceDirectory, directory, fileName, {"png"})) {
            auto SetPath = std::make_unique<SetterCommand<std::string>>(&textureFilePath_, kApplicationResourceDirectory + "/" + directory + "/" + fileName);
            CommandCombo commandCombo;
            commandCombo.AddCommand(std::move(SetPath));
            commandCombo.SetFuncOnAfterCommand([this]() {
                textureIndex_ = TextureManager::LoadTexture(textureFilePath_);
            },
                true);
            OriGine::EditorController::GetInstance()->PushCommand(std::make_unique<CommandCombo>(commandCombo));
        }
    };

#endif // _DEBUG
}

void OriGine::to_json(nlohmann::json& _j, const DissolveEffectParam& _comp) {
    _j = nlohmann::json{
        {"isActive", _comp.isActive_},
        {"textureFilePath", _comp.textureFilePath_},
        {"edgeWidth", _comp.paramBuffer_->edgeWidth},
        {"threshold", _comp.paramBuffer_->threshold},
        {"outLineColor", _comp.paramBuffer_->outLineColor},
        {"materialIndex", _comp.materialIndex_}};
}
void OriGine::from_json(const nlohmann::json& _j, DissolveEffectParam& _comp) {
    _j.at("isActive").get_to(_comp.isActive_);
    _j.at("textureFilePath").get_to(_comp.textureFilePath_);

    _j.at("edgeWidth").get_to(_comp.paramBuffer_->edgeWidth);
    _j.at("threshold").get_to(_comp.paramBuffer_->threshold);
    _j.at("outLineColor").get_to(_comp.paramBuffer_->outLineColor);
    if (_j.contains("materialIndex")) {
        _j.at("materialIndex").get_to(_comp.materialIndex_);
    }
}
