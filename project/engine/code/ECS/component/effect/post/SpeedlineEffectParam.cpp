#include "SpeedlineEffectParam.h"

/// Engine
#define RESOURCE_DIRECTORY
#include "Engine.h"
#include "EngineInclude.h"
#include "texture/TextureManager.h"

// directX12
#include "directX12/DxDevice.h"

#ifdef _DEBUG
/// util
#include "myFileSystem/MyFileSystem.h"

#include "myGui/MyGui.h"
#endif // _DEBUG

void SpeedlineEffectParam::Initialize(Entity*) {
    if (isActive_) {
        cBuffer_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
    }
    if (!radialTextureFilePath_.empty()) {
        LoadRadialTexture(radialTextureFilePath_);
    }
}

void SpeedlineEffectParam::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] Entity* _entity, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG
    if (CheckBoxCommand("isActive##" + _parentLabel, isActive_)) {
        Play();
    }

    DragGuiVectorCommand("screenCenterUV##" + _parentLabel, cBuffer_.openData_.screenCenterUV, 0.01f);
    DragGuiCommand("intensity##" + _parentLabel, cBuffer_.openData_.intensity, 0.01f);
    DragGuiCommand("density##" + _parentLabel, cBuffer_.openData_.density, 0.01f);
    ColorEditGuiCommand("color##" + _parentLabel, cBuffer_.openData_.color);
    DragGuiCommand("time##" + _parentLabel, cBuffer_.openData_.time, 0.01f);
    DragGuiCommand("fadeStart##" + _parentLabel, cBuffer_.openData_.fadeStart, 0.01f);
    DragGuiCommand("fadePow##" + _parentLabel, cBuffer_.openData_.fadePow, 0.01f);

    ImGui::Text("radialTexture Path : %s", radialTextureFilePath_.c_str());

    auto askLoadTexture = [this]([[maybe_unused]] const std::string& _parentLabel) {
        bool ask          = false;
        std::string label = "Load Texture##" + _parentLabel;
        ask               = ImGui::Button(label.c_str());

        ask |= ImGui::ImageButton(
            ImTextureID(TextureManager::GetDescriptorGpuHandle(radialTextureIndex_).ptr),
            ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), 4, ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1));

        return ask;
    };
    if (askLoadTexture(_parentLabel)) {
        std::string directory, fileName;
        if (myfs::SelectFileDialog(kApplicationResourceDirectory + "/texture", directory, fileName, {"png"})) {
            std::string newPath = kApplicationResourceDirectory + "/texture/" + directory + "/" + fileName;
            auto command        = std::make_unique<SetterCommand<std::string>>(&radialTextureFilePath_, newPath, [this](std::string*) { this->radialTextureIndex_ = TextureManager::LoadTexture(radialTextureFilePath_); }, true, nullptr);
            EditorController::GetInstance()->PushCommand(std::move(command));
        }
    }
#endif // _DEBUG
}
void SpeedlineEffectParam::Finalize() {
    if (isActive_) {
        Stop();
    }
}

void SpeedlineEffectParam::Play() {
    if (isActive_) {
        return;
    }
    isActive_ = true;
    cBuffer_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
}

void SpeedlineEffectParam::Stop() {
    isActive_ = false;
    cBuffer_.Finalize();
}

void SpeedlineEffectParam::LoadRadialTexture(const std::string& _path) {
    radialTextureFilePath_ = _path;
    radialTextureIndex_    = TextureManager::LoadTexture(radialTextureFilePath_);
}

void to_json(nlohmann::json& j, const SpeedlineEffectParam& p) {
    j = nlohmann::json{
        {"isActive", p.isActive_},
        {"screenCenterUV", p.cBuffer_.openData_.screenCenterUV},
        {"intensity", p.cBuffer_.openData_.intensity},
        {"density", p.cBuffer_.openData_.density},
        {"color", p.cBuffer_.openData_.color},
        {"time", p.cBuffer_.openData_.time},
        {"fadeStart", p.cBuffer_.openData_.fadeStart},
        {"fadePow", p.cBuffer_.openData_.fadePow},
        {"radialTextureFilePath", p.radialTextureFilePath_},
    };
}

void from_json(const nlohmann::json& j, SpeedlineEffectParam& p) {
    j.at("isActive").get_to(p.isActive_);
    j.at("screenCenterUV").get_to(p.cBuffer_.openData_.screenCenterUV);
    j.at("intensity").get_to(p.cBuffer_.openData_.intensity);
    j.at("density").get_to(p.cBuffer_.openData_.density);
    j.at("color").get_to(p.cBuffer_.openData_.color);
    j.at("time").get_to(p.cBuffer_.openData_.time);
    j.at("fadeStart").get_to(p.cBuffer_.openData_.fadeStart);
    j.at("fadePow").get_to(p.cBuffer_.openData_.fadePow);
    j.at("radialTextureFilePath").get_to(p.radialTextureFilePath_);
}
