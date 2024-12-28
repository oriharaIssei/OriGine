#include "AnimationObject3d.h"

#include "animation/Animation.h"
#include "animation/AnimationManager.h"
#include "material/Material.h"
#include "material/texture/TextureManager.h"
#include "model/Model.h"
#include "model/ModelManager.h"

#pragma region "Static"
std::unique_ptr<AnimationObject3d> AnimationObject3d::Create(const std::string& _directoryPath, const std::string& _filename) {
    std::unique_ptr<AnimationObject3d> newInstance = std::make_unique<AnimationObject3d>();
    newInstance->model_                            = ModelManager::getInstance()->Create(_directoryPath, _filename);
    newInstance->animation_                        = AnimationManager::getInstance()->Load(_directoryPath, _filename);
    return newInstance;
}

std::unique_ptr<AnimationObject3d> AnimationObject3d::Create(const AnimationSetting& _animationSetting) {
    std::unique_ptr<AnimationObject3d> newInstance = std::make_unique<AnimationObject3d>();
    // model
    newInstance->model_ = ModelManager::getInstance()->Create(_animationSetting.targetModelDirection, _animationSetting.targetModelFileName);
    // animation
    newInstance->animation_ = AnimationManager::getInstance()->Load(_animationSetting.targetAnimationDirection, _animationSetting.name + ".anm");
    return newInstance;
}

std::unique_ptr<AnimationObject3d> AnimationObject3d::Create(const std::string& _modelDirectoryPath, const std::string& _modelFilename, const std::string& _animationDirectoryPath, const std::string& _animationFilename) {
    std::unique_ptr<AnimationObject3d> newInstance = std::make_unique<AnimationObject3d>();
    // model
    newInstance->model_ = ModelManager::getInstance()->Create(_modelDirectoryPath, _modelFilename);
    // animation
    newInstance->animation_ = AnimationManager::getInstance()->Load(_animationDirectoryPath, _animationFilename);
    return newInstance;
}
#pragma endregion

AnimationObject3d::AnimationObject3d() {}

AnimationObject3d::~AnimationObject3d() {}

void AnimationObject3d::Update(float deltaTime) {
    // Animationより 先に Object 座標系の 行進
    transform_.UpdateMatrix();

    // アニメーションの更新
    animation_->Update(deltaTime, model_.get(), transform_.worldMat);

    // モデルの更新

    // rootNode
    auto& rootNode                                                                                        = model_->meshData_->rootNode;
    model_->meshData_->mesh_[model_->meshData_->meshIndexes[rootNode.name]].transform_.openData_.worldMat = transform_.worldMat * rootNode.localMatrix;

    // ChildNode
    for (const auto& node : rootNode.children) {
        auto& mesh = model_->meshData_->mesh_[model_->meshData_->meshIndexes[node.name]];
        // mesh の ワールド行列を更新
        mesh.transform_.openData_.worldMat = transform_.worldMat * node.localMatrix;
    }
    for (auto& mesh : model_->meshData_->mesh_) {
        mesh.transform_.ConvertToBuffer();
    }
}

void AnimationObject3d::Draw() {
    drawFuncTable_[(int)model_->currentState_]();
}

void AnimationObject3d::DrawThis() {
    ModelManager* manager = ModelManager::getInstance();
    auto* commandList     = manager->dxCommand_->getCommandList();

    uint32_t index = 0;

    for (auto& mesh : model_->meshData_->mesh_) {
        auto& material                  = model_->materialData_[index];
        ID3D12DescriptorHeap* ppHeaps[] = {DxHeap::getInstance()->getSrvHeap()};
        commandList->SetDescriptorHeaps(1, ppHeaps);
        commandList->SetGraphicsRootDescriptorTable(
            7,
            TextureManager::getDescriptorGpuHandle(material.textureNumber));

        commandList->IASetVertexBuffers(0, 1, &mesh.meshBuff->vbView);
        commandList->IASetIndexBuffer(&mesh.meshBuff->ibView);

        mesh.transform_.SetForRootParameter(commandList, 0);

        material.material->SetForRootParameter(commandList, 2);
        // 描画!!!
        commandList->DrawIndexedInstanced(UINT(mesh.indexSize), 1, 0, 0, 0);

        ++index;
    }
}

const Model* AnimationObject3d::getModel() const {
    return model_.get();
}

Model* AnimationObject3d::getModel() {
    return model_.get();
}

void AnimationObject3d::setModel(std::unique_ptr<Model> model) {
    model_ = std::move(model);
}
void AnimationObject3d::setModel(const std::string& directory, const std::string& filename) {
    model_ = ModelManager::getInstance()->Create(directory, filename);
}

const Animation* AnimationObject3d::getAnimation() const {
    return animation_.get();
}

Animation* AnimationObject3d::getAnimation() {
    return animation_.get();
}

void AnimationObject3d::setAnimation(std::unique_ptr<Animation> animation) {
    animation_ = std::move(animation);
}

void AnimationObject3d::setMaterial(IConstantBuffer<Material>* material, uint32_t index) {
    model_->materialData_[index].material = material;
}
