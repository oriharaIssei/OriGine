#include "DistortionEffectParam.h"

/// engine
#include "directX12/DxDevice.h"
#include "Engine.h"
#include "scene/Scene.h"
#include "asset/AssetSystem.h"
// asset
#include "asset/TextureAsset.h"
/// util
#include "myFileSystem/MyFileSystem.h"
/// externals
#ifdef _DEBUG
#include "myGui/MyGui.h"
#endif // _DEBUG

using namespace OriGine;

void DistortionEffectParam::Initialize([[maybe_unused]] Scene* _scene, [[maybe_unused]] const EntityHandle& _hostEntity) {
    effectParamData_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
    effectParamData_.ConvertToBuffer();
    materialBuffer_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
    materialBuffer_.ConvertToBuffer(ColorAndUvTransform());

    LoadTexture(texturePath_);
}

void DistortionEffectParam::LoadTexture(const std::string& _path) {
    texturePath_ = _path;
    if (texturePath_.empty()) {
        textureIndex_ = 0;
        return;
    }
    textureIndex_ = AssetSystem::GetInstance()->GetManager<TextureAsset>()->LoadAsset(texturePath_);
}

void DistortionEffectParam::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] const EntityHandle& _handle, [[maybe_unused]] const std::string& _parentLabel) {

#ifdef _DEBUG
    CheckBoxCommand("Active##" + _parentLabel, isActive_);

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

    ImGui::Spacing();

    DragGuiVectorCommand("Distortion Bias##" + _parentLabel, effectParamData_->distortionBias, 0.01f);
    DragGuiVectorCommand("Distortion Strength##" + _parentLabel, effectParamData_->distortionStrength, 0.01f);

    ImGui::Separator();

    auto askLoadTexture = [this]([[maybe_unused]] const std::string& _parentLabel) {
        bool ask          = false;
        std::string label = "Load Texture##" + _parentLabel;
        ask               = ImGui::Button(label.c_str());
        ask |= ImGui::ImageButton(
            ImTextureID(AssetSystem::GetInstance()->GetManager<TextureAsset>()->GetAsset(textureIndex_).srv.GetGpuHandle().ptr),
            ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), 4, ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1));

        return ask;
    };
    ImGui::Text("Texture Directory: %s", texturePath_.c_str());
    if (askLoadTexture(_parentLabel)) {
        std::string directory;
        std::string fileName;
        if (myfs::SelectFileDialog(kApplicationResourceDirectory, directory, fileName, {"png"})) {
            auto SetPath = std::make_unique<SetterCommand<std::string>>(&texturePath_, kApplicationResourceDirectory + "/" + directory + "/" + fileName);
            CommandCombo commandCombo;
            commandCombo.AddCommand(std::move(SetPath));
            commandCombo.SetFuncOnAfterCommand([this]() {
                LoadTexture(texturePath_);
            },
                true);
            OriGine::EditorController::GetInstance()->PushCommand(std::make_unique<CommandCombo>(commandCombo));
        }
    };

#endif // _DEBUG
}

void DistortionEffectParam::Finalize() {
    effectParamData_.Finalize();
}

void OriGine::to_json(nlohmann::json& _j, const DistortionEffectParam& _comp) {
    _j["distortionBias"]     = _comp.effectParamData_->distortionBias;
    _j["distortionStrength"] = _comp.effectParamData_->distortionStrength;

    _j["isActive"]      = _comp.isActive_;
    _j["materialIndex"] = _comp.materialIndex_;
    _j["texturePath"]   = _comp.texturePath_;
}

void OriGine::from_json(const nlohmann::json& _j, DistortionEffectParam& _comp) {
    _comp.effectParamData_->distortionBias     = _j.value("distortionBias", Vec2f());
    _comp.effectParamData_->distortionStrength = _j.value("distortionStrength", Vec2f());

    if (_j.contains("materialIndex")) {
        _j.at("materialIndex").get_to(_comp.materialIndex_);
    }
    if (_j.contains("isActive")) {
        _j.at("isActive").get_to(_comp.isActive_);
    }

    if (_j.contains("texturePath")) {
        _j.at("texturePath").get_to(_comp.texturePath_);
    } else if (_j.contains("textuerPath")) {
        _j.at("textuerPath").get_to(_comp.texturePath_);
    }
}
