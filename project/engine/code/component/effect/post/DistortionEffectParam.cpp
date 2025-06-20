#include "DistortionEffectParam.h"

/// engine
#include "directX12/DxDevice.h"
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

void DistortionEffectParam::Initialize(GameEntity* _hostEntity) {
    effectParamData_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
    effectParamData_.ConvertToBuffer();

    for (auto& [object, type] : distortionObjects_) {
        object->Initialize(_hostEntity);
    }
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
            ImGui::OpenPopup("AddObject");
        }

        if (ImGui::BeginPopup("AddObject")) {
            static PrimitiveType newObjectType;

            if (ImGui::BeginCombo("PrimitiveType", PrimitiveTypeToString(newObjectType))) {
                for (int32_t i = 0; i < int32_t(PrimitiveType::Count); ++i) {
                    PrimitiveType selectType = (PrimitiveType)i;
                    bool isSelected          = newObjectType == selectType;

                    if (ImGui::Selectable(PrimitiveTypeToString(selectType), isSelected)) {
                        EditorController::getInstance()->pushCommand(
                            std::make_unique<SetterCommand<PrimitiveType>>(&newObjectType, selectType));
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            if (ImGui::Button("Add")) {
                std::shared_ptr<PrimitiveMeshRendererBase> newObject;
                switch (newObjectType) {
                case PrimitiveType::Plane:
                    newObject = std::make_shared<PlaneRenderer>();
                    break;
                case PrimitiveType::Ring:
                    newObject = std::make_shared<RingRenderer>();
                    break;
                default:
                    LOG_ERROR("Unsupported Primitive Type for Distortion Object: {}", std::to_string(int32_t(newObjectType)));
                    break;
                }
                if (newObject) {
                    newObject->Initialize(nullptr);

                    auto command = std::make_unique<AddElementCommand<std::vector<std::pair<std::shared_ptr<PrimitiveMeshRendererBase>, PrimitiveType>>>>(
                        &distortionObjects_, std::make_pair(newObject, newObjectType));
                    EditorController::getInstance()->pushCommand(std::move(command));
                    isChanged = true;
                }
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
        for (auto& [obj, type] : distortionObjects_) {
            objectNodeName = "Distortion Object_" + std::string(PrimitiveTypeToString(type)) + std::to_string(objectIndex);
            if (ImGui::Button(std::string("X##" + objectNodeName).c_str())) {
                auto command = std::make_unique<EraseElementCommand<std::vector<std::pair<std::shared_ptr<PrimitiveMeshRendererBase>, PrimitiveType>>>>(&distortionObjects_, distortionObjects_.begin() + objectIndex);
                EditorController::getInstance()->pushCommand(std::move(command));
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
    for (auto& [obj, type] : distortionObjects_) {
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
    for (const auto& [obj, type] : param.distortionObjects_) {
        nlohmann::json objectData = nlohmann::json::object();
        objectData["objectType"]  = static_cast<int32_t>(type);
        if (obj) {
            // 型ごとに分岐してシリアライズ
            switch (type) {
            case PrimitiveType::Plane:
                objectData["objectData"] = *std::static_pointer_cast<PlaneRenderer>(obj);
                break;
            case PrimitiveType::Ring:
                objectData["objectData"] = *std::static_pointer_cast<RingRenderer>(obj);
                break;
            }
        }
        j["distortionObjects"].push_back(objectData);
    }
}

void from_json(const nlohmann::json& j, DistortionEffectParam& param) {
    j.at("distortionBias").get_to(param.effectParamData_->distortionBias);
    j.at("distortionStrength").get_to(param.effectParamData_->distortionStrength);

    j.at("uvScale").get_to(param.effectParamData_->uvTransform.scale_);
    j.at("uvRotate").get_to(param.effectParamData_->uvTransform.rotate_);
    j.at("uvTranslate").get_to(param.effectParamData_->uvTransform.translate_);

    for (auto& obj : j["distortionObjects"]) {
        nlohmann::json objectData;
        PrimitiveType objectType;
        objectType = PrimitiveType(obj["objectType"]);

        if (objectType == PrimitiveType::Plane) {
            auto newObject = std::make_shared<PlaneRenderer>();
            from_json(obj["objectData"], *newObject);
            param.distortionObjects_.emplace_back(newObject, objectType);
        } else if (objectType == PrimitiveType::Ring) {
            auto newObject = std::make_shared<RingRenderer>();
            from_json(obj["objectData"], *newObject);
            param.distortionObjects_.emplace_back(newObject, objectType);
        }
    }
}
