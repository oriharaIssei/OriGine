#include "PlaneRenderer.h"

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

void PlaneRenderer::Initialize(Scene* _scene, EntityHandle _hostEntity) {
    MeshRenderer::Initialize(_scene, _hostEntity);

    // _mesh Init
    if (!meshGroup_->empty()) {
        meshGroup_->clear();
    }

    meshGroup_->emplace_back(MeshType());
    auto& mesh = meshGroup_->back();
    mesh.Initialize(4, 6);

    transformBuff_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);
    materialBuff_.CreateBuffer(Engine::GetInstance()->GetDxDevice()->device_);

    // create _mesh
    CreateMesh(&mesh);

    // loadTexture
    if (!textureFilePath_.empty()) {
        textureIndex_ = TextureManager::LoadTexture(textureFilePath_);
    }
}

void PlaneRenderer::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] EntityHandle _handle, [[maybe_unused]] const std::string& _parentLabel) {
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
    auto& materials            = _scene->GetComponents<Material>(_handle);
    int32_t entityMaterialSize = static_cast<int32_t>(materials.size());
    InputGuiCommand(label, materialIndex_);

    materialIndex_ = std::clamp(materialIndex_, -1, entityMaterialSize);
    if (materialIndex_ >= 0) {
        label = "Material##" + _parentLabel;
        if (ImGui::TreeNode(label.c_str())) {
            materials[materialIndex_].Edit(_scene, _handle, "Material" + _parentLabel);
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

    ImGui::Spacing();
    ImGui::SeparatorText("Transform");
    ImGui::Spacing();

    // buffer Datas
    label = "Transform##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        transformBuff_.openData_.Edit(_scene, _handle, _parentLabel);

        transformBuff_.ConvertToBuffer();
        ImGui::TreePop();
    }

#endif // _DEBUG
}

void OriGine::to_json(nlohmann::json& j, const PlaneRenderer& r) {
    j["isRenderer"] = r.isRender_;
    j["isCulling"]  = r.isCulling_;
    j["blendMode"]  = static_cast<int32_t>(r.currentBlend_);

    j["textureFilePath"] = r.textureFilePath_;

    to_json(j["transform"], r.transformBuff_.openData_);
    j["materialIndex"] = r.materialIndex_;
}

void OriGine::from_json(const nlohmann::json& j, PlaneRenderer& r) {
    j.at("isRenderer").get_to(r.isRender_);
    if (j.contains("isCulling")) {
        j.at("isCulling").get_to(r.isCulling_);
    }
    int32_t blendMode = 0;
    j.at("blendMode").get_to(blendMode);
    r.currentBlend_ = static_cast<BlendMode>(blendMode);

    if (j.contains("textureFilePath")) {
        j.at("textureFilePath").get_to(r.textureFilePath_);
    } else {
        std::string directory, fileName;
        j.at("textureDirectory").get_to(directory);
        j.at("textureFileName").get_to(fileName);
        if (!fileName.empty()) {
            r.textureFilePath_ = directory + "/" + fileName;
        }
    }

    from_json(j.at("transform"), r.transformBuff_.openData_);
    if (j.find("materialIndex") != j.end()) {
        r.materialIndex_ = j.at("materialIndex");
    }
}
