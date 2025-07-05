#include "SkinningAnimationComponent.h"

/// stl
#include <algorithm>

#define RESOURCE_DIRECTORY
#define ENGINE_INCLUDE
/// engine
#include "AnimationManager.h"
#include "EngineInclude.h"
#include "model/ModelManager.h"

/// ECS
// component
#include "component/renderer/MeshRenderer.h"
#include "ECS/ECSManager.h"

/// lib
#include "myFileSystem/MyFileSystem.h"

#ifdef _DEBUG
#include "myGui/MyGui.h"
#endif // _DEBUG

void SkinningAnimationComponent::Initialize(GameEntity* _entity) {
    entity_ = _entity;

    if (!directory_.empty() && !fileName_.empty()) {
        Load(directory_, fileName_);
    }
    if (animationData_ != nullptr && animationState_.isPlay_) {
        Play();
    }
    currentTime_ = 0;
}

bool SkinningAnimationComponent::Edit() {
    bool isChanged = false;

#ifdef _DEBUG
    ImGui::Text("Animation File: %s", fileName_.c_str());
    if (ImGui::Button("Load")) {
        std::string directory;
        std::string fileName;
        if (myfs::selectFileDialog(kApplicationResourceDirectory, directory, fileName, {"gltf", "anm"})) {
            auto setPath = std::make_unique<SetterCommand<std::string>>(&directory_, kApplicationResourceDirectory + "/" + directory);
            auto setFile = std::make_unique<SetterCommand<std::string>>(&fileName_, fileName);
            CommandCombo commandCombo;
            commandCombo.addCommand(std::move(setPath));
            commandCombo.addCommand(std::move(setFile));
            commandCombo.setFuncOnAfterCommand([this]() {
                animationData_ = AnimationManager::getInstance()->Load(directory_, fileName_);
                duration_      = animationData_->duration;
            },
                true);
            EditorController::getInstance()->pushCommand(std::make_unique<CommandCombo>(commandCombo));
            isChanged = true;
        }
    }

    if (animationData_) {
        isChanged |= DragGuiCommand("Duration", duration_, 0.01f, 0.0f, 100.0f);

        isChanged |= CheckBoxCommand("Play", animationState_.isPlay_);
        isChanged |= CheckBoxCommand("Loop", animationState_.isLoop_);
        isChanged |= DragGuiCommand("Playback Speed", playbackSpeed_, 0.01f, 0.0f);
    }

    int32_t entityModelMeshRendererSize = ECSManager::getInstance()->getComponentArray<ModelMeshRenderer>()->getComponentSize(entity_);
    InputGuiCommand<int32_t>("Bind Mode MeshRenderer Index", bindModeMeshRendererIndex_, "%d",
        [entityModelMeshRendererSize](int32_t* _newVal) {
            *_newVal = std::clamp(*_newVal, 0, entityModelMeshRendererSize - 1);
        });

#endif // _DEBUG

    return isChanged;
}

void SkinningAnimationComponent::Finalize() {
    animationData_.reset();
    DeleteSkinnedVertex();
    entity_ = nullptr;
}

void SkinningAnimationComponent::Load(const std::string& directory, const std::string& fileName) {
    directory_     = directory;
    fileName_      = fileName;
    animationData_ = AnimationManager::getInstance()->Load(directory_, fileName_);
}

void SkinningAnimationComponent::Play() {
    if (animationData_) {
        animationState_.isPlay_ = true;
        animationState_.isEnd_  = false;
        currentTime_            = 0.0f;

        CreateSkinnedVertex();
    }
}

void SkinningAnimationComponent::Stop() {
    if (animationData_) {
        animationState_.isPlay_ = false;
        animationState_.isEnd_  = true;
    }
}

void SkinningAnimationComponent::CreateSkinnedVertex() {
    ModelMeshRenderer* meshRenderer = getComponent<ModelMeshRenderer>(entity_, bindModeMeshRendererIndex_);
    if (!meshRenderer) {
        LOG_ERROR("MeshRenderer not found for SkinningAnimationComponent");
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

    DxDescriptorHeap<DxDescriptorHeapType::CBV_SRV_UAV>* uavHeap = Engine::getInstance()->getSrvHeap(); // cbv_srv_uav heap
    auto& device                                                 = Engine::getInstance()->getDxDevice()->getDevice();
    uint32_t meshGroupSize                                       = static_cast<uint32_t>(meshGroup->size());

    ModelMeshData* modelMeshData = ModelManager::getInstance()->getModelMeshData(meshRenderer->getDirectory(), meshRenderer->getFileName());
    if (!modelMeshData) {
        LOG_ERROR("ModelMeshData not found for directory: {}, fileName: {}", meshRenderer->getDirectory(), meshRenderer->getFileName());
        return;
    }

    // バッファ数が一致しない場合はリサイズ
    if (skinnedVertexBuffer_.size() != meshGroupSize) {

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

void to_json(nlohmann::json& j, const SkinningAnimationComponent& r) {
    j["directory"] = r.directory_;
    j["fileName"]  = r.fileName_;

    j["bindModeMeshRendererIndex"] = r.bindModeMeshRendererIndex_;

    j["duration"] = r.duration_;

    j["playbackSpeed"] = r.playbackSpeed_;
    j["isPlay"]        = r.animationState_.isPlay_;
    j["isLoop"]        = r.animationState_.isLoop_;
}

void from_json(const nlohmann::json& j, SkinningAnimationComponent& r) {
    j.at("directory").get_to(r.directory_);
    j.at("fileName").get_to(r.fileName_);

    j.at("bindModeMeshRendererIndex").get_to(r.bindModeMeshRendererIndex_);

    j.at("duration").get_to(r.duration_);

    j.at("playbackSpeed").get_to(r.playbackSpeed_);
    j.at("isPlay").get_to(r.animationState_.isPlay_);
    j.at("isLoop").get_to(r.animationState_.isLoop_);
}
