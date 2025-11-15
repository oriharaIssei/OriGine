#include "DistortionEffectParam.h"

/// engine
#include "directX12/DxDevice.h"
#include "Engine.h"
#include "scene/Scene.h"
#include "texture/TextureManager.h"
// component
#include "component/renderer/primitive/base/PrimitiveMeshFactory.h"
#include "component/renderer/primitive/base/PrimitiveMeshRendererBase.h"
#include "component/renderer/primitive/base/PrimitiveType.h"
#include "component/renderer/primitive/BoxRenderer.h"
#include "component/renderer/primitive/CylinderRenderer.h"
#include "component/renderer/primitive/PlaneRenderer.h"
#include "component/renderer/primitive/RingRenderer.h"
#include "component/renderer/primitive/SphereRenderer.h"

#include "myFileSystem/MyFileSystem.h"

#ifdef _DEBUG
#include "myGui/MyGui.h"
#endif // _DEBUG

void DistortionEffectParam::Initialize(Entity* _hostEntity) {
    effectParamData_.CreateBuffer(Engine::getInstance()->getDxDevice()->device_);
    effectParamData_.ConvertToBuffer();
    materialBuffer_.CreateBuffer(Engine::getInstance()->getDxDevice()->device_);
    materialBuffer_.ConvertToBuffer(ColorAndUvTransform());

    if (use3dObjectList_) {
        for (auto& [object, type] : distortionObjects_) {
            object->Initialize(_hostEntity);
        }
    } else {
        if (!texturePath_.empty()) {
            textureIndex_ = TextureManager::LoadTexture(texturePath_);
        }
    }
}

void DistortionEffectParam::LoadTexture(const std::string& _path) {
    if (_path.empty()) {
        return;
    }
    texturePath_  = _path;
    textureIndex_ = TextureManager::LoadTexture(texturePath_);
}

void DistortionEffectParam::Edit([[maybe_unused]] Scene* _scene, [[maybe_unused]] Entity* _entity, [[maybe_unused]] const std::string& _parentLabel) {

#ifdef DEBUG
    CheckBoxCommand("Active##" + _parentLabel, isActive_);
    CheckBoxCommand("Use 3D Object##" + _parentLabel, use3dObjectList_);

    ImGui::Spacing();

    std::string label          = "MaterialIndex##" + _parentLabel;
    auto materials             = _scene->getComponents<Material>(_entity);
    int32_t entityMaterialSize = materials != nullptr ? static_cast<int32_t>(materials->size()) : 0;

    if (entityMaterialSize <= 0) {
        ImGui::InputInt(label.c_str(), &materialIndex_, 0, 0, ImGuiInputTextFlags_ReadOnly);
    } else {
        InputGuiCommand(label, materialIndex_);
        materialIndex_ = std::clamp(materialIndex_, 0, entityMaterialSize - 1);
    }

    ImGui::Spacing();

    DragGuiVectorCommand("Distortion Bias##" + _parentLabel, effectParamData_->distortionBias, 0.01f);
    DragGuiVectorCommand("Distortion Strength##" + _parentLabel, effectParamData_->distortionStrength, 0.01f);

    ImGui::Separator();

    if (use3dObjectList_) {
        ImGui::Spacing();

        // TODO : 3DObjectのを別のコンポーネントにする
        std::string objectNodeName;
        int32_t objectIndex     = 0;
        std::string objectLabel = "Distortion Object##" + _parentLabel;
        if (ImGui::TreeNode(objectLabel.c_str())) {
            objectLabel = "Add Object##" + _parentLabel;
            if (ImGui::Button(objectLabel.c_str())) {
                ImGui::OpenPopup(objectLabel.c_str());
            }

            if (ImGui::BeginPopup(objectLabel.c_str())) {
                static PrimitiveType newObjectType;

                objectLabel = "PrimitiveType##" + _parentLabel;
                if (ImGui::BeginCombo(objectLabel.c_str(), std::to_string(newObjectType).c_str())) {
                    for (int32_t i = 0; i < int32_t(PrimitiveType::Count); ++i) {
                        PrimitiveType selectType = (PrimitiveType)i;
                        bool isSelected          = newObjectType == selectType;

                        if (ImGui::Selectable(std::to_string(selectType).c_str(), isSelected)) {
                            EditorController::getInstance()->pushCommand(
                                std::make_unique<SetterCommand<PrimitiveType>>(&newObjectType, selectType));
                        }
                        if (isSelected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

                objectLabel = "Add##" + _parentLabel;
                if (ImGui::Button(objectLabel.c_str())) {
                    std::shared_ptr<PrimitiveMeshRendererBase> newObject = PrimitiveMeshFactory::getInstance()->CreatePrimitiveMeshBy(newObjectType);

                    if (newObject) {
                        newObject->Initialize(_entity);

                        auto command = std::make_unique<AddElementCommand<std::vector<std::pair<std::shared_ptr<PrimitiveMeshRendererBase>, PrimitiveType>>>>(
                            &distortionObjects_, std::make_pair(newObject, newObjectType));
                        EditorController::getInstance()->pushCommand(std::move(command));
                    }
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }
            for (auto& [obj, type] : distortionObjects_) {
                objectNodeName = "Distortion Object_" + std::to_string(type) + std::to_string(objectIndex);
                if (ImGui::Button(std::string("X##" + objectNodeName).c_str())) {
                    auto command = std::make_unique<EraseElementCommand<std::vector<std::pair<std::shared_ptr<PrimitiveMeshRendererBase>, PrimitiveType>>>>(&distortionObjects_, distortionObjects_.begin() + objectIndex);
                    EditorController::getInstance()->pushCommand(std::move(command));
                    continue;
                }
                ImGui::SameLine();
                if (ImGui::TreeNode(objectNodeName.c_str())) {
                    if (obj) {
                        obj->Edit(_scene, _entity, _parentLabel + objectNodeName);
                    }
                    ImGui::TreePop();
                }
                ++objectIndex;
            }

            ImGui::TreePop();
        }
    } else {
        // object not use
        if (!distortionObjects_.empty()) {
            distortionObjects_.clear();
        }

        auto askLoadTexture = [this]([[maybe_unused]] const std::string& _parentLabel) {
            bool ask          = false;
            std::string label = "Load Texture##" + _parentLabel;
            ask               = ImGui::Button(label.c_str());
            ask |= ImGui::ImageButton(
                ImTextureID(TextureManager::getDescriptorGpuHandle(textureIndex_).ptr),
                ImVec2(32, 32), ImVec2(0, 0), ImVec2(1, 1), 4, ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1));

            return ask;
        };
        ImGui::Text("Texture Directory: %s", texturePath_.c_str());
        if (askLoadTexture(_parentLabel)) {
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
        };
    }

#endif // DEBUG
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

    j["isActive"]        = param.isActive_;
    j["use3dObjectList"] = param.use3dObjectList_;
    j["materialIndex"]   = param.materialIndex_;

    if (param.use3dObjectList_) {
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
                case PrimitiveType::Box:
                    objectData["objectData"] = *std::static_pointer_cast<BoxRenderer>(obj);
                    break;
                case PrimitiveType::Sphere:
                    objectData["objectData"] = *std::static_pointer_cast<SphereRenderer>(obj);
                    break;
                case PrimitiveType::Cylinder:
                    objectData["objectData"] = *std::static_pointer_cast<CylinderRenderer>(obj);
                    break;

                default:
                    LOG_ERROR("Unsupported Primitive Type for Distortion Object: {}", std::to_string(int32_t(type)));
                    break;
                }
            }
            j["distortionObjects"].push_back(objectData);
        }
    } else {
        j["textuerPath"] = param.texturePath_;
    }
}

void from_json(const nlohmann::json& j, DistortionEffectParam& param) {
    param.effectParamData_->distortionBias     = j.value("distortionBias", Vec2f());
    param.effectParamData_->distortionStrength = j.value("distortionStrength", Vec2f());

    if (j.contains("materialIndex")) {
        j.at("materialIndex").get_to(param.materialIndex_);
    }
    if (j.contains("isActive")) {
        j.at("isActive").get_to(param.isActive_);
    }

    if (j.contains("use3dObjectList")) {
        j.at("use3dObjectList").get_to(param.use3dObjectList_);
    } else {
        param.use3dObjectList_ = true; // 以前のデータとの互換性のため、use3dObjectList_が無い場合はtrueにする
    }

    if (param.use3dObjectList_) {
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
    } else {
        if (j.contains("textuerPath")) {
            j.at("textuerPath").get_to(param.texturePath_);
        }
    }
}
