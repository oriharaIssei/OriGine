#include "DissolveEffectParam.h"

/// engine
#define ENGINE_INCLUDE
#define RESOURCE_DIRECTORY
#include "engine/EngineInclude.h"
// directX12
#include "directX12/DxDevice.h"

#include "texture/TextureManager.h"


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
void DissolveEffectParam::Finalize() {
    if (isActive_) {
        paramBuffer_.Finalize();
    }
    textureFilePath_.clear();
    textureIndex_ = 0;
    isActive_     = false;
}

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

void DissolveEffectParam::Edit(Scene* /*_scene*/, GameEntity* /*_entity*/,[[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    if (CheckBoxCommand("Active##" + _parentLabel, isActive_)) {
            // パラメータバッファの作成
            paramBuffer_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
    }
    DragGuiCommand("Threshold##" + _parentLabel, paramBuffer_->threshold, 0.001f, 0.0f, 1.0f, "%.4f");
    DragGuiCommand("Edge Width##" + _parentLabel, paramBuffer_->edgeWidth, 0.001f, 0.0f, 1.0f, "%.4f");
    ColorEditGuiCommand("OutLine Color##" + _parentLabel, paramBuffer_->outLineColor);


    auto askLoadTexture = [this](const std::string& _parentLabel) {
        bool ask = false;

        std::string label = "Load Texture##" + _parentLabel;
        ask               = ImGui::Button(label.c_str());
        ask = ImGui::ImageButton(
            ImTextureID(TextureManager::getDescriptorGpuHandle(textureIndex_).ptr),
            ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), 4, ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1));

        return ask;
    };

    ImGui::Text("Texture Directory: %s", textureFilePath_.c_str());
    if (askLoadTexture(_parentLabel)) {
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
        }
    };
    std::string label = "UvTransform##" + _parentLabel;

    if (ImGui::TreeNode(label.c_str())) {
        DragGuiVectorCommand("Scale##" + _parentLabel, paramBuffer_->uvTransform.scale_, 0.01f);
        DragGuiCommand("Rotate##" + _parentLabel, paramBuffer_->uvTransform.rotate_, 0.01f);
        DragGuiVectorCommand("Translate##" + _parentLabel, paramBuffer_->uvTransform.translate_, 0.01f);
        ImGui::TreePop();
    }

#endif // _DEBUG
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
