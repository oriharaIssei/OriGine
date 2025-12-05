#include "CylinderRenderer.h"

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

void CylinderRenderer::Initialize(Entity* _hostEntity) {
    MeshRenderer::Initialize(_hostEntity);

    // culling しない
    isCulling_ = false;

    // _mesh Init
    if (!meshGroup_->empty()) {
        meshGroup_->clear();
    }

    meshGroup_->emplace_back(MeshType());
    auto& mesh = meshGroup_->back();

    transformBuff_.CreateBuffer(OriGine::Engine::GetInstance()->GetDxDevice()->device_);
    materialBuff_.CreateBuffer(OriGine::Engine::GetInstance()->GetDxDevice()->device_);

    // create _mesh
    CreateMesh(&mesh);

    // loadTexture
    if (!textureDirectory_.empty() && !textureFileName_.empty()) {
        textureIndex_ = TextureManager::LoadTexture(textureDirectory_ + "/" + textureFileName_);
    }
}

void CylinderRenderer::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] Entity* _entity, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG
    ImGui::SeparatorText("Material");
    ImGui::Spacing();

    CheckBoxCommand("IsRender##" + _parentLabel, isRender_);
    CheckBoxCommand("IsCulling##" + _parentLabel, isCulling_);

    ImGui::Text("BlendMode :");
    ImGui::SameLine();
    std::string label = "##BlendMode" + _parentLabel;
    if (ImGui::BeginCombo(label.c_str(), blendModeStr[(int32_t)currentBlend_].c_str())) {
        bool isSelected    = false;
        int32_t blendIndex = 0;
        for (auto& blendModeName : blendModeStr) {
            isSelected = blendModeName == blendModeStr[(int32_t)currentBlend_];

            if (ImGui::Selectable(blendModeName.c_str(), isSelected)) {
                EditorController::GetInstance()->PushCommand(
                    std::make_unique<SetterCommand<BlendMode>>(&currentBlend_, static_cast<BlendMode>(blendIndex)));
                break;
            }

            ++blendIndex;
        }
        ImGui::EndCombo();
    }

    ImGui::Spacing();

    label                      = "MaterialIndex##" + _parentLabel;
    auto materials             = _scene->GetComponents<Material>(_entity);
    int32_t entityMaterialSize = materials != nullptr ? static_cast<int32_t>(materials->size()) : 0;
    InputGuiCommand(label, materialIndex_);

    materialIndex_ = std::clamp(materialIndex_, -1, entityMaterialSize);
    if (materialIndex_ >= 0) {
        label = "Material##" + _parentLabel;
        if (ImGui::TreeNode(label.c_str())) {
            materials->operator[](materialIndex_).Edit(_scene, _entity, "Material" + _parentLabel);
            materialBuff_.ConvertToBuffer(materials->operator[](materialIndex_));
            ImGui::TreePop();
        }
    }

    ImGui::Spacing();

    // texture
    ImGui::Text("Texture Directory : %s", textureDirectory_.c_str());
    ImGui::Text("Texture FileName  : %s", textureFileName_.c_str());
    label = "LoadTexture##" + _parentLabel;
    if (AskLoadTextureButton(textureIndex_, _parentLabel)) {
        std::string directory = "";
        std::string fileName  = "";
        if (myfs::selectFileDialog(kApplicationResourceDirectory, directory, fileName, {"png"})) {
            auto commandCombo = std::make_unique<CommandCombo>();
            commandCombo->AddCommand(std::make_shared<SetterCommand<std::string>>(&textureDirectory_, kApplicationResourceDirectory + "/" + directory));
            commandCombo->AddCommand(std::make_shared<SetterCommand<std::string>>(&textureFileName_, fileName));
            commandCombo->SetFuncOnAfterCommand([this]() { textureIndex_ = TextureManager::LoadTexture(textureDirectory_ + "/" + textureFileName_); }, true);
            EditorController::GetInstance()->PushCommand(std::move(commandCombo));
        }
    }

    ImGui::Spacing();
    ImGui::SeparatorText("Cylinder");
    ImGui::Spacing();

    // shape
    label = "TopRadius##" + _parentLabel;
    DragGuiVectorCommand<2, float>(label.c_str(), primitive_.topRadius_, 0.01f, 0.01f, 0.f, "%.3f", [this](Vector<2, float>* _value) {
        primitive_.topRadius_ = *_value;
        CreateMesh(&meshGroup_->back());
    });

    label = "BottomRadius##" + _parentLabel;
    DragGuiVectorCommand<2, float>(label.c_str(), primitive_.bottomRadius_, 0.01f, 0.01f, 0.f, "%.3f", [this](Vector<2, float>* _value) {
        primitive_.bottomRadius_ = *_value;
        CreateMesh(&meshGroup_->back());
    });

    label = "Height##" + _parentLabel;
    DragGuiCommand<float>(label.c_str(), primitive_.height_, 0.01f, 0.01f, {}, "%.3f", [this](float* _value) {
        primitive_.height_ = *_value;
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

void to_json(nlohmann::json& j, const CylinderRenderer& c) {
    j["isRenderer"]       = c.isRender_;
    j["isCulling"]        = c.isCulling_;
    j["blendMode"]        = static_cast<int32_t>(c.currentBlend_);
    j["textureDirectory"] = c.textureDirectory_;
    j["textureFileName"]  = c.textureFileName_;
    to_json(j["transform"], c.transformBuff_.openData_);
    j["materialIndex"] = c.materialIndex_;

    j["topRadius"]    = c.primitive_.topRadius_;
    j["bottomRadius"] = c.primitive_.bottomRadius_;
    j["height"]       = c.primitive_.height_;
}

void from_json(const nlohmann::json& j, CylinderRenderer& c) {
    j.at("isRenderer").get_to(c.isRender_);
    if (j.contains("isCulling")) {
        j.at("isCulling").get_to(c.isCulling_ );
    }
    int32_t blendMode = 0;
    j.at("blendMode").get_to(blendMode);
    c.currentBlend_ = static_cast<BlendMode>(blendMode);
    j.at("textureDirectory").get_to(c.textureDirectory_);
    j.at("textureFileName").get_to(c.textureFileName_);
    from_json(j.at("transform"), c.transformBuff_.openData_);
    c.materialIndex_ = j.at("materialIndex");

    j.at("topRadius").get_to(c.primitive_.topRadius_);
    j.at("bottomRadius").get_to(c.primitive_.bottomRadius_);
    j.at("height").get_to(c.primitive_.height_);
}
