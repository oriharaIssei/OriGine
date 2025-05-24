#include "DistortionEffectParam.h"

/// engine
#include "Engine.h"
#include "texture/TextureManager.h"
// component
#include "component/renderer/primitive/Primitive.h"

/// lib
#include "myFileSystem/MyFileSystem.h"

#ifdef _DEBUG
#include "myGui/MyGui.h"
#endif // _DEBUG

void DistortionParamData::UpdateUVMat() {
    uvMat = MakeMatrix::Affine({uvTransform.scale_, 1.f}, {0.f, 0.f, uvTransform.rotate_}, {uvTransform.translate_, 0.f});
}

void DistortionEffectParam::Initialize(GameEntity* /* _hostEntity*/) {
    effectParamData_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
    effectParamData_.ConvertToBuffer();
}

bool DistortionEffectParam::Edit() {
    bool isChanged = false;
#ifdef DEBUG
    isChanged |= DragGuiVectorCommand("UV Scale", effectParamData_->uvTransform.scale_);
    isChanged |= DragGuiCommand("UV Rotate", effectParamData_->uvTransform.rotate_);
    isChanged |= DragGuiVectorCommand("UV Translate", effectParamData_->uvTransform.translate_);
    if (isChanged) {
        effectParamData_->UpdateUVMat();
        effectParamData_.ConvertToBuffer();
    }

    ImGui::Spacing();

    DragGuiCommand("Distortion Bias", effectParamData_->distortionBias, 0.01f);
    DragGuiCommand("Distortion Strength", effectParamData_->distortionStrength, 0.01f);

    ImGui::Separator();
    ImGui::Spacing();

    std::string objectNodeName;
    int32_t objectIndex = 0;
    if (ImGui::TreeNode("Distortion Object")) {
        if (ImGui::Button("Add Object")) {
            auto newObject = std::make_shared<PlaneRenderer>();
            newObject->Initialize(nullptr);
            auto command = std::make_unique<AddElementCommand<std::vector<std::shared_ptr<PlaneRenderer>>>>(&distortionObjects_, newObject);
            EditorGroup::getInstance()->pushCommand(std::move(command));
            isChanged = true;
        }
        for (auto& obj : distortionObjects_) {
            objectNodeName = "Distortion Object_" + std::to_string(objectIndex);
            if (ImGui::Button(std::string("X##" + objectNodeName).c_str())) {
                auto command = std::make_unique<EraseElementCommand<std::vector<std::shared_ptr<PlaneRenderer>>>>(&distortionObjects_, distortionObjects_.begin() + objectIndex);
                EditorGroup::getInstance()->pushCommand(std::move(command));
                isChanged = true;
                continue;
            }
            ImGui::SameLine();
            if (ImGui::TreeNode(objectNodeName.c_str())) {
                if (obj) {
                    isChanged |= obj->Edit();
                }
                ImGui::TreePop();
            }
            objectIndex++;
        }
        ImGui::TreePop();
    }

#endif // DEBUG
    return isChanged;
}

void DistortionEffectParam::Finalize() {
    for (auto& obj : distortionObjects_) {
        if (obj) {
            obj->Finalize();
        }
    }
    distortionObjects_.clear();
    effectParamData_.Finalize();
}

void to_json(nlohmann::json& j, const DistortionEffectParam& param) {
    j["distortionBias"]     = param.effectParamData_->distortionBias;
    j["distortionStrength"] = param.effectParamData_->distortionStrength;

    j["uvScale"]     = param.effectParamData_->uvTransform.scale_;
    j["uvRotate"]    = param.effectParamData_->uvTransform.rotate_;
    j["uvTranslate"] = param.effectParamData_->uvTransform.translate_;

    j["distortionObjects"] = nlohmann::json::array();
    for (const auto& obj : param.distortionObjects_) {
        nlohmann::json objectData;
        objectData = *obj;
        j["distortionObjects"].push_back(objectData);
    }
}

void from_json(const nlohmann::json& j, DistortionEffectParam& param) {
    j.at("distortionBias").get_to(param.effectParamData_->distortionBias);
    j.at("distortionStrength").get_to(param.effectParamData_->distortionStrength);

    j.at("uvScale").get_to(param.effectParamData_->uvTransform.scale_);
    j.at("uvRotate").get_to(param.effectParamData_->uvTransform.rotate_);
    j.at("uvTranslate").get_to(param.effectParamData_->uvTransform.translate_);

    for (auto& obj : param.distortionObjects_) {
        nlohmann::json objectData;
        j.at("distortionObjects").get_to(objectData);
        obj = std::make_shared<PlaneRenderer>();
        *obj = objectData;
    }
}
