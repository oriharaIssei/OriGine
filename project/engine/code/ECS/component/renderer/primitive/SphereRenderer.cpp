#include "SphereRenderer.h"

/// engine
#define RESOURCE_DIRECTORY
#include "EngineInclude.h"
#include "scene/Scene.h"
// directX12
#include "directX12/DxDevice.h"
// module
#include "editor/EditorController.h"
#include "editor/IEditor.h"

#include "myFileSystem/MyFileSystem.h"

/// externals
#ifdef _DEBUG
#include "myGui/MyGui.h"
#include <imgui/imgui.h>
#endif // _DEBUG

using namespace OriGine;

void SphereRenderer::Initialize(Scene* _scene, EntityHandle _entity) {
    MeshRenderer::Initialize(_scene, _entity);

    // _mesh Init
    if (!meshGroup_->empty()) {
        meshGroup_->clear();
    }

    transformBuff_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
    materialBuff_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);

    meshGroup_->emplace_back(MeshType());
    auto& mesh = meshGroup_->back();
    // create _mesh
    CreateMesh(&mesh);

    // loadTexture
    if (!textureFilePath_.empty()) {
        textureIndex_ = TextureManager::LoadTexture(textureFilePath_);
    }
}

void SphereRenderer::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] EntityHandle _entity, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG
    ImGui::SeparatorText("Material");
    ImGui::Spacing();

    CheckBoxCommand("IsRender##" + _parentLabel, isRender_);
    CheckBoxCommand("IsCulling##" + _parentLabel, isCulling_);

    ImGui::Text("BlendMode :");
    ImGui::SameLine();
    std::string label = "##BlendMode" + _parentLabel;
    if (ImGui::BeginCombo(label.c_str(), kBlendModeStr[(int32_t)currentBlend_].c_str())) {
        bool isSelected    = false;
        int32_t blendIndex = 0;
        for (auto& blendModeName : kBlendModeStr) {
            isSelected = blendModeName == kBlendModeStr[(int32_t)currentBlend_];

            if (ImGui::Selectable(blendModeName.c_str(), isSelected)) {
                OriGine::EditorController::GetInstance()->PushCommand(
                    std::make_unique<SetterCommand<BlendMode>>(&currentBlend_, static_cast<BlendMode>(blendIndex)));
                break;
            }

            ++blendIndex;
        }
        ImGui::EndCombo();
    }

    ImGui::Spacing();

    label                      = "MaterialIndex##" + _parentLabel;
    auto& materials            = _scene->GetComponents<Material>(_entity);
    int32_t entityMaterialSize = static_cast<int32_t>(materials.size()) - 1;
    InputGuiCommand(label, materialIndex_);

    materialIndex_ = std::clamp(materialIndex_, -1, entityMaterialSize);
    if (materialIndex_ >= 0) {
        label = "Material##" + _parentLabel;
        if (ImGui::TreeNode(label.c_str())) {
            materials[materialIndex_].Edit(_scene, _entity, "Material" + _parentLabel);
            materialBuff_.ConvertToBuffer(materials[materialIndex_]);
            ImGui::TreePop();
        }
    }

    ImGui::Spacing();

    // texture
    ImGui::Text("Texture File Path : %s", textureFilePath_.c_str());
    label = "LoadTexture##" + _parentLabel;
    if (AskLoadTextureButton(textureIndex_, _parentLabel)) {
        std::string directory = "";
        std::string fileName  = "";
        if (myfs::SelectFileDialog(kApplicationResourceDirectory, directory, fileName, {"png"})) {
            auto commandCombo = std::make_unique<CommandCombo>();
            commandCombo->AddCommand(std::make_shared<SetterCommand<std::string>>(&textureFilePath_, kApplicationResourceDirectory + "/" + directory + "/" + fileName));
            commandCombo->SetFuncOnAfterCommand([this]() { textureIndex_ = TextureManager::LoadTexture(textureFilePath_); }, true);
            OriGine::EditorController::GetInstance()->PushCommand(std::move(commandCombo));
        }
    }

    // shape
    ImGui::Spacing();
    ImGui::SeparatorText("Sphere");
    ImGui::Spacing();

    // shape
    label = "Division Latitude##" + _parentLabel;
    DragGuiCommand<uint32_t>(label.c_str(), primitive_.divisionLatitude_, 1, 1, 1000, "%d", [this](uint32_t* _value) {
        primitive_.divisionLatitude_ = *_value;
        CreateMesh(&meshGroup_->back());
    });

    label = "Division Longitude##" + _parentLabel;
    DragGuiCommand<uint32_t>(label.c_str(), primitive_.divisionLongitude_, 1, 1, 1000, "%d", [this](uint32_t* _value) {
        primitive_.divisionLongitude_ = *_value;
        CreateMesh(&meshGroup_->back());
    });

    label = "Radius##" + _parentLabel;
    DragGuiCommand<float>(label.c_str(), primitive_.radius_, 0.01f, 0.01f, 100.f, "%.2f", [this](float* _value) {
        primitive_.radius_ = *_value;
        CreateMesh(&meshGroup_->back());
    });

    ImGui::Spacing();
    ImGui::SeparatorText("Transform");
    ImGui::Spacing();

    // buffer Datas
    label = "Transform##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        transformBuff_.openData_.Edit(_scene, _entity, _parentLabel);

        transformBuff_.ConvertToBuffer();
        ImGui::TreePop();
    }
#endif // _DEBUG
}

void OriGine::to_json(nlohmann::json& _j, const SphereRenderer& _comp) {
    _j["isRenderer"] = _comp.isRender_;
    _j["isCulling"]  = _comp.isCulling_;
    _j["blendMode"]  = static_cast<int32_t>(_comp.currentBlend_);

    _j["textureFilePath"] = _comp.textureFilePath_;

    to_json(_j["transform"], _comp.transformBuff_.openData_);
    _j["materialIndex"]     = _comp.materialIndex_;
    _j["radius"]            = _comp.primitive_.radius_;
    _j["divisionLatitude"]  = _comp.primitive_.divisionLatitude_;
    _j["divisionLongitude"] = _comp.primitive_.divisionLongitude_;
}
void OriGine::from_json(const nlohmann::json& _j, SphereRenderer& _comp) {
    _j.at("isRenderer").get_to(_comp.isRender_);
    if (_j.contains("isCulling")) {
        _j.at("isCulling").get_to(_comp.isCulling_);
    }
    int32_t blendMode = 0;
    _j.at("blendMode").get_to(blendMode);
    _comp.currentBlend_ = static_cast<BlendMode>(blendMode);

    if (_j.contains("textureFilePath")) {
        _j.at("textureFilePath").get_to(_comp.textureFilePath_);
    } else {
        std::string directory, fileName;
        _j.at("textureDirectory").get_to(directory);
        _j.at("textureFileName").get_to(fileName);
        if (!fileName.empty()) {
            _comp.textureFilePath_ = directory + "/" + fileName;
        }
    }

    from_json(_j.at("transform"), _comp.transformBuff_.openData_);
    if (_j.find("materialIndex") != _j.end()) {
        _comp.materialIndex_ = _j.at("materialIndex");
    }
    _j.at("radius").get_to(_comp.primitive_.radius_);
    _j.at("divisionLatitude").get_to(_comp.primitive_.divisionLatitude_);
    _j.at("divisionLongitude").get_to(_comp.primitive_.divisionLongitude_);
}
