#include "BoxRenderer.h"

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

void BoxRenderer::Initialize(GameEntity* _hostEntity) {
    MeshRenderer::Initialize(_hostEntity);

    // _mesh Init
    if (!meshGroup_->empty()) {
        meshGroup_->clear();
    }

    meshGroup_->emplace_back(MeshType());
    auto& mesh = meshGroup_->back();

    transformBuff_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
    materialBuff_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());

    // create _mesh
    createMesh(&mesh);

    // loadTexture
    if (!textureDirectory_.empty() && !textureFileName_.empty()) {
        textureIndex_ = TextureManager::LoadTexture(textureDirectory_ + "/" + textureFileName_);
    }
}

void BoxRenderer::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] GameEntity* _entity, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG
    ImGui::Text("BlendMode :");
    ImGui::SameLine();
    std::string label = "##BlendMode" + _parentLabel;
    if (ImGui::BeginCombo(label.c_str(), blendModeStr[(int32_t)currentBlend_].c_str())) {
        bool isSelected    = false;
        int32_t blendIndex = 0;
        for (auto& blendModeName : blendModeStr) {
            isSelected = blendModeName == blendModeStr[(int32_t)currentBlend_];

            if (ImGui::Selectable(blendModeName.c_str(), isSelected)) {
                EditorController::getInstance()->pushCommand(
                    std::make_unique<SetterCommand<BlendMode>>(&currentBlend_, static_cast<BlendMode>(blendIndex)));
                break;
            }

            ++blendIndex;
        }
        ImGui::EndCombo();
    }

    // texture
    ImGui::Text("Texture Directory : %s", textureDirectory_.c_str());
    ImGui::Text("Texture FileName  : %s", textureFileName_.c_str());
    label = "LoadTexture##" + _parentLabel;
    if (ImGui::Button(label.c_str())) {
        std::string directory = "";
        std::string fileName  = "";
        if (myfs::selectFileDialog(kApplicationResourceDirectory, directory, fileName, {"png"})) {
            auto commandCombo = std::make_unique<CommandCombo>();
            commandCombo->addCommand(std::make_shared<SetterCommand<std::string>>(&textureDirectory_, kApplicationResourceDirectory + "/" + directory));
            commandCombo->addCommand(std::make_shared<SetterCommand<std::string>>(&textureFileName_, fileName));
            commandCombo->setFuncOnAfterCommand([this]() { textureIndex_ = TextureManager::LoadTexture(textureDirectory_ + "/" + textureFileName_); }, true);
            EditorController::getInstance()->pushCommand(std::move(commandCombo));
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // shape
    Vec3f size = primitive_.getSize();
    label      = "Size##" + _parentLabel;
    DragGuiVectorCommand<3, float>(label.c_str(), size, 0.01f, 0.01f, {}, "%.3f", [this](Vec<3, float>* _value) {
        primitive_.setSize(*_value);
        createMesh(&meshGroup_->back());
    });

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // buffer Datas
    label = "Transform##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        transformBuff_.openData_.Edit(_scene, _entity, _parentLabel);

        transformBuff_.ConvertToBuffer();
        ImGui::TreePop();
    }

    label                      = "MaterialIndex##" + _parentLabel;
    auto materials             = _scene->getComponents<Material>(_entity);
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

#endif // _DEBUG
}

void to_json(nlohmann::json& j, const BoxRenderer& r) {
    j["isRenderer"]       = r.isRender_;
    j["blendMode"]        = static_cast<int32_t>(r.currentBlend_);
    j["textureDirectory"] = r.textureDirectory_;
    j["textureFileName"]  = r.textureFileName_;
    to_json(j["transform"], r.transformBuff_.openData_);
    j["materialIndex"] = r.materialIndex_;

    j["size"] = r.primitive_.getSize();
}

void from_json(const nlohmann::json& j, BoxRenderer& r) {
    j.at("isRenderer").get_to(r.isRender_);
    int32_t blendMode = 0;
    j.at("blendMode").get_to(blendMode);
    r.currentBlend_ = static_cast<BlendMode>(blendMode);
    j.at("textureDirectory").get_to(r.textureDirectory_);
    j.at("textureFileName").get_to(r.textureFileName_);
    from_json(j.at("transform"), r.transformBuff_.openData_);
    if (j.find("materialIndex") != j.end()) {
        r.materialIndex_ = j.at("materialIndex");
    }

    Vec3f size = {1.0f, 1.0f, 1.0f};
    j.at("size").get_to(size);
    r.primitive_.setSize(size);
}
