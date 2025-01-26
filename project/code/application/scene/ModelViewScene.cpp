#include "ModelViewScene.h"

///local
//camera
#include "camera/CameraManager.h"

//lib
#include "material/Material.h"
#include "material/texture/TextureManager.h"
#include "myFileSystem/MyFileSystem.h"
#include "primitiveDrawer/PrimitiveDrawer.h"

/// externals
#include "imgui/imgui.h"

std::list<std::pair<std::string, std::string>> ModelViewScene::modelFileList_;
std::list<std::pair<std::string, std::string>> ModelViewScene::textureFileList_ = myfs::SearchFile("resource/Texture", "png", false);
ModelViewScene::ModelViewScene()
    : IScene("ModelViewScene") {}

ModelViewScene::~ModelViewScene() {}

void ModelViewScene::Init() {
    //===================== DebugCamera =====================//
    debugCamera_ = std::make_unique<DebugCamera>();
    debugCamera_->Init();
    debugCamera_->setViewRotate(Vector3(0.3f, 3.14f, 0.0f));
    debugCamera_->setViewTranslate(Vector3(0.0f, 2.0f, 10.0f));

    //===================== EditObject =====================//
    editObject_ = std::make_unique<Object3d>();

    //===================== Ground =====================//
    ground_ = std::make_unique<Object3d>();
    ground_->Init("resource/Models/CheckBoard", "CheckBoard.obj");
    ground_->transform_.translate.v[Y] = -1.0f;
    ground_->transform_.UpdateMatrix();

    //===================== ModelFileList =====================//
    modelFileList_ = myfs::SearchFile("resource/Models", "gltf", false);
    modelFileList_.splice(modelFileList_.begin(), myfs::SearchFile("resource/Models", "obj", false));
}

void ModelViewScene::Update() {
    ///================================================================
    /// Camera
    ///================================================================
    debugCamera_->Update();
    debugCamera_->DebugUpdate();
    CameraManager::getInstance()->setTransform(debugCamera_->getCameraTransform());

    if (ImGui::Begin("ModelEdit", nullptr, ImGuiWindowFlags_MenuBar)) {
        ///================================================================
        /// メニューバー
        ///================================================================
        if (ImGui::BeginMenuBar()) {
            //======================= File =======================//
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("ReloadFileList")) {
                    modelFileList_ = myfs::SearchFile("resource/Models", "gltf", false);
                    modelFileList_.splice(modelFileList_.begin(), myfs::SearchFile("resource/Models", "obj", false));
                }
                if (ImGui::BeginMenu("Open")) {
                    for (auto& [modelFileDir, modelFileName] : modelFileList_) {
                        if (ImGui::MenuItem(modelFileName.c_str())) {
                            editObject_->Init(modelFileDir, modelFileName);
                        }
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        if (editObject_) {
            auto model = editObject_->getModel();
            if (model) {
                int meshIndex = 0;
                std::string meshLabel;
                for (auto& mesh : model->meshData_->mesh_) {
                    meshLabel = "Mesh[" + std::to_string(meshIndex) + ']';
                    if (ImGui::TreeNode(meshLabel.c_str())) {
                        //============================ Material ============================//
                        if (ImGui::TreeNode("Material")) {
                            for (auto& [materialName, material] : Engine::getInstance()->getMaterialManager()->getMaterialPallet()) {
                                if (ImGui::Button(materialName.c_str())) {
                                    model->setMaterialBuff(meshIndex, material.get());
                                    break;
                                }
                            }
                            ImGui::TreePop();
                        }

                        //============================ TextureSelect ============================//
                        if (ImGui::TreeNode("Textures")) {
                            for (auto& [textureDir, textureName] : textureFileList_) {
                                if (ImGui::Button(textureName.c_str())) {
                                    uint32_t textureIndex = TextureManager::LoadTexture(textureDir + "/" + textureName);
                                    editObject_->setTexture(textureIndex, meshIndex);
                                    break;
                                }
                            }
                            ImGui::TreePop();
                        }
                        ImGui::TreePop();
                    }
                    ++meshIndex;
                }
            }
        }
    }
    ImGui::End();
}

void ModelViewScene::Draw3d() {
    ground_->Draw();

    if (editObject_->getModel()) {
        editObject_->Draw();
    }
}

void ModelViewScene::DrawLine() {
}

void ModelViewScene::DrawSprite() {
}

void ModelViewScene::DrawParticle() {
}
