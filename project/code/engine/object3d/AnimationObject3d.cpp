#include "AnimationObject3d.h"

#include "Engine.h"

#include "animation/Animation.h"
#include "animation/AnimationManager.h"
#include "material/Material.h"
#include "material/texture/TextureManager.h"
#include "model/Model.h"
#include "model/ModelManager.h"

AnimationObject3d::AnimationObject3d() {}

AnimationObject3d::~AnimationObject3d() {}

void AnimationObject3d::Init(
    const std::string& _directoryPath,
    const std::string& _filename) {
    this->model_ = ModelManager::getInstance()->Create(
        _directoryPath,
        _filename,
        [this](Model* model) {
            transform_.Init();
            transform_.UpdateMatrix();
            for (auto& mesh : model->meshData_->mesh_) {
                mesh.transform_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
                mesh.transform_.openData_.Init();
                mesh.transform_.openData_.parent = &transform_;

                mesh.transform_.openData_.UpdateMatrix();
                mesh.transform_.ConvertToBuffer();
            }
        });
    this->currentAnimationName_ = _filename;
    this->animation_            = AnimationManager::getInstance()->Load(_directoryPath, _filename);
}

void AnimationObject3d::Init(const AnimationSetting& _animationSetting) {
    // model
    this->model_ = ModelManager::getInstance()->Create(
        _animationSetting.targetModelDirection,
        _animationSetting.targetModelFileName,
        [this](Model* model) {
            transform_.Init();
            transform_.UpdateMatrix();
            for (auto& mesh : model->meshData_->mesh_) {
                mesh.transform_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
                mesh.transform_.openData_.Init();
                mesh.transform_.openData_.parent = &transform_;

                mesh.transform_.openData_.UpdateMatrix();
                mesh.transform_.ConvertToBuffer();
            }
        });
    // animation
    this->currentAnimationName_ = _animationSetting.name;
    this->animation_ =
        AnimationManager::getInstance()->Load(
            _animationSetting.targetAnimationDirection,
            _animationSetting.name + ".anm");
}

void AnimationObject3d::Init(
    const std::string& _modelDirectoryPath,
    const std::string& _modelFilename,
    const std::string& _animationDirectoryPath,
    const std::string& _animationFilename) {
    // model
    this->model_ =
        ModelManager::getInstance()->Create(
            _modelDirectoryPath,
            _modelFilename,
            [this](Model* model) {
                transform_.Init();
                transform_.UpdateMatrix();
                for (auto& mesh : model->meshData_->mesh_) {
                    mesh.transform_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());
                    mesh.transform_.openData_.Init();
                    mesh.transform_.openData_.parent = &transform_;

                    mesh.transform_.openData_.UpdateMatrix();
                    mesh.transform_.ConvertToBuffer();
                }
            });

    // animation
    this->currentAnimationName_ = _animationFilename;
    this->animation_ =
        AnimationManager::getInstance()->Load(
            _animationDirectoryPath,
            _animationFilename);
}

void AnimationObject3d::Update(float deltaTime) {
    // Animationより 先に Object 座標系の 行進
    transform_.UpdateMatrix();

    if (toNextAnimation_ && nextAnimation_) {
        // 現在の姿勢から 次のアニメーションの姿勢への補間
        toNextAnimation_->Update(deltaTime, model_.get(), transform_.worldMat);
        if (toNextAnimation_->isEnd()) {
            animation_ = std::move(nextAnimation_);
            nextAnimation_.reset();
            toNextAnimation_.reset();
        }
    } else {
        // アニメーションの更新
        animation_->Update(deltaTime, model_.get(), transform_.worldMat);
    }

    // モデルの更新

    // rootNode
    auto& rootNode = model_->meshData_->rootNode;
    { // rootMeshUpdate
        auto rootMeshIndexItr = model_->meshData_->meshIndexes.find(rootNode.name);
        if (rootMeshIndexItr != model_->meshData_->meshIndexes.end()) {
            Mesh3D& rootMesh                       = model_->meshData_->mesh_[rootMeshIndexItr->second];
            rootMesh.transform_.openData_.worldMat = transform_.worldMat * rootNode.localMatrix;
            rootMesh.transform_.ConvertToBuffer();
        }
    }
    // ChildNode Update
    for (const auto& node : rootNode.children) {
        auto meshIndexItr = model_->meshData_->meshIndexes.find(node.name);
        if (meshIndexItr != model_->meshData_->meshIndexes.end()) {
            IConstantBuffer<Transform>& meshTransform = model_->meshData_->mesh_[meshIndexItr->second].transform_;
            // mesh の ワールド行列を更新
            meshTransform.openData_.worldMat = transform_.worldMat * node.localMatrix;
            meshTransform.ConvertToBuffer();
        }
    }
}

void AnimationObject3d::Draw() {
    drawFuncTable_[(int)model_->meshData_->currentState_]();
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

void AnimationObject3d::setAnimation(const std::string& directory, const std::string& filename) {
    animation_ = std::move(AnimationManager::getInstance()->Load(directory, filename));
}

void AnimationObject3d::setNextAnimation(const std::string& directory, const std::string& filename, float _lerpTime) {
    nextAnimation_ = std::move(AnimationManager::getInstance()->Load(directory, filename));

    AnimationManager* animationManager = AnimationManager::getInstance();
    int toNextAnimationDataIndex       = animationManager->addAnimationData("to" + filename + "from" + currentAnimationName_, std::make_unique<AnimationData>(_lerpTime));
    AnimationData* toNextAnimationData = const_cast<AnimationData*>(animationManager->getAnimationData(toNextAnimationDataIndex));

    toNextAnimationData->nodeAnimations.clear();
    for (const auto& [nodeName, nodeAnimation] : animation_->getData()->nodeAnimations) {
        toNextAnimationData->nodeAnimations[nodeName] = {
            .scale     = AnimationCurve<Vector3>(),
            .rotate    = AnimationCurve<Quaternion>(),
            .translate = AnimationCurve<Vector3>()};

        ///=============================================
        /// 現在の姿勢をはじめに追加
        toNextAnimationData->nodeAnimations[nodeName].scale.push_back(KeyframeVector3(
            0.0f,
            animation_->getCurrentScale(nodeName)));
        toNextAnimationData->nodeAnimations[nodeName].rotate.push_back(KeyframeQuaternion(
            0.0f,
            animation_->getCurrentRotate(nodeName)));
        toNextAnimationData->nodeAnimations[nodeName].translate.push_back(KeyframeVector3(
            0.0f,
            animation_->getCurrentTranslate(nodeName)));

        ///=============================================
        /// 次の姿勢を追加
        toNextAnimationData->nodeAnimations[nodeName].scale.push_back(KeyframeVector3(
            _lerpTime,
            nextAnimation_->getData()->nodeAnimations[nodeName].scale[0].value));
        toNextAnimationData->nodeAnimations[nodeName].rotate.push_back(KeyframeQuaternion(
            _lerpTime,
            nextAnimation_->getData()->nodeAnimations[nodeName].rotate[0].value));
        toNextAnimationData->nodeAnimations[nodeName].translate.push_back(KeyframeVector3(
            _lerpTime,
            nextAnimation_->getData()->nodeAnimations[nodeName].translate[0].value));
    }
    toNextAnimation_ = std::make_unique<Animation>(toNextAnimationData);
    toNextAnimation_->setDuration(_lerpTime);
}

void AnimationObject3d::setAnimation(std::unique_ptr<Animation> animation) {
    animation_ = std::move(animation);
}

void AnimationObject3d::setMaterial(IConstantBuffer<Material>* material, uint32_t index) {
    model_->materialData_[index].material = material;
}
