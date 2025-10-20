#include "GradationTextureComponent.h"

#define ENGINE_INCLUDE
#define RESOURCE_DIRECTORY
#include "EngineInclude.h"
#include "scene/Scene.h"
#include "texture/TextureManager.h"

#ifdef _DEBUG
#include "myFileSystem/MyFileSystem.h"
#include "myGui/MyGui.h"
#endif // _DEBUG

void to_json(nlohmann::json& j, const GradationTextureComponent& _g) {
    j["isActive"]      = _g.isActive_;
    j["texturePath"]   = _g.texturePath_;
    j["materialIndex"] = _g.materialIndex_;

    j["inputChannel"]  = static_cast<int32_t>(_g.paramBuff_.openData_.inputChannel);
    j["outputChannel"] = static_cast<int32_t>(_g.paramBuff_.openData_.outputChannel);
}

void from_json(const nlohmann::json& j, GradationTextureComponent& _g) {
    _g.isActive_      = j.value("isActive", true);
    _g.texturePath_   = j.value("texturePath", "");
    _g.materialIndex_ = j.value("materialIndex", -1);

    _g.paramBuff_.openData_.inputChannel  = static_cast<ColorChannel>(j.value("inputChannel", 0));
    _g.paramBuff_.openData_.outputChannel = static_cast<ColorChannel>(j.value("outputChannel", 0));
}

void GradationTextureComponent::Initialize(Entity* /*_entity*/) {
    auto& device = Engine::getInstance()->getDxDevice()->getDevice();
    paramBuff_.CreateBuffer(device);
    materialBuff_.CreateBuffer(device);

    if (!texturePath_.empty()) {
        textureIndex_ = TextureManager::LoadTexture(texturePath_);
    }
}

void GradationTextureComponent::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] Entity* _entity, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    InputGuiCommand("isActive##" + _parentLabel, isActive_);

    ImGui::Text("Texture Path : %s", texturePath_.c_str());

    std::string label = "LoadTexture##" + _parentLabel;
    if (AskLoadTextureButton(textureIndex_, label)) {
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
    }

    label                      = "MaterialIndex##" + _parentLabel;
    auto materials             = _scene->getComponents<Material>(_entity);
    int32_t entityMaterialSize = materials != nullptr ? static_cast<int32_t>(materials->size()) : 0;

    InputGuiCommand(label, materialIndex_);
    materialIndex_ = std::clamp(materialIndex_, -1, entityMaterialSize - 1);
    if (materialIndex_ >= 0) {
        label = "Material##" + _parentLabel;
        if (ImGui::TreeNode(label.c_str())) {
            materials->operator[](materialIndex_).Edit(_scene, _entity, _parentLabel);
            ImGui::TreePop();
        }
    }

    ImGui::SeparatorText("InputColorChannel");
    const char* channelItems[] = {"R", "G", "B", "A"};
    int32_t inputChannel       = static_cast<int32_t>(paramBuff_.openData_.inputChannel);
    if (ImGui::Combo(("InputChannel##" + _parentLabel).c_str(), &inputChannel, channelItems, IM_ARRAYSIZE(channelItems))) {
        paramBuff_.openData_.inputChannel = static_cast<ColorChannel>(inputChannel);
    }
    ImGui::SeparatorText("OutputColorChannel");
    int32_t outputChannel = static_cast<int32_t>(paramBuff_.openData_.outputChannel);
    if (ImGui::Combo(("OutputChannel##" + _parentLabel).c_str(), &outputChannel, channelItems, IM_ARRAYSIZE(channelItems))) {
        paramBuff_.openData_.outputChannel = static_cast<ColorChannel>(outputChannel);
    }

#endif // _DEBUG
}

void GradationTextureComponent::Finalize() {
    paramBuff_.Finalize();
    materialBuff_.Finalize();
}

void GradationTextureComponent::LoadTexture(const std::string& _texturePath) {
    texturePath_ = _texturePath;
    if (!texturePath_.empty()) {
        textureIndex_ = TextureManager::LoadTexture(texturePath_);
    }
}
