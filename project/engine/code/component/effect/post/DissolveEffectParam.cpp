#include "DissolveEffectParam.h"

/// engine
#define ENGINE_INCLUDE
#define RESOURCE_DIRECTORY
#include "engine/EngineInclude.h"
// directX12
#include "directX12/DxDevice.h"

#include "texture/TextureManager.h"

/// lib
#include "myFileSystem/MyFileSystem.h"

#ifdef _DEBUG
#include "myGui/MyGui.h"
#endif // _DEBUG

void DissolveEffectParam::Initialize(GameEntity* /*_entity*/) {

    if (!textureFilePath_.empty()) {
        textureIndex_ = TextureManager::LoadTexture(textureFilePath_);
    }

    if (isActive_) {
        paramBuffer_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
    }
}
void DissolveEffectParam::Finalize() {}

void DissolveEffectParam::LoadTexture(const std::string& filePath) {
    if (filePath.empty()) {
        return;
    }
    textureFilePath_ = filePath;
    textureIndex_    = TextureManager::LoadTexture(textureFilePath_);
}

void DissolveEffectParam::Play() {
    isActive_ = true;
    // buff の作成
    paramBuffer_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
}

bool DissolveEffectParam::Edit() {
    bool isEdited = false;

#ifdef _DEBUG

    isEdited = CheckBoxCommand("Active", isActive_);
    isEdited |= DragGuiCommand("Threshold", paramBuffer_->threshold, 0.001f, 0.0f, 1.0f, "%.4f");
    isEdited |= DragGuiCommand("Edge Width", paramBuffer_->edgeWidth, 0.001f, 0.0f, 1.0f, "%.4f");
    isEdited |= ColorEditGuiCommand("OutLine Color", paramBuffer_->outLineColor);

    auto askLoadTexture = [this]() {
        bool ask = false;

        ask = ImGui::Button("Load Texture");
        ask = ImGui::ImageButton(
            ImTextureID(TextureManager::getDescriptorGpuHandle(textureIndex_).ptr),
            ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), 4, ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1));

        return ask;
    };

    ImGui::Text("Texture Directory: %s", textureFilePath_.c_str());
    if (askLoadTexture()) {
        std::string directory;
        std::string fileName;
        if (myfs::selectFileDialog(kApplicationResourceDirectory, directory, fileName, {"png"})) {
            auto setPath = std::make_unique<SetterCommand<std::string>>(&textureFilePath_, kApplicationResourceDirectory + "/" + directory + "/" + fileName);
            CommandCombo commandCombo;
            commandCombo.addCommand(std::move(setPath));
            commandCombo.setFuncOnAfterCommand([this]() {
                textureIndex_ = TextureManager::LoadTexture(textureFilePath_);
            },
                true);
            EditorController::getInstance()->pushCommand(std::make_unique<CommandCombo>(commandCombo));

            isEdited = true;
        }
    };

    if (ImGui::TreeNode("UvTransform")) {
        isEdited |= DragGuiVectorCommand("Scale", paramBuffer_->uvTransform.scale_, 0.01f);
        isEdited |= DragGuiCommand("Rotate", paramBuffer_->uvTransform.rotate_, 0.01f);
        isEdited |= DragGuiVectorCommand("Translate", paramBuffer_->uvTransform.translate_, 0.01f);
        ImGui::TreePop();
    }

#endif // _DEBUG

    return isEdited;
}

void to_json(nlohmann::json& j, const DissolveEffectParam& param) {
    j = nlohmann::json{
        {"isActive", param.isActive_},
        {"textureFilePath", param.textureFilePath_},
        {"edgeWidth", param.paramBuffer_->edgeWidth},
        {"threshold", param.paramBuffer_->threshold},
        {"outLineColor", param.paramBuffer_->outLineColor},
        {"uvScale", param.paramBuffer_->uvTransform.scale_},
        {"uvRotate", param.paramBuffer_->uvTransform.rotate_},
        {"uvTranslate", param.paramBuffer_->uvTransform.translate_}};
}
void from_json(const nlohmann::json& j, DissolveEffectParam& param) {
    j.at("isActive").get_to(param.isActive_);
    j.at("textureFilePath").get_to(param.textureFilePath_);

    j.at("edgeWidth").get_to(param.paramBuffer_->edgeWidth);
    j.at("threshold").get_to(param.paramBuffer_->threshold);
    j.at("outLineColor").get_to(param.paramBuffer_->outLineColor);
    j.at("uvScale").get_to(param.paramBuffer_->uvTransform.scale_);
    j.at("uvRotate").get_to(param.paramBuffer_->uvTransform.rotate_);
    j.at("uvTranslate").get_to(param.paramBuffer_->uvTransform.translate_);
}
