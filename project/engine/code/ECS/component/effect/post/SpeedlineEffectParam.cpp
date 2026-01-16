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

using namespace OriGine;

void SpeedlineEffectParam::Initialize([[maybe_unused]] Scene* _scene, [[maybe_unused]] EntityHandle _entity) {
    if (isActive_) {
        cBuffer_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
    }
    LoadRadialTexture(radialTextureFilePath_);
}

void SpeedlineEffectParam::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] EntityHandle _entity, [[maybe_unused]] const std::string& _parentLabel) {
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
            auto command        = std::make_unique<SetterCommand<std::string>>(&radialTextureFilePath_, newPath, [this](std::string* _path) { this->LoadRadialTexture(*_path); }, true, nullptr);
            OriGine::EditorController::GetInstance()->PushCommand(std::move(command));
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
    if (radialTextureFilePath_.empty()) {
        radialTextureIndex_ = 0;
        return;
    }
    radialTextureIndex_ = TextureManager::LoadTexture(radialTextureFilePath_);
}

void OriGine::to_json(nlohmann::json& _j, const SpeedlineEffectParam& _comp) {
    _j = nlohmann::json{
        {"isActive", _comp.isActive_},
        {"screenCenterUV", _comp.cBuffer_.openData_.screenCenterUV},
        {"intensity", _comp.cBuffer_.openData_.intensity},
        {"density", _comp.cBuffer_.openData_.density},
        {"color", _comp.cBuffer_.openData_.color},
        {"time", _comp.cBuffer_.openData_.time},
        {"fadeStart", _comp.cBuffer_.openData_.fadeStart},
        {"fadePow", _comp.cBuffer_.openData_.fadePow},
        {"radialTextureFilePath", _comp.radialTextureFilePath_},
    };
}

void OriGine::from_json(const nlohmann::json& _j, SpeedlineEffectParam& _comp) {
    _j.at("isActive").get_to(_comp.isActive_);
    _j.at("screenCenterUV").get_to(_comp.cBuffer_.openData_.screenCenterUV);
    _j.at("intensity").get_to(_comp.cBuffer_.openData_.intensity);
    _j.at("density").get_to(_comp.cBuffer_.openData_.density);
    _j.at("color").get_to(_comp.cBuffer_.openData_.color);
    _j.at("time").get_to(_comp.cBuffer_.openData_.time);
    _j.at("fadeStart").get_to(_comp.cBuffer_.openData_.fadeStart);
    _j.at("fadePow").get_to(_comp.cBuffer_.openData_.fadePow);
    _j.at("radialTextureFilePath").get_to(_comp.radialTextureFilePath_);
}
