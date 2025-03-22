#include "Material.h"

#include "globalVariables/GlobalVariables.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#endif // _DEBUG

#include "Engine.h"

const char* isLightUse[] = {
    "False",
    "True",
};

void Material::UpdateUvMatrix() {
    uvMat_ = MakeMatrix::Affine(uvScale_, uvRotate_, uvTranslate_);
}

IConstantBuffer<Material>* MaterialManager::Create(const std::string& materialName) {
    if (materialPallet_.count(materialName) == 0) {
        materialPallet_[materialName] = std::make_unique<IConstantBuffer<Material>>(IConstantBuffer<Material>(materialName));
        materialPallet_[materialName]->CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
        materialPallet_[materialName]->ConvertToBuffer();
    }
    return materialPallet_[materialName].get();
}

IConstantBuffer<Material>* MaterialManager::Create(const std::string& materialName, const Material& data) {
    materialPallet_[materialName]            = std::make_unique<IConstantBuffer<Material>>(IConstantBuffer<Material>(materialName));
    materialPallet_[materialName]->openData_ = data;
    materialPallet_[materialName]->CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
    materialPallet_[materialName]->ConvertToBuffer();
    return materialPallet_[materialName].get();
}

void MaterialManager::Finalize() {
    for (auto& material : materialPallet_) {
        material.second->Finalize();
    }
    materialPallet_.clear();
}

IConstantBuffer<Material>* MaterialManager::getMaterial(const std::string& name) {
    auto itr = materialPallet_.find(name);
    if (itr == materialPallet_.end()) {
        return nullptr;
    }
    return itr->second.get();
}

void MaterialManager::DeleteMaterial(const std::string& materialName) {
    materialPallet_[materialName].reset();
    materialPallet_[materialName] = nullptr;

    std::erase_if(materialPallet_, [](const auto& pair) {
        return pair.second == nullptr;
    });
}

void MaterialEditor::Update() {
#ifdef _DEBUG

    if (ImGui::Begin("MaterialManager", nullptr, ImGuiWindowFlags_MenuBar)) {
        MenuUpdate();

        // メニューバーの外でポップアップを開く
        if (openCreateNewPopup_) {
            ImGui::OpenPopup("Create_New_Material");
            openCreateNewPopup_ = false;
        }

        // ポップアップウィンドウ
        if (ImGui::BeginPopup("Create_New_Material")) {
            // ファイル名の入力
            static char fileName[256] = "";
            ImGui::InputText("MaterialName", fileName, 256);
            // 作成ボタン
            if (ImGui::Button("Create")) {
                materialManager_->Create(fileName);
                ImGui::CloseCurrentPopup();
            }
            // キャンセルボタン
            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        for (auto& material : materialManager_->materialPallet_) {
            if (ImGui::TreeNode(material.first.c_str())) {
                ImGui::ColorEdit4(
                    std::string(material.first + "Color").c_str(),
                    reinterpret_cast<float*>(material.second->openData_.color_.operator Vec4f*()));

                ImGui::Checkbox(
                    (material.first + " enableLighting").c_str(),
                    (bool*)(&material.second->openData_.enableLighting_));

                ImGui::DragFloat3(
                    (material.first + " uvScale").c_str(),
                    reinterpret_cast<float*>(material.second->openData_.uvScale_.operator Vec3f*()),
                    0.1f);
                ImGui::DragFloat3(
                    (material.first + " uvRotate").c_str(),
                    reinterpret_cast<float*>(material.second->openData_.uvRotate_.operator Vec3f*()),
                    0.1f);
                ImGui::DragFloat3(
                    (material.first + " uvTranslate").c_str(),
                    reinterpret_cast<float*>(material.second->openData_.uvTranslate_.operator Vec3f*()),
                    0.1f);

                ImGui::DragFloat(
                    (material.first + " Shininess").c_str(),
                    material.second->openData_.shininess_,
                    0.01f,
                    0.0f,
                    FLT_MAX);
                ImGui::ColorEdit3(
                    (material.first + " SpecularColor").c_str(),
                    reinterpret_cast<float*>(material.second->openData_.specularColor_.operator Vec3f*()));

                material.second->ConvertToBuffer();
                ImGui::TreePop();
            }
        }
    }
    ImGui::End();
#endif // _DEBUG
}

void MaterialEditor::MenuUpdate() {
#ifdef _DEBUG
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("SaveAll")) {
                GlobalVariables::getInstance()->SaveScene("Materials");
            }
            if (ImGui::MenuItem("CreateNew")) {
                openCreateNewPopup_ = true;
            }
            ImGui::EndMenu();
        }
    }
    ImGui::EndMenuBar();
#endif // _DEBUG
}
