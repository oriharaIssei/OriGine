#include "SkinningAnimationComponent.h"

/// stl
#include <algorithm>

/// engine
#define RESOURCE_DIRECTORY
#define ENGINE_INCLUDE
#include "EngineInclude.h"
#include "model/ModelManager.h"
#include "scene/Scene.h"
// asset
#include "asset/AssetSystem.h"
#include "asset/manager/AnimationAssetManager.h"
/// ECS
#include "entity/Entity.h"
// component
#include "component/renderer/ModelMeshRenderer.h"
/// util
#include "myFileSystem/MyFileSystem.h"
/// externals
#ifdef ORIGINE_EDITOR_ENABLED
#include "myGui/MyGui.h"
#endif // _DEBUG

using namespace OriGine;

/// <summary>
/// アセットインデックスからアニメーションデータを解決する.
/// </summary>
const AnimationData* SkinningAnimationComponent::ResolveAnimationData(size_t _assetIndex) {
    if (_assetIndex == kInvalidAssetIndex) {
        return nullptr;
    }
    auto* manager = AssetSystem::GetInstance()->GetManager<AnimationAsset>();
    if (manager == nullptr) {
        return nullptr;
    }
    return manager->IsAlive(_assetIndex) ? &manager->GetAsset(_assetIndex).data : nullptr;
}

/// <summary>
/// AnimationCombo が参照しているアセットを解放する.
/// </summary>
void SkinningAnimationComponent::ReleaseAnimationAsset(AnimationCombo& _combo) {
    if (_combo.animationAssetIndex == kInvalidAssetIndex) {
        return;
    }
    if (auto* manager = AssetSystem::GetInstance()->GetManager<AnimationAsset>()) {
        manager->ReleaseAsset(_combo.animationAssetIndex);
    }
    _combo.animationAssetIndex = kInvalidAssetIndex;
}

/// <summary>
/// AnimationCombo の directory / fileName に従ってアセットを取得し直す.
/// </summary>
const AnimationData* SkinningAnimationComponent::LoadAnimationAsset(AnimationCombo& _combo) {
    ReleaseAnimationAsset(_combo);

    if (_combo.fileName.empty()) {
        return nullptr;
    }
    auto* manager = AssetSystem::GetInstance()->GetManager<AnimationAsset>();
    if (manager == nullptr) {
        LOG_ERROR("AnimationAssetManager is not registered.");
        return nullptr;
    }

    // AnimationCombo::directory はシーンJSONに保存される値であり、
    // アプリのリソースディレクトリからの相対パスとして扱う（絶対パスを入れないこと）
    std::string assetPath = kApplicationResourceDirectory;
    if (!_combo.directory.empty()) {
        assetPath += "/" + _combo.directory;
    }
    assetPath += "/" + _combo.fileName;

    _combo.animationAssetIndex = manager->LoadAsset(assetPath);
    return ResolveAnimationData(_combo.animationAssetIndex);
}

void OriGine::to_json(nlohmann::json& _j, const SkinningAnimationComponent& _comp) {
    _j["bindModeMeshRendererIndex"] = _comp.bindModeMeshRendererIndex_;

    _j["Animations"] = nlohmann::json::array();
    for (const auto& animation : _comp.animationTable_) {
        nlohmann::json animationJson;
        animationJson["directory"]     = animation.directory;
        animationJson["fileName"]      = animation.fileName;
        animationJson["duration"]      = animation.duration;
        animationJson["playbackSpeed"] = animation.playbackSpeed;
        animationJson["isPlay"]        = animation.animationState.isPlay_;
        animationJson["isLoop"]        = animation.animationState.isLoop_;
        _j["Animations"].push_back(animationJson);
    }
}

void OriGine::from_json(const nlohmann::json& _j, SkinningAnimationComponent& _comp) {
    _j.at("bindModeMeshRendererIndex").get_to(_comp.bindModeMeshRendererIndex_);

    if (!_comp.animationTable_.empty()) {
        _comp.animationTable_.clear();
    }
    if (_j.contains("Animations")) {
        for (const auto& animationJson : _j.at("Animations")) {
            SkinningAnimationComponent::AnimationCombo animation;
            animation.directory              = animationJson.at("directory").get<std::string>();
            animation.fileName               = animationJson.at("fileName").get<std::string>();
            animation.duration               = animationJson.at("duration").get<float>();
            animation.playbackSpeed          = animationJson.at("playbackSpeed").get<float>();
            animation.animationState.isPlay_ = animationJson.at("isPlay").get<bool>();
            animation.animationState.isLoop_ = animationJson.at("isLoop").get<bool>();
            _comp.animationTable_.emplace_back(animation);
        }
    }
}

void SkinningAnimationComponent::Initialize(Scene* /*_scene*/, const EntityHandle& _entity) {
    entityHandle_ = _entity;

    int32_t animationIndex = 0;
    for (auto& animation : animationTable_) {

        animation.currentTime           = 0.0f;
        animation.animationState.isEnd_ = false;

        LoadAnimationAsset(animation);

        this->animationIndexBinder_[animation.fileName] = animationIndex;
        ++animationIndex;
    }
}

void SkinningAnimationComponent::Edit([[maybe_unused]] Scene* _scene, const EntityHandle& /*_entity*/, [[maybe_unused]] const std::string& _parentLabel) {

#ifdef ORIGINE_EDITOR_ENABLED

    auto& modelMeshes = _scene->GetComponents<ModelMeshRenderer>(entityHandle_);
    InputGuiCommand<int32_t>("Bind Mode MeshRenderer Index##" + _parentLabel, bindModeMeshRendererIndex_, "%d",
        [meshRenderSize = modelMeshes.size()](int32_t* _newVal) {
            *_newVal = std::clamp(*_newVal, 0, static_cast<int32_t>(meshRenderSize) - 1);
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
                auto newAnimation      = AnimationCombo{};
                newAnimation.directory = directory;
                newAnimation.fileName  = fileName;

                if (const AnimationData* data = LoadAnimationAsset(newAnimation)) {
                    newAnimation.duration = data->duration;
                }

                auto commandCombo = std::make_unique<CommandCombo>();
                commandCombo->AddCommand(std::make_shared<AddElementCommand<std::vector<AnimationCombo>>>(&animationTable_, newAnimation));
                commandCombo->SetFuncOnAfterCommand([this, fileName]() {
                    animationIndexBinder_[fileName] = static_cast<int32_t>(animationTable_.size()) - 1;
                },
                    true);
                OriGine::EditorController::GetInstance()->PushCommand(std::move(commandCombo));
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
                    // directory は相対パスで保持する（LoadAnimationAsset 側で
                    // kApplicationResourceDirectory を前置するため、ここで絶対パスにしてはいけない）
                    auto SetPath = std::make_unique<SetterCommand<std::string>>(&animation.directory, directory);
                    auto SetFile = std::make_unique<SetterCommand<std::string>>(&animation.fileName, fileName);
                    CommandCombo commandCombo;
                    commandCombo.AddCommand(std::move(SetPath));
                    commandCombo.AddCommand(std::move(SetFile));
                    commandCombo.SetFuncOnAfterCommand([this, index]() {
                        auto& animation = animationTable_[index];
                        if (const AnimationData* data = LoadAnimationAsset(animation)) {
                            animation.duration = data->duration;
                        }
                    },
                        true);
                    OriGine::EditorController::GetInstance()->PushCommand(std::make_unique<CommandCombo>(commandCombo));
                }
            }

            if (ResolveAnimationData(animation.animationAssetIndex)) {
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

    // 参照カウントを戻しておかないと、コンポーネント破棄後もアセットが解放されない
    for (auto& animation : animationTable_) {
        ReleaseAnimationAsset(animation);
    }

    animationIndexBinder_.clear();
    animationTable_.clear();

    currentAnimationIndex_ = 0;
    blendingAnimationData_ = std::nullopt;

    bindModeMeshRendererIndex_ = -1;

    entityHandle_ = EntityHandle();
}

void SkinningAnimationComponent::AddLoad(const std::string& _directory, const std::string& _fileName) {
    if (_directory.empty() || _fileName.empty()) {
        LOG_ERROR("Directory or fileName is empty.");
        return;
    }
    // 既に同じアニメーションが存在する場合は何もしない
    auto itr = animationIndexBinder_.find(_fileName);
    if (itr != animationIndexBinder_.end()) {
        LOG_ERROR("Animation with name '{}' already exists.", _fileName);
        return;
    }
    // 新しいアニメーションを追加
    auto& newAnimation     = animationTable_.emplace_back(AnimationCombo{});
    newAnimation.directory = _directory;
    newAnimation.fileName  = _fileName;

    LoadAnimationAsset(newAnimation);

    animationIndexBinder_[_fileName] = static_cast<int32_t>(animationTable_.size()) - 1;
}

void SkinningAnimationComponent::Play() {
    auto& animation = animationTable_[currentAnimationIndex_];

    if (animation.animationAssetIndex == kInvalidAssetIndex) {
        LoadAnimationAsset(animation);
    }
    animation.animationState.isPlay_ = true;
    animation.animationState.isEnd_  = false;
    animation.currentTime            = 0.0f;
}

void SkinningAnimationComponent::Play(int32_t _index) {
    if (_index < 0 || _index >= static_cast<int32_t>(animationTable_.size())) {
        LOG_ERROR("Invalid animation index: {}", _index);
        return;
    }
    auto& animation = animationTable_[_index];
    if (animation.animationAssetIndex == kInvalidAssetIndex) {
        LoadAnimationAsset(animation);
    }
    animation.animationState.isPlay_ = true;
    animation.prePlay                = false; // 前のアニメーションを停止
    animation.animationState.isEnd_  = false;
    animation.currentTime            = 0.0f;
}

void SkinningAnimationComponent::Play(const std::string& _name) {
    int32_t index = GetAnimationIndex(_name);
    if (index < 0 || index >= static_cast<int32_t>(animationTable_.size())) {
        LOG_ERROR("Invalid animation name: {}", _name);
        return;
    }
    Play(index);
}

void SkinningAnimationComponent::PlayNext(int32_t _index, float _blendTime) {
    if (_index < 0 || _index >= static_cast<int32_t>(animationTable_.size())) {
        LOG_ERROR("Invalid animation index: {}", _index);
        return;
    }

    blendingAnimationData_ = AnimationBlendData{_index, _blendTime, 0.0f};

    auto& nextAnimation = animationTable_[blendingAnimationData_.value().targetAnimationIndex];
    if (nextAnimation.animationAssetIndex == kInvalidAssetIndex) {
        LoadAnimationAsset(nextAnimation);
    }
    nextAnimation.animationState.isPlay_ = true;
    nextAnimation.prePlay                = false; // 前のアニメーションを停止
    nextAnimation.animationState.isEnd_  = false;
    nextAnimation.currentTime            = 0.0f;
}

void SkinningAnimationComponent::PlayNext(const std::string& _name, float _blendTime) {
    int32_t index = GetAnimationIndex(_name);
    if (index < 0 || index >= static_cast<int32_t>(animationTable_.size())) {
        LOG_ERROR("Invalid animation name: {}", _name);
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

    ModelMeshRenderer* meshRenderer = _scene->GetComponent<ModelMeshRenderer>(entityHandle_, bindModeMeshRendererIndex_);
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
            "MeshGroup is null in SkinningAnimationComponent.\n EntityHandle : {}\n",
            uuids::to_string(entityHandle_.uuid));
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
            UAVEntry uavEntry(&skinnedVertexBuffer_[i].buffer, nullptr, uavDesc);
            skinnedVertexBuffer_[i].descriptor            = uavHeap->CreateDescriptor(&uavEntry);
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
