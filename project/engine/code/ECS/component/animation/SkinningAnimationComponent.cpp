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
        animationJson["directory"]     = animation.directory;
        animationJson["fileName"]      = animation.fileName;
        animationJson["duration"]      = animation.duration;
        animationJson["playbackSpeed"] = animation.playbackSpeed;
        animationJson["isPlay"]        = animation.animationState.isPlay_;
        animationJson["isLoop"]        = animation.animationState.isLoop_;
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
            animation.directory              = animationJson.at("directory").get<std::string>();
            animation.fileName               = animationJson.at("fileName").get<std::string>();
            animation.duration               = animationJson.at("duration").get<float>();
            animation.playbackSpeed          = animationJson.at("playbackSpeed").get<float>();
            animation.animationState.isPlay_ = animationJson.at("isPlay").get<bool>();
            animation.animationState.isLoop_ = animationJson.at("isLoop").get<bool>();
            r.animationTable_.emplace_back(animation);
        }
    }
}

void SkinningAnimationComponent::Initialize(Entity* _entity) {
    entity_ = _entity;

    int32_t animationIndex = 0;
    for (auto& animation : animationTable_) {

        animation.currentTime           = 0.0f;
        animation.animationState.isEnd_ = false;

        animation.animationData = AnimationManager::GetInstance()->Load(
            kApplicationResourceDirectory + "/" + animation.directory, animation.fileName);

        this->animationIndexBinder_[animation.fileName] = animationIndex;
        ++animationIndex;
    }
}

void SkinningAnimationComponent::Edit([[maybe_unused]] Scene* _scene, Entity* /*_entity*/, [[maybe_unused]] const std::string& _parentLabel) {

#ifdef _DEBUG

    int32_t entityModelMeshRendererSize = _scene->GetComponentArray<ModelMeshRenderer>()->GetComponentSize(entity_);
    InputGuiCommand<int32_t>("Bind Mode MeshRenderer Index##" + _parentLabel, bindModeMeshRendererIndex_, "%d",
        [entityModelMeshRendererSize](int32_t* _newVal) {
            *_newVal = std::clamp(*_newVal, 0, entityModelMeshRendererSize - 1);
        });

    ImGui::SeparatorText("Animations");
    std::string label = "+ add" + _parentLabel;
    if (ImGui::Button(label.c_str())) {
        std::string directory;
        std::string fileName;
        if (myfs::SelectFileDialog(kApplicationResourceDirectory, directory, fileName, {"gltf", "anm"})) {
            // 既に同じアニメーションが存在する場合は何もしない
            auto itr = animationIndexBinder_.find(fileName);
            if (itr == animationIndexBinder_.end()) {
                // 新しいアニメーションを追加
                auto newAnimation       = AnimationCombo{};
                newAnimation.directory = directory;
                newAnimation.fileName  = fileName;

                newAnimation.animationData = AnimationManager::GetInstance()->Load(kApplicationResourceDirectory + "/" + directory, fileName);

                newAnimation.duration = newAnimation.animationData->duration;

                auto commandCombo = std::make_unique<CommandCombo>();
                commandCombo->AddCommand(std::make_shared<AddElementCommand<std::vector<AnimationCombo>>>(&animationTable_, newAnimation));
                commandCombo->SetFuncOnAfterCommand([this, fileName]() {
                    animationIndexBinder_[fileName] = static_cast<int32_t>(animationTable_.size()) - 1;
                },
                    true);
                EditorController::GetInstance()->PushCommand(std::move(commandCombo));
            } else {
                LOG_ERROR("Animation with name '{}' already exists.", fileName);
            }
        }
    }
    ImGui::Spacing();

    std::string nodeLabel = "";
    int32_t index         = 0;
    for (auto& animation : animationTable_) {
        nodeLabel = animation.fileName + "##" + _parentLabel;
        if (ImGui::TreeNode(nodeLabel.c_str())) {
            ImGui::Text("Animation File: %s", animation.fileName.c_str());

            nodeLabel = "Load" + animation.fileName + "##" + _parentLabel;
            if (ImGui::Button(nodeLabel.c_str())) {
                std::string directory;
                std::string fileName;
                if (myfs::SelectFileDialog(kApplicationResourceDirectory, directory, fileName, {"gltf", "anm"})) {
                    auto SetPath = std::make_unique<SetterCommand<std::string>>(&animation.directory, kApplicationResourceDirectory + "/" + directory);
                    auto SetFile = std::make_unique<SetterCommand<std::string>>(&animation.fileName, fileName);
                    CommandCombo commandCombo;
                    commandCombo.AddCommand(std::move(SetPath));
                    commandCombo.AddCommand(std::move(SetFile));
                    commandCombo.SetFuncOnAfterCommand([this, index]() {
                        auto& animation          = animationTable_[index];
                        animation.animationData = AnimationManager::GetInstance()->Load(animation.directory, animation.fileName);
                        animation.duration      = animation.animationData->duration;
                    },
                        true);
                    EditorController::GetInstance()->PushCommand(std::make_unique<CommandCombo>(commandCombo));
                }
            }

            if (animation.animationData) {
                DragGuiCommand("Duration##" + _parentLabel, animation.duration, 0.01f, 0.0f, 100.0f);

                CheckBoxCommand("Play##" + _parentLabel, animation.animationState.isPlay_);
                CheckBoxCommand("Loop##" + _parentLabel, animation.animationState.isLoop_);
                DragGuiCommand("Playback Speed##" + _parentLabel, animation.playbackSpeed, 0.01f, 0.0f);
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

void SkinningAnimationComponent::AddLoad(const std::string& directory, const std::string& fileName) {
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
    newAnimation.directory = directory;
    newAnimation.fileName  = fileName;

    newAnimation.animationData = AnimationManager::GetInstance()->Load(kApplicationResourceDirectory + "/" + directory, fileName);

    animationIndexBinder_[fileName] = static_cast<int32_t>(animationTable_.size()) - 1;
}

void SkinningAnimationComponent::Play() {
    auto& animation = animationTable_[currentAnimationIndex_];

    if (!animation.animationData) {
        animation.animationData = AnimationManager::GetInstance()->Load(animation.directory, animation.fileName);
    }
    animation.animationState.isPlay_ = true;
    animation.animationState.isEnd_  = false;
    animation.currentTime            = 0.0f;
}

void SkinningAnimationComponent::Play(int32_t index) {
    if (index < 0 || index >= static_cast<int32_t>(animationTable_.size())) {
        LOG_ERROR("Invalid animation index: {}", index);
        return;
    }
    auto& animation = animationTable_[index];
    if (!animation.animationData) {
        animation.animationData = AnimationManager::GetInstance()->Load(animation.directory, animation.fileName);
    }
    animation.animationState.isPlay_ = true;
    animation.prePlay                = false; // 前のアニメーションを停止
    animation.animationState.isEnd_  = false;
    animation.currentTime            = 0.0f;
}

void SkinningAnimationComponent::Play(const std::string& name) {
    int32_t index = GetAnimationIndex(name);
    if (index < 0 || index >= static_cast<int32_t>(animationTable_.size())) {
        LOG_ERROR("Invalid animation name: {}", name);
        return;
    }
    Play(index);
}

void SkinningAnimationComponent::PlayNext(int32_t index, float _blendTime) {
    if (index < 0 || index >= static_cast<int32_t>(animationTable_.size())) {
        LOG_ERROR("Invalid animation index: {}", index);
        return;
    }

    blendingAnimationData_ = AnimationBlendData{index, _blendTime, 0.0f};

    auto& nextAnimation = animationTable_[blendingAnimationData_.value().targetAnimationIndex];
    if (!nextAnimation.animationData) {
        nextAnimation.animationData = AnimationManager::GetInstance()->Load(nextAnimation.directory, nextAnimation.fileName);
    }
    nextAnimation.animationState.isPlay_ = true;
    nextAnimation.prePlay                = false; // 前のアニメーションを停止
    nextAnimation.animationState.isEnd_  = false;
    nextAnimation.currentTime            = 0.0f;
}

void SkinningAnimationComponent::PlayNext(const std::string& name, float _blendTime) {
    int32_t index = GetAnimationIndex(name);
    if (index < 0 || index >= static_cast<int32_t>(animationTable_.size())) {
        LOG_ERROR("Invalid animation name: {}", name);
        return;
    }
    PlayNext(index, _blendTime);
}

void SkinningAnimationComponent::Stop() {
    auto& animation = animationTable_[currentAnimationIndex_];

    animation.animationState.isPlay_ = false;
    animation.currentTime            = 0.0f;
}

void SkinningAnimationComponent::CreateSkinnedVertex(Scene* _scene) {
    DxDescriptorHeap<DxDescriptorHeapType::CBV_SRV_UAV>* uavHeap = Engine::GetInstance()->GetSrvHeap(); // cbv_srv_uav heap
    auto& device                                                 = Engine::GetInstance()->GetDxDevice()->device_;

    ModelMeshRenderer* meshRenderer = _scene->GetComponent<ModelMeshRenderer>(entity_, bindModeMeshRendererIndex_);
    if (!meshRenderer) {
        LOG_ERROR("MeshRenderer not found for SkinningAnimationComponent");
        return;
    }
    ModelMeshData* modelMeshData = ModelManager::GetInstance()->GetModelMeshData(meshRenderer->GetDirectory(), meshRenderer->GetFileName());
    if (!modelMeshData) {
        LOG_ERROR("ModelMeshData not found for directory: {}, fileName: {}", meshRenderer->GetDirectory(), meshRenderer->GetFileName());
        return;
    }
    auto* meshGroup = meshRenderer->GetMeshGroup().get();
    if (!meshGroup) {
        LOG_ERROR(
            "MeshGroup is null in SkinningAnimationComponent.\n EntityName : {}\n EntityID   : {}\n ModelName  : {}\n",
            entity_->GetDataType(),
            entity_->GetID(),
            meshRenderer->GetFileName());
        return;
    }

    uint32_t meshGroupSize = static_cast<uint32_t>(meshGroup->size());
    // スキニングされた頂点バッファのサイズがメッシュグループのサイズと一致しない場合はリサイズ
    if (skinnedVertexBuffer_.size() != meshGroupSize) {

        // バッファ数が一致しない場合はリサイズ

        if (skinnedVertexBuffer_.size() > meshGroupSize) {
            // 縮小時、削除されるバッファのFinalizeを呼ぶ
            for (size_t i = meshGroupSize; i < skinnedVertexBuffer_.size(); ++i) {
                if (!skinnedVertexBuffer_[i].buffer.IsValid()) {
                    continue;
                }
                skinnedVertexBuffer_[i].buffer.Finalize();
                uavHeap->ReleaseDescriptor(skinnedVertexBuffer_[i].descriptor);
            }
        }
        skinnedVertexBuffer_.resize(meshGroupSize);
    }

    for (uint32_t i = 0; i < meshGroupSize; ++i) {
        auto& mesh                = (*meshGroup)[i];
        size_t requiredBufferSize = mesh.vertexes_.size() * sizeof(decltype(mesh.vertexes_)::value_type);

        bool needRecreate = false;

        // バッファが未生成、またはサイズが異なる場合は再生成
        if (!skinnedVertexBuffer_[i].buffer.IsValid() || skinnedVertexBuffer_[i].buffer.GetSizeInBytes() != requiredBufferSize) {
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

            skinnedVertexBuffer_[i].vbView.BufferLocation = skinnedVertexBuffer_[i].buffer.GetResource()->GetGPUVirtualAddress();
            skinnedVertexBuffer_[i].vbView.SizeInBytes    = static_cast<UINT>(skinnedVertexBuffer_[i].buffer.GetSizeInBytes());
            skinnedVertexBuffer_[i].vbView.StrideInBytes  = sizeof(decltype(mesh.vertexes_)::value_type);
        }

        if (!modelMeshData->skeleton.has_value()) {
            LOG_ERROR("Skeleton not found in ModelMeshData for directory: {}, fileName: {}", meshRenderer->GetDirectory(), meshRenderer->GetFileName());
            skeleton_ = Skeleton{};
            continue;
        }
        skeleton_ = modelMeshData->skeleton.value();
    }
}
void SkinningAnimationComponent::DeleteSkinnedVertex() {
    // UAVディスクリプタを解放
    for (auto& skinnedVertex : skinnedVertexBuffer_) {
        if (skinnedVertex.descriptor.GetIndex() >= 0) {
            Engine::GetInstance()->GetSrvHeap()->ReleaseDescriptor(skinnedVertex.descriptor);
        }
        skinnedVertex.buffer.Finalize();
    }
    skinnedVertexBuffer_.clear();
}
