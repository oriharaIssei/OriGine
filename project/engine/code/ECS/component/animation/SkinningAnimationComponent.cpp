#include "SkinningAnimationComponent.h"

/// stl
#include <algorithm>

#define RESOURCE_DIRECTORY
#define ENGINE_INCLUDE
/// engine
#include "AnimationManager.h"
#include "EngineInclude.h"
#include "entity/Entity.h"
#include "model/ModelManager.h"
#include "scene/Scene.h"
/// ECS
// component
#include "component/renderer/MeshRenderer.h"


#include "myFileSystem/MyFileSystem.h"

#ifdef _DEBUG
#include "myGui/MyGui.h"
#endif // _DEBUG

void to_json(nlohmann::json& j, const SkinningAnimationComponent& r) {
    j["bindModeMeshRendererIndex"] = r.bindModeMeshRendererIndex_;

    j["Animations"] = nlohmann::json::array();
    for (const auto& animation : r.animationTable_) {
        nlohmann::json animationJson;
        animationJson["directory"]     = animation.directory_;
        animationJson["fileName"]      = animation.fileName_;
        animationJson["duration"]      = animation.duration_;
        animationJson["playbackSpeed"] = animation.playbackSpeed_;
        animationJson["isPlay"]        = animation.animationState_.isPlay_;
        animationJson["isLoop"]        = animation.animationState_.isLoop_;
        j["Animations"].push_back(animationJson);
    }
}

void from_json(const nlohmann::json& j, SkinningAnimationComponent& r) {
    j.at("bindModeMeshRendererIndex").get_to(r.bindModeMeshRendererIndex_);

    if (!r.animationTable_.empty()) {
        r.animationTable_.clear();
    }
    if (j.contains("Animations")) {
        for (const auto& animationJson : j.at("Animations")) {
            SkinningAnimationComponent::AnimationCombo animation;
            animation.directory_              = animationJson.at("directory").get<std::string>();
            animation.fileName_               = animationJson.at("fileName").get<std::string>();
            animation.duration_               = animationJson.at("duration").get<float>();
            animation.playbackSpeed_          = animationJson.at("playbackSpeed").get<float>();
            animation.animationState_.isPlay_ = animationJson.at("isPlay").get<bool>();
            animation.animationState_.isLoop_ = animationJson.at("isLoop").get<bool>();
            r.animationTable_.emplace_back(animation);
        }
    }
}

void SkinningAnimationComponent::Initialize(Entity* _entity) {
    entity_ = _entity;

    int32_t animationIndex = 0;
    for (auto& animation : animationTable_) {

        animation.currentTime_           = 0.0f;
        animation.animationState_.isEnd_ = false;

        animation.animationData_ = AnimationManager::getInstance()->Load(
            kApplicationResourceDirectory + "/" + animation.directory_, animation.fileName_);

        this->animationIndexBinder_[animation.fileName_] = animationIndex;
        ++animationIndex;
    }
}

void SkinningAnimationComponent::Edit([[maybe_unused]] Scene* _scene, Entity* /*_entity*/, [[maybe_unused]] const std::string& _parentLabel) {
    
#ifdef _DEBUG

    int32_t entityModelMeshRendererSize = _scene->getComponentArray<ModelMeshRenderer>()->getComponentSize(entity_);
    InputGuiCommand<int32_t>("Bind Mode MeshRenderer Index##" + _parentLabel, bindModeMeshRendererIndex_, "%d",
        [entityModelMeshRendererSize](int32_t* _newVal) {
            *_newVal = std::clamp(*_newVal, 0, entityModelMeshRendererSize - 1);
        });

    ImGui::SeparatorText("Animations");
    std::string label = "+ add" + _parentLabel;
    if (ImGui::Button(label.c_str())) {
        std::string directory;
        std::string fileName;
        if (myfs::selectFileDialog(kApplicationResourceDirectory, directory, fileName, {"gltf", "anm"})) {
            // 既に同じアニメーションが存在する場合は何もしない
            auto itr = animationIndexBinder_.find(fileName);
            if (itr == animationIndexBinder_.end()) {
                // 新しいアニメーションを追加
                auto newAnimation       = AnimationCombo{};
                newAnimation.directory_ = directory;
                newAnimation.fileName_  = fileName;

                newAnimation.animationData_ = AnimationManager::getInstance()->Load(kApplicationResourceDirectory + "/" + directory, fileName);

                newAnimation.duration_ = newAnimation.animationData_->duration;

                auto commandCombo = std::make_unique<CommandCombo>();
                commandCombo->addCommand(std::make_shared<AddElementCommand<std::vector<AnimationCombo>>>(&animationTable_, newAnimation));
                commandCombo->setFuncOnAfterCommand([this, fileName]() {
                    animationIndexBinder_[fileName] = static_cast<int32_t>(animationTable_.size()) - 1;
                },
                    true);
                EditorController::getInstance()->pushCommand(std::move(commandCombo));
            } else {
                LOG_ERROR("Animation with name '{}' already exists.", fileName);
            }
        }
    }
    ImGui::Spacing();

    std::string nodeLabel = "";
    int32_t index         = 0;
    for (auto& animation : animationTable_) {
        nodeLabel = animation.fileName_ + "##" + _parentLabel;
        if (ImGui::TreeNode(nodeLabel.c_str())) {
            ImGui::Text("Animation File: %s", animation.fileName_.c_str());

            nodeLabel = "Load" + animation.fileName_ + "##" + _parentLabel;
            if (ImGui::Button(nodeLabel.c_str())) {
                std::string directory;
                std::string fileName;
                if (myfs::selectFileDialog(kApplicationResourceDirectory, directory, fileName, {"gltf", "anm"})) {
                    auto setPath = std::make_unique<SetterCommand<std::string>>(&animation.directory_, kApplicationResourceDirectory + "/" + directory);
                    auto setFile = std::make_unique<SetterCommand<std::string>>(&animation.fileName_, fileName);
                    CommandCombo commandCombo;
                    commandCombo.addCommand(std::move(setPath));
                    commandCombo.addCommand(std::move(setFile));
                    commandCombo.setFuncOnAfterCommand([this, index]() {
                        auto& animation          = animationTable_[index];
                        animation.animationData_ = AnimationManager::getInstance()->Load(animation.directory_, animation.fileName_);
                        animation.duration_      = animation.animationData_->duration;
                    },
                        true);
                    EditorController::getInstance()->pushCommand(std::make_unique<CommandCombo>(commandCombo));
                }
            }

            if (animation.animationData_) {
                DragGuiCommand("Duration##" + _parentLabel, animation.duration_, 0.01f, 0.0f, 100.0f);

                CheckBoxCommand("Play##" + _parentLabel, animation.animationState_.isPlay_);
                CheckBoxCommand("Loop##" + _parentLabel, animation.animationState_.isLoop_);
                DragGuiCommand("Playback Speed##" + _parentLabel, animation.playbackSpeed_, 0.01f, 0.0f);
            }

            ImGui::TreePop();
        }
    }

#endif // _DEBUG
}

void SkinningAnimationComponent::Finalize() {
    DeleteSkinnedVertex();

    animationIndexBinder_.clear();
    animationTable_.clear();

    currentAnimationIndex_ = 0;
    blendingAnimationData_ = std::nullopt;

    bindModeMeshRendererIndex_ = -1;

    entity_ = nullptr;
}

void SkinningAnimationComponent::addLoad(const std::string& directory, const std::string& fileName) {
    if (directory.empty() || fileName.empty()) {
        LOG_ERROR("Directory or fileName is empty.");
        return;
    }
    // 既に同じアニメーションが存在する場合は何もしない
    auto itr = animationIndexBinder_.find(fileName);
    if (itr != animationIndexBinder_.end()) {
        LOG_ERROR("Animation with name '{}' already exists.", fileName);
        return;
    }
    // 新しいアニメーションを追加
    auto& newAnimation      = animationTable_.emplace_back(AnimationCombo{});
    newAnimation.directory_ = directory;
    newAnimation.fileName_  = fileName;

    newAnimation.animationData_ = AnimationManager::getInstance()->Load(kApplicationResourceDirectory + "/" + directory, fileName);

    animationIndexBinder_[fileName] = static_cast<int32_t>(animationTable_.size()) - 1;
}

void SkinningAnimationComponent::Play() {
    auto& animation = animationTable_[currentAnimationIndex_];

    if (!animation.animationData_) {
        animation.animationData_ = AnimationManager::getInstance()->Load(animation.directory_, animation.fileName_);
    }
    animation.animationState_.isPlay_ = true;
    animation.animationState_.isEnd_  = false;
    animation.currentTime_            = 0.0f;
}

void SkinningAnimationComponent::Play(int32_t index) {
    if (index < 0 || index >= static_cast<int32_t>(animationTable_.size())) {
        LOG_ERROR("Invalid animation index: {}", index);
        return;
    }
    auto& animation = animationTable_[index];
    if (!animation.animationData_) {
        animation.animationData_ = AnimationManager::getInstance()->Load(animation.directory_, animation.fileName_);
    }
    animation.animationState_.isPlay_ = true;
    animation.prePlay_                = false; // 前のアニメーションを停止
    animation.animationState_.isEnd_  = false;
    animation.currentTime_            = 0.0f;
}

void SkinningAnimationComponent::Play(const std::string& name) {
    int32_t index = getAnimationIndex(name);
    if (index < 0 || index >= static_cast<int32_t>(animationTable_.size())) {
        LOG_ERROR("Invalid animation name: {}", name);
        return;
    }
    Play(index);
}

void SkinningAnimationComponent::PlayNext( int32_t index, float _blendTime) {
    if (index < 0 || index >= static_cast<int32_t>(animationTable_.size())) {
        LOG_ERROR("Invalid animation index: {}", index);
        return;
    }

    blendingAnimationData_ = AnimationBlendData{index, _blendTime, 0.0f};

    auto& nextAnimation = animationTable_[blendingAnimationData_.value().targetAnimationIndex_];
    if (!nextAnimation.animationData_) {
        nextAnimation.animationData_ = AnimationManager::getInstance()->Load(nextAnimation.directory_, nextAnimation.fileName_);
    }
    nextAnimation.animationState_.isPlay_ = true;
    nextAnimation.prePlay_                = false; // 前のアニメーションを停止
    nextAnimation.animationState_.isEnd_  = false;
    nextAnimation.currentTime_            = 0.0f;
}

void SkinningAnimationComponent::PlayNext(const std::string& name, float _blendTime) {
    int32_t index = getAnimationIndex(name);
    if (index < 0 || index >= static_cast<int32_t>(animationTable_.size())) {
        LOG_ERROR("Invalid animation name: {}", name);
        return;
    }
    PlayNext(index, _blendTime);
}

void SkinningAnimationComponent::Stop() {
    auto& animation = animationTable_[currentAnimationIndex_];

    animation.animationState_.isPlay_ = false;
    animation.currentTime_            = 0.0f;
}

void SkinningAnimationComponent::CreateSkinnedVertex(Scene* _scene) {
    DxDescriptorHeap<DxDescriptorHeapType::CBV_SRV_UAV>* uavHeap = Engine::getInstance()->getSrvHeap(); // cbv_srv_uav heap
    auto& device                                                 = Engine::getInstance()->getDxDevice()->getDevice();

    ModelMeshRenderer* meshRenderer = _scene->getComponent<ModelMeshRenderer>(entity_, bindModeMeshRendererIndex_);
    if (!meshRenderer) {
        LOG_ERROR("MeshRenderer not found for SkinningAnimationComponent");
        return;
    }
    ModelMeshData* modelMeshData = ModelManager::getInstance()->getModelMeshData(meshRenderer->getDirectory(), meshRenderer->getFileName());
    if (!modelMeshData) {
        LOG_ERROR("ModelMeshData not found for directory: {}, fileName: {}", meshRenderer->getDirectory(), meshRenderer->getFileName());
        return;
    }
    auto* meshGroup = meshRenderer->getMeshGroup().get();
    if (!meshGroup) {
        LOG_ERROR(
            "MeshGroup is null in SkinningAnimationComponent.\n EntityName : {}\n EntityID   : {}\n ModelName  : {}\n",
            entity_->getDataType(),
            entity_->getID(),
            meshRenderer->getFileName());
        return;
    }

    uint32_t meshGroupSize = static_cast<uint32_t>(meshGroup->size());
    // スキニングされた頂点バッファのサイズがメッシュグループのサイズと一致しない場合はリサイズ
    if (skinnedVertexBuffer_.size() != meshGroupSize) {

        // バッファ数が一致しない場合はリサイズ

        if (skinnedVertexBuffer_.size() > meshGroupSize) {
            // 縮小時、削除されるバッファのFinalizeを呼ぶ
            for (size_t i = meshGroupSize; i < skinnedVertexBuffer_.size(); ++i) {
                if (!skinnedVertexBuffer_[i].buffer.isValid()) {
                    continue;
                }
                skinnedVertexBuffer_[i].buffer.Finalize();
                uavHeap->ReleaseDescriptor(skinnedVertexBuffer_[i].descriptor);
                skinnedVertexBuffer_[i].descriptor.reset();
            }
        }
        skinnedVertexBuffer_.resize(meshGroupSize);
    }

    for (uint32_t i = 0; i < meshGroupSize; ++i) {
        auto& mesh                = (*meshGroup)[i];
        size_t requiredBufferSize = mesh.vertexes_.size() * sizeof(decltype(mesh.vertexes_)::value_type);

        bool needRecreate = false;

        // バッファが未生成、またはサイズが異なる場合は再生成
        if (!skinnedVertexBuffer_[i].buffer.isValid() || skinnedVertexBuffer_[i].buffer.sizeInBytes() != requiredBufferSize) {
            needRecreate = true;
        }

        if (needRecreate) {
            // 既存バッファを解放（必要なら）
            skinnedVertexBuffer_[i].buffer.Finalize();

            // 新規バッファ生成
            skinnedVertexBuffer_[i].buffer = DxResource();
            skinnedVertexBuffer_[i].buffer.CreateUAVBuffer(device, requiredBufferSize);

            // UAVディスクリプタも再生成
            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
            uavDesc.Format                      = DXGI_FORMAT_UNKNOWN; // UAVはフォーマットを持たない
            uavDesc.ViewDimension               = D3D12_UAV_DIMENSION_BUFFER;
            uavDesc.Buffer.FirstElement         = 0;
            uavDesc.Buffer.NumElements          = static_cast<uint32_t>(mesh.vertexes_.size());
            uavDesc.Buffer.CounterOffsetInBytes = 0; // カウンターオフセットは0
            uavDesc.Buffer.Flags                = D3D12_BUFFER_UAV_FLAG_NONE; // 特にフラグは必要ない
            uavDesc.Buffer.StructureByteStride  = sizeof(decltype(mesh.vertexes_)::value_type);
            skinnedVertexBuffer_[i].descriptor  = uavHeap->CreateDescriptor(uavDesc, &skinnedVertexBuffer_[i].buffer);

            skinnedVertexBuffer_[i].vbView.BufferLocation = skinnedVertexBuffer_[i].buffer.getResource()->GetGPUVirtualAddress();
            skinnedVertexBuffer_[i].vbView.SizeInBytes    = static_cast<UINT>(skinnedVertexBuffer_[i].buffer.sizeInBytes());
            skinnedVertexBuffer_[i].vbView.StrideInBytes  = sizeof(decltype(mesh.vertexes_)::value_type);
        }

        if (!modelMeshData->skeleton.has_value()) {
            LOG_ERROR("Skeleton not found in ModelMeshData for directory: {}, fileName: {}", meshRenderer->getDirectory(), meshRenderer->getFileName());
            skeleton_ = Skeleton{};
            continue;
        }
        skeleton_ = modelMeshData->skeleton.value();
    }
}
void SkinningAnimationComponent::DeleteSkinnedVertex() {
    // UAVディスクリプタを解放
    for (auto& skinnedVertex : skinnedVertexBuffer_) {
        if (skinnedVertex.descriptor) {
            Engine::getInstance()->getSrvHeap()->ReleaseDescriptor(skinnedVertex.descriptor);
            skinnedVertex.descriptor.reset();
        }
        skinnedVertex.buffer.Finalize();
    }
    skinnedVertexBuffer_.clear();
}
