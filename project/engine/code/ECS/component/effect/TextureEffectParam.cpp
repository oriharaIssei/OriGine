#include "TextureEffectParam.h"

/// engine
#define RESOURCE_DIRECTORY
#define ENGINE_INCLUDE
#include "directX12/DxDevice.h"
#include "EngineInclude.h"
#include "texture/TextureManager.h"

#ifdef _DEBUG
#include "myFileSystem/MyFileSystem.h"
#include "myGui/MyGui.h"
#include "util/timeline/Timeline.h"
#endif // _DEBUG

void to_json(nlohmann::json& j, const TextureEffectParam& param) {
    // effectFlag
    j["effectFlag"] = param.effectParamData_->effectFlag;

    // texturePath
    j["dissolveTexPath"]   = param.dissolveTexPath_;
    j["distortionTexPath"] = param.distortionTexPath_;
    j["maskTexPath"]       = param.maskTexPath_;

    // uniqueParam
    // dissolve
    j["dissolveColor"]     = param.effectParamData_->dissolveColor;
    j["dissolveEdgeWidth"] = param.effectParamData_->dissolveEdgeWidth;
    j["dissolveThreshold"] = param.effectParamData_->dissolveThreshold;
    // distortion
    j["distortionStrength"] = param.effectParamData_->distortionStrength;
    j["distortionBias"]     = param.effectParamData_->distortionBias;

    /// ===============================================================
    // uv
    /// ===============================================================
    // dissolveUV
    nlohmann::json dissolveUV = nlohmann::json::object();
    dissolveUV["scale"]       = param.effectParamData_->dissolveUV.scale_;
    dissolveUV["rotate"]      = param.effectParamData_->dissolveUV.rotate_;
    dissolveUV["translate"]   = param.effectParamData_->dissolveUV.translate_;
    j["dissolveUV"]           = dissolveUV;
    // distortionUV
    nlohmann::json distortionUV = nlohmann::json::object();
    distortionUV["scale"]       = param.effectParamData_->distortionUV.scale_;
    distortionUV["rotate"]      = param.effectParamData_->distortionUV.rotate_;
    distortionUV["translate"]   = param.effectParamData_->distortionUV.translate_;
    j["distortionUV"]           = distortionUV;
    // maskUV
    nlohmann::json maskUV = nlohmann::json::object();
    maskUV["scale"]       = param.effectParamData_->maskUV.scale_;
    maskUV["rotate"]      = param.effectParamData_->maskUV.rotate_;
    maskUV["translate"]   = param.effectParamData_->maskUV.translate_;
    j["maskUV"]           = maskUV;

    /// ===============================================================
    // animation
    /// ===============================================================
    // dissolveAnim
    j["dissolveAnimDuration"]        = param.dissolveAnim_.duration;
    j["dissolveAnimIsPlay"]          = param.dissolveAnimState_.isPlay_;
    j["dissolveAnimIsLoop"]          = param.dissolveAnimState_.isLoop_;
    nlohmann::json dissolveAnimScale = nlohmann::json::array();
    for (auto& scaleKey : param.dissolveAnim_.scale) {
        dissolveAnimScale.push_back({{"time", scaleKey.time}, {"value", scaleKey.value}});
    }
    nlohmann::json dissolveAnimRotate = nlohmann::json::array();
    for (auto& rotateKey : param.dissolveAnim_.rotate) {
        dissolveAnimRotate.push_back({{"time", rotateKey.time}, {"value", rotateKey.value}});
    }
    nlohmann::json dissolveAnimTranslate = nlohmann::json::array();
    for (auto& translateKey : param.dissolveAnim_.translate) {
        dissolveAnimTranslate.push_back({{"time", translateKey.time}, {"value", translateKey.value}});
    }
    j["dissolveAnimScale"]     = dissolveAnimScale;
    j["dissolveAnimRotate"]    = dissolveAnimRotate;
    j["dissolveAnimTranslate"] = dissolveAnimTranslate;

    // distortionAnim
    j["distortionStrength"]            = param.effectParamData_->distortionStrength;
    j["distortionAnimDuration"]        = param.distortionAnim_.duration;
    j["distortionAnimIsPlay"]          = param.distortionAnimState_.isPlay_;
    j["distortionAnimIsLoop"]          = param.distortionAnimState_.isLoop_;
    nlohmann::json distortionAnimScale = nlohmann::json::array();
    for (auto& scaleKey : param.distortionAnim_.scale) {
        distortionAnimScale.push_back({{"time", scaleKey.time}, {"value", scaleKey.value}});
    }
    nlohmann::json distortionAnimRotate = nlohmann::json::array();
    for (auto& rotateKey : param.distortionAnim_.rotate) {
        distortionAnimRotate.push_back({{"time", rotateKey.time}, {"value", rotateKey.value}});
    }
    nlohmann::json distortionAnimTranslate = nlohmann::json::array();
    for (auto& translateKey : param.distortionAnim_.translate) {
        distortionAnimTranslate.push_back({{"time", translateKey.time}, {"value", translateKey.value}});
    }
    j["distortionAnimScale"]     = distortionAnimScale;
    j["distortionAnimRotate"]    = distortionAnimRotate;
    j["distortionAnimTranslate"] = distortionAnimTranslate;

    // maskAnim
    j["maskAnimDuration"]        = param.maskAnim_.duration;
    j["maskAnimIsPlay"]          = param.maskAnimState_.isPlay_;
    j["maskAnimIsLoop"]          = param.maskAnimState_.isLoop_;
    nlohmann::json maskAnimScale = nlohmann::json::array();
    for (auto& scaleKey : param.maskAnim_.scale) {
        maskAnimScale.push_back({{"time", scaleKey.time}, {"value", scaleKey.value}});
    }
    nlohmann::json maskAnimRotate = nlohmann::json::array();
    for (auto& rotateKey : param.maskAnim_.rotate) {
        maskAnimRotate.push_back({{"time", rotateKey.time}, {"value", rotateKey.value}});
    }
    nlohmann::json maskAnimTranslate = nlohmann::json::array();
    for (auto& translateKey : param.maskAnim_.translate) {
        maskAnimTranslate.push_back({{"time", translateKey.time}, {"value", translateKey.value}});
    }
    j["maskAnimScale"]     = maskAnimScale;
    j["maskAnimRotate"]    = maskAnimRotate;
    j["maskAnimTranslate"] = maskAnimTranslate;
}
void from_json(const nlohmann::json& j, TextureEffectParam& param) {
    // effectFlag
    j.at("effectFlag").get_to(param.effectParamData_->effectFlag);

    // texturePath
    j.at("dissolveTexPath").get_to(param.dissolveTexPath_);
    j.at("distortionTexPath").get_to(param.distortionTexPath_);
    j.at("maskTexPath").get_to(param.maskTexPath_);

    /// ===============================================================
    // uv
    /// ===============================================================
    // dissolveUV
    j.at("dissolveUV").at("scale").get_to(param.effectParamData_->dissolveUV.scale_);
    j.at("dissolveUV").at("rotate").get_to(param.effectParamData_->dissolveUV.rotate_);
    j.at("dissolveUV").at("translate").get_to(param.effectParamData_->dissolveUV.translate_);
    // distortionUV
    j.at("distortionUV").at("scale").get_to(param.effectParamData_->distortionUV.scale_);
    j.at("distortionUV").at("rotate").get_to(param.effectParamData_->distortionUV.rotate_);
    j.at("distortionUV").at("translate").get_to(param.effectParamData_->distortionUV.translate_);
    // maskUV
    j.at("maskUV").at("scale").get_to(param.effectParamData_->maskUV.scale_);
    j.at("maskUV").at("rotate").get_to(param.effectParamData_->maskUV.rotate_);
    j.at("maskUV").at("translate").get_to(param.effectParamData_->maskUV.translate_);

    // effectParamData
    if (j.find("dissolveColor") != j.end()) {
        j.at("dissolveColor").get_to(param.effectParamData_->dissolveColor);
    }
    // effectParamData
    if (j.find("dissolveEdgeWidth") != j.end()) {
        j.at("dissolveEdgeWidth").get_to(param.effectParamData_->dissolveEdgeWidth);
    }
    j.at("dissolveThreshold").get_to(param.effectParamData_->dissolveThreshold);
    j.at("distortionStrength").get_to(param.effectParamData_->distortionStrength);
    j.at("distortionBias").get_to(param.effectParamData_->distortionBias);

    /// ===============================================================
    // animation
    /// ===============================================================
    // dissolveAnim
    j.at("dissolveAnimDuration").get_to(param.dissolveAnim_.duration);
    j.at("dissolveAnimIsPlay").get_to(param.dissolveAnimState_.isPlay_);
    j.at("dissolveAnimIsLoop").get_to(param.dissolveAnimState_.isLoop_);
    for (auto& scaleKey : j.at("dissolveAnimScale")) {
        param.dissolveAnim_.scale.push_back({scaleKey.at("time"), scaleKey.at("value")});
    }
    for (auto& rotateKey : j.at("dissolveAnimRotate")) {
        param.dissolveAnim_.rotate.push_back({rotateKey.at("time"), rotateKey.at("value")});
    }
    for (auto& translateKey : j.at("dissolveAnimTranslate")) {
        param.dissolveAnim_.translate.push_back({translateKey.at("time"), translateKey.at("value")});
    }

    // distortionAnim
    j.at("distortionAnimDuration").get_to(param.distortionAnim_.duration);
    j.at("distortionAnimIsPlay").get_to(param.distortionAnimState_.isPlay_);
    j.at("distortionAnimIsLoop").get_to(param.distortionAnimState_.isLoop_);
    for (auto& scaleKey : j.at("distortionAnimScale")) {
        param.distortionAnim_.scale.push_back({scaleKey.at("time"), scaleKey.at("value")});
    }
    for (auto& rotateKey : j.at("distortionAnimRotate")) {
        param.distortionAnim_.rotate.push_back({rotateKey.at("time"), rotateKey.at("value")});
    }
    for (auto& translateKey : j.at("distortionAnimTranslate")) {
        param.distortionAnim_.translate.push_back({translateKey.at("time"), translateKey.at("value")});
    }

    // maskAnim
    j.at("maskAnimDuration").get_to(param.maskAnim_.duration);
    j.at("maskAnimIsPlay").get_to(param.maskAnimState_.isPlay_);
    j.at("maskAnimIsLoop").get_to(param.maskAnimState_.isLoop_);
    for (auto& scaleKey : j.at("maskAnimScale")) {
        param.maskAnim_.scale.push_back({scaleKey.at("time"), scaleKey.at("value")});
    }
    for (auto& rotateKey : j.at("maskAnimRotate")) {
        param.maskAnim_.rotate.push_back({rotateKey.at("time"), rotateKey.at("value")});
    }
    for (auto& translateKey : j.at("maskAnimTranslate")) {
        param.maskAnim_.translate.push_back({translateKey.at("time"), translateKey.at("value")});
    }
}

void TextureEffectParam::Initialize(GameEntity* /*_hostEntity*/) {
    ///=========================================================
    // Load Texture
    ///=========================================================
    if (!dissolveTexPath_.empty()) {
        dissolveTexIndex_ = TextureManager::LoadTexture(dissolveTexPath_);
    }
    if (!distortionTexPath_.empty()) {
        distortionTexIndex_ = TextureManager::LoadTexture(distortionTexPath_);
    }
    if (!maskTexPath_.empty()) {
        maskTexIndex_ = TextureManager::LoadTexture(maskTexPath_);
    }

    ///=========================================================
    // Create Buffer
    ///=========================================================
    effectParamData_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
}

void TextureEffectParam::Edit(Scene* /*_scene*/, GameEntity* /*_entity*/, const std::string& _parentLabel) {
#ifdef _DEBUG

    ///===========================================
    // Effect Flags
    ///===========================================
    bool isDissolve   = (effectParamData_->effectFlag & (int32_t)TextureEffectParamData::EffectFlag::Dissolve);
    std::string label = "is Dissolve##" + _parentLabel;
    if (ImGui::Checkbox(label.c_str(), &isDissolve)) {
        int32_t newFlag;
        if (isDissolve) {
            newFlag = effectParamData_->effectFlag | (int32_t)TextureEffectParamData::EffectFlag::Dissolve;
        } else {
            newFlag = effectParamData_->effectFlag & ~(int32_t)TextureEffectParamData::EffectFlag::Dissolve;
        }
        auto command = std::make_unique<SetterCommand<int32_t>>(&effectParamData_->effectFlag, newFlag);
        EditorController::getInstance()->pushCommand(std::move(command));
    }
    bool isDistortion = (effectParamData_->effectFlag & (int32_t)TextureEffectParamData::EffectFlag::Distortion);
    label             = "is Distortion##" + _parentLabel;
    if (ImGui::Checkbox(label.c_str(), &isDistortion)) {
        int32_t newFlag;
        if (isDistortion) {
            newFlag = effectParamData_->effectFlag | (int32_t)TextureEffectParamData::EffectFlag::Distortion;
        } else {
            newFlag = effectParamData_->effectFlag & ~(int32_t)TextureEffectParamData::EffectFlag::Distortion;
        }
        auto command = std::make_unique<SetterCommand<int32_t>>(&effectParamData_->effectFlag, newFlag);
        EditorController::getInstance()->pushCommand(std::move(command));
    }
    bool isMask = (effectParamData_->effectFlag & (int32_t)TextureEffectParamData::EffectFlag::Mask);
    label       = "is Mask##" + _parentLabel;
    if (ImGui::Checkbox(label.c_str(), &isMask)) {
        int32_t newFlag;
        if (isMask) {
            newFlag = effectParamData_->effectFlag | (int32_t)TextureEffectParamData::EffectFlag::Mask;
        } else {
            newFlag = effectParamData_->effectFlag & ~(int32_t)TextureEffectParamData::EffectFlag::Mask;
        }
        auto command = std::make_unique<SetterCommand<int32_t>>(&effectParamData_->effectFlag, newFlag);
        EditorController::getInstance()->pushCommand(std::move(command));
    }

    ///===========================================
    // Dissolve
    ///===========================================
    if (isDissolve) {
        if (ImGui::TreeNode("Dissolve")) {
            label = "Play##Dissolve" + _parentLabel;
            CheckBoxCommand(label, dissolveAnimState_.isPlay_);
            label = "Loop##Dissolve" + _parentLabel;
            CheckBoxCommand(label, dissolveAnimState_.isLoop_);

            label = "Dissolve Threshold##" + _parentLabel;
            DragGuiCommand(label, effectParamData_->dissolveThreshold, 0.01f);
            label = "Dissolve Edge Width##" + _parentLabel;
            DragGuiCommand(label, effectParamData_->dissolveEdgeWidth, 0.001f, 0.0f, {}, "%.4f");
            label = "Dissolve Color##" + _parentLabel;
            ColorEditGuiCommand<4>(label, effectParamData_->dissolveColor);

            ImGui::Separator();

            ImGui::Text("Texture Path : %s", dissolveTexPath_.c_str());
            label = "TextureLoad##Dissolve" + _parentLabel;
            if (ImGui::Button(label.c_str())) {
                std::string directory, filename;
                if (MyFileSystem::selectFileDialog(
                        kApplicationResourceDirectory,
                        directory,
                        filename,
                        {"png"})) {
                    auto commandCombo = std::make_unique<CommandCombo>();
                    commandCombo->addCommand(std::make_shared<SetterCommand<std::string>>(&dissolveTexPath_, kApplicationResourceDirectory + "/" + directory + "/" + filename));
                    commandCombo->setFuncOnAfterCommand([this]() {
                        dissolveTexIndex_ = TextureManager::LoadTexture(dissolveTexPath_);
                    },
                        true);

                    EditorController::getInstance()->pushCommand(std::move(commandCombo));
                }
            }

            label = "DissolveDuration##" + _parentLabel;
            DragGuiCommand(label.c_str(), dissolveAnim_.duration, 0.01f, 0.0f);

            ImGui::Text("UV Scale");
            label = "DissolveUVScale##" + _parentLabel;
            DragGuiVectorCommand<2, float>(
                label,
                effectParamData_->dissolveUV.scale_,
                0.01f);
            ImGui::EditKeyFrame(label, dissolveAnim_.scale, dissolveAnim_.duration);
            if (!dissolveAnim_.scale.empty()) {
                dissolveAnim_.scale.front().value = effectParamData_->dissolveUV.scale_;
            } else {
                dissolveAnim_.scale.push_back({0.f, effectParamData_->dissolveUV.scale_});
            }

            ImGui::Text("UV Rotate");
            label = "DissolveUVRotate##" + _parentLabel;
            DragGuiCommand<float>(label, effectParamData_->dissolveUV.rotate_, 0.01f);
            ImGui::EditKeyFrame(label, dissolveAnim_.rotate, dissolveAnim_.duration);
            if (!dissolveAnim_.rotate.empty()) {
                dissolveAnim_.rotate.front().value = effectParamData_->dissolveUV.rotate_;
            } else {
                dissolveAnim_.rotate.push_back({0.f, effectParamData_->dissolveUV.rotate_});
            }

            ImGui::Text("UV Translate");
            label = "DissolveUVTranslate##" + _parentLabel;
            DragGuiVectorCommand<2, float>(label, effectParamData_->dissolveUV.translate_, 0.01f);
            ImGui::EditKeyFrame(label, dissolveAnim_.translate, dissolveAnim_.duration);
            if (!dissolveAnim_.translate.empty()) {
                dissolveAnim_.translate.front().value = effectParamData_->dissolveUV.translate_;
            } else {
                dissolveAnim_.translate.push_back({0.f, effectParamData_->dissolveUV.translate_});
            }

            ImGui::TreePop();
        }
    }

    ///===========================================
    // Distortion
    ///===========================================
    if (isDistortion) {
        label = "Distortion##" + _parentLabel;
        if (ImGui::TreeNode(label.c_str())) {
            label = "Play##Distortion" + _parentLabel;
            CheckBoxCommand(label, distortionAnimState_.isPlay_);
            label = "Loop##Distortion" + _parentLabel;
            CheckBoxCommand(label, distortionAnimState_.isLoop_);

            label = "DistortionStrength##" + _parentLabel;
            DragGuiCommand(label, effectParamData_->distortionStrength, 0.01f);
            label = "DistortionBias##" + _parentLabel;
            SlideGuiCommand<float>(label, effectParamData_->distortionBias, 0.0f, 1.f);

            ImGui::Separator();

            ImGui::Text("TexturePath :  %s", distortionTexPath_.c_str());
            label = "TextureLoad##Distortion" + _parentLabel;
            if (ImGui::Button(label.c_str())) {
                std::string directory, filename;
                if (MyFileSystem::selectFileDialog(
                        kApplicationResourceDirectory,
                        directory,
                        filename,
                        {"png"})) {
                    auto commandCombo = std::make_unique<CommandCombo>();
                    commandCombo->addCommand(std::make_shared<SetterCommand<std::string>>(&distortionTexPath_, kApplicationResourceDirectory + "/" + directory + "/" + filename));
                    commandCombo->setFuncOnAfterCommand([this]() {
                        distortionTexIndex_ = TextureManager::LoadTexture(distortionTexPath_);
                    },
                        true);
                    EditorController::getInstance()->pushCommand(std::move(commandCombo));
                }
            }

            label = "DistortionDuration##" + _parentLabel;
            DragGuiCommand(label, distortionAnim_.duration, 0.01f, 0.0f);

            ImGui::Text("UV Scale");
            label = "DistortionUVScale##" + _parentLabel;
            DragGuiVectorCommand<2, float>(label, effectParamData_->distortionUV.scale_, 0.01f);
            ImGui::EditKeyFrame(label, distortionAnim_.scale, distortionAnim_.duration);
            if (!distortionAnim_.scale.empty()) {
                distortionAnim_.scale.front().value = effectParamData_->distortionUV.scale_;
            } else {
                distortionAnim_.scale.push_back({0.f, effectParamData_->distortionUV.scale_});
            }

            ImGui::Text("UV Rotate");
            label = "DistortionUVRotate##" + _parentLabel;
            DragGuiCommand<float>(label, effectParamData_->distortionUV.rotate_, 0.01f);
            ImGui::EditKeyFrame(label, distortionAnim_.rotate, distortionAnim_.duration);
            if (!distortionAnim_.rotate.empty()) {
                distortionAnim_.rotate.front().value = effectParamData_->distortionUV.rotate_;
            } else {
                distortionAnim_.rotate.push_back({0.f, effectParamData_->distortionUV.rotate_});
            }

            ImGui::Text("UV Translate");
            label = "DistortionUVTranslate##" + _parentLabel;
            DragGuiVectorCommand<2, float>(label, effectParamData_->distortionUV.translate_, 0.01f);
            ImGui::EditKeyFrame(label, distortionAnim_.translate, distortionAnim_.duration);
            if (!distortionAnim_.translate.empty()) {
                distortionAnim_.translate.front().value = effectParamData_->distortionUV.translate_;
            }

            ImGui::TreePop();
        }
    }

    ///===========================================
    // Mask
    ///===========================================
    if (isMask) {
        label = "Mask##" + _parentLabel;
        if (ImGui::TreeNode(label.c_str())) {
            label = "Play##Mask" + _parentLabel;
            CheckBoxCommand(label, maskAnimState_.isPlay_);
            label = "Loop##Mask" + _parentLabel;
            CheckBoxCommand(label, maskAnimState_.isLoop_);

            ImGui::Separator();

            ImGui::Text("TexturePath : %s", maskTexPath_.c_str());
            label = "TextureLoad##Mask" + _parentLabel;
            if (ImGui::Button(label.c_str())) {
                std::string directory, filename;
                if (MyFileSystem::selectFileDialog(
                        kApplicationResourceDirectory,
                        directory,
                        filename,
                        {"png"})) {

                    auto commandCombo = std::make_unique<CommandCombo>();
                    commandCombo->addCommand(std::make_shared<SetterCommand<std::string>>(&maskTexPath_, kApplicationResourceDirectory + "/" + directory + "/" + filename));
                    commandCombo->setFuncOnAfterCommand([this]() {
                        maskTexIndex_ = TextureManager::LoadTexture(maskTexPath_);
                    },
                        true);
                    EditorController::getInstance()->pushCommand(std::move(commandCombo));
                }
            }

            label = "MaskDuration##" + _parentLabel;
            DragGuiCommand(label, maskAnim_.duration, 0.01f, 0.0f);

            ImGui::Text("UV Scale");
            label = "MaskUVScale##" + _parentLabel;
            DragGuiVectorCommand<2, float>(label, effectParamData_->maskUV.scale_, 0.01f);
            ImGui::EditKeyFrame(label, maskAnim_.scale, maskAnim_.duration);
            if (!maskAnim_.scale.empty()) {
                maskAnim_.scale.front().value = effectParamData_->maskUV.scale_;
            } else {
                maskAnim_.scale.push_back({0.f, effectParamData_->maskUV.scale_});
            }

            ImGui::Text("UV Rotate");
            label = "MaskUVRotate##" + _parentLabel;
            DragGuiCommand<float>(label, effectParamData_->maskUV.rotate_, 0.01f);
            ImGui::EditKeyFrame(label, maskAnim_.rotate, maskAnim_.duration);
            if (!maskAnim_.rotate.empty()) {
                maskAnim_.rotate.front().value = effectParamData_->maskUV.rotate_;
            } else {
                maskAnim_.rotate.push_back({0.f, effectParamData_->maskUV.rotate_});
            }

            ImGui::Text("UV Translate");
            label = "MaskUVTranslate##" + _parentLabel;
            DragGuiVectorCommand<2, float>(label, effectParamData_->maskUV.translate_, 0.01f);
            ImGui::EditKeyFrame(label, maskAnim_.translate, maskAnim_.duration);
            if (!maskAnim_.translate.empty()) {
                maskAnim_.translate.front().value = effectParamData_->maskUV.translate_;
            } else {
                maskAnim_.translate.push_back({0.f, effectParamData_->maskUV.translate_});
            }

            ImGui::TreePop();
        }
    }

    effectParamData_->UpdateTransform();

#endif // _DEBUG
}

void TextureEffectParam::Finalize() {}

void TextureEffectParam::LoadDissolveTexture(const std::string& path) {
    dissolveTexPath_  = path;
    dissolveTexIndex_ = TextureManager::LoadTexture(dissolveTexPath_);
}

void TextureEffectParam::LoadDistortionTexture(const std::string& path) {
    distortionTexPath_  = path;
    distortionTexIndex_ = TextureManager::LoadTexture(distortionTexPath_);
}

void TextureEffectParam::LoadMaskTexture(const std::string& path) {
    maskTexPath_  = path;
    maskTexIndex_ = TextureManager::LoadTexture(maskTexPath_);
}

void TextureEffectParamData::UpdateTransform() {
    dissolveUVMat   = MakeMatrix::Affine({dissolveUV.scale_, 1.f}, {0.f, 0.f, dissolveUV.rotate_}, {dissolveUV.translate_, 0.f});
    distortionUVMat = MakeMatrix::Affine({distortionUV.scale_, 1.f}, {0.f, 0.f, distortionUV.rotate_}, {distortionUV.translate_, 0.f});
    maskUVMat       = MakeMatrix::Affine({maskUV.scale_, 1.f}, {0.f, 0.f, maskUV.rotate_}, {maskUV.translate_, 0.f});
}
