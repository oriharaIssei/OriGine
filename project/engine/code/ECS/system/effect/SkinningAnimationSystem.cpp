#include "SkinningAnimationSystem.h"

#define DELTA_TIME
#include "EngineInclude.h"
/// Engine
#include "model/ModelManager.h"
// directX12Object
#include "directX12/DxCommand.h"
#include "directX12/ShaderManager.h"
// component
#include "component/animation/SkinningAnimationComponent.h"
#include "component/renderer/MeshRenderer.h"

/// lib
#include "logger/Logger.h"

static void ApplyAnimation(Skeleton& _skeleton, AnimationData* _animationData, float _animationTime) {
    for (Joint& joint : _skeleton.joints) {
        auto itr = _animationData->animationNodes_.find(joint.name);
        if (itr == _animationData->animationNodes_.end()) {
            LOG_WARN("Joint {} not found in animation data", joint.name);
            continue;
        }
        const ModelAnimationNode& nodeAnimation = itr->second;
        joint.transform.scale                   = CalculateValue::Linear(nodeAnimation.scale, _animationTime);
        joint.transform.rotate                  = CalculateValue::Linear(nodeAnimation.rotate, _animationTime);
        joint.transform.translate               = CalculateValue::Linear(nodeAnimation.translate, _animationTime);
    }
}

static void ApplyBlendedAnimation(
    Skeleton& _skeleton,
    AnimationData* _animA, float _timeA,
    AnimationData* _animB, float _timeB,
    float blendWeight) {
    for (size_t i = 0; i < _skeleton.joints.size(); ++i) {
        Joint& joint = _skeleton.joints[i];
        auto itA     = _animA->animationNodes_.find(joint.name);
        auto itB     = _animB->animationNodes_.find(joint.name);

        if (itA == _animA->animationNodes_.end() || itB == _animB->animationNodes_.end()) {
            if (itA != _animA->animationNodes_.end()) {
                const ModelAnimationNode& nodeAnimation = itA->second;
                joint.transform.scale                   = CalculateValue::Linear(nodeAnimation.scale, _timeA);
                joint.transform.rotate                  = CalculateValue::Linear(nodeAnimation.rotate, _timeA);
                joint.transform.translate               = CalculateValue::Linear(nodeAnimation.translate, _timeA);
            } else if (itB != _animB->animationNodes_.end()) {
                const ModelAnimationNode& nodeAnimation = itB->second;
                joint.transform.scale                   = CalculateValue::Linear(nodeAnimation.scale, _timeB);
                joint.transform.rotate                  = CalculateValue::Linear(nodeAnimation.rotate, _timeB);
                joint.transform.translate               = CalculateValue::Linear(nodeAnimation.translate, _timeB);
            }
            continue;
        }

        const ModelAnimationNode& nodeA = itA->second;
        const ModelAnimationNode& nodeB = itB->second;

        joint.transform.scale     = Lerp(CalculateValue::Linear(nodeA.scale, _timeA), CalculateValue::Linear(nodeB.scale, _timeB), blendWeight);
        joint.transform.rotate    = Slerp(CalculateValue::Linear(nodeA.rotate, _timeA), CalculateValue::Linear(nodeB.rotate, _timeB), blendWeight);
        joint.transform.translate = Lerp(CalculateValue::Linear(nodeA.translate, _timeA), CalculateValue::Linear(nodeB.translate, _timeB), blendWeight);
    }
}

SkinningAnimationSystem::SkinningAnimationSystem()
    : ISystem(SystemCategory::Effect) {}
SkinningAnimationSystem::~SkinningAnimationSystem() {}

void SkinningAnimationSystem::Initialize() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");
    CreatePSO();
}

void SkinningAnimationSystem::Update() {
    if (entityIDs_.empty()) {
        return;
    }
    ISystem::eraseDeadEntity();

    usingCS_ = false;

    StartCS();
    for (auto& id : entityIDs_) {
        GameEntity* entity = getEntity(id);
        UpdateEntity(entity);
    }
    if (usingCS_) {
        ExecuteCS();
    }
}

void SkinningAnimationSystem::Finalize() {
    if (dxCommand_) {
        dxCommand_->Finalize();
        dxCommand_.reset();
    }
    pso_ = nullptr;
}

void SkinningAnimationSystem::UpdateEntity(GameEntity* _entity) {
    if (!_entity) {
        return;
    }

    int32_t compSize = getComponentArray<SkinningAnimationComponent>()->getComponentSize(_entity);

    const float deltaTime = getMainDeltaTime();
    for (int32_t i = 0; i < compSize; ++i) {
        auto* animationComponent = getComponent<SkinningAnimationComponent>(_entity, i);

        if (!animationComponent) {
            continue;
        }

        int32_t currentAnimationIndex = animationComponent->getCurrentAnimationIndex();
        if (!animationComponent->isPrePlay() && animationComponent->isPlay()) {
            animationComponent->CreateSkinnedVertex(this->getScene());
        }

        animationComponent->setIsPrePlay(animationComponent->isPlay());
        if (!animationComponent->isPlay(currentAnimationIndex)) {
            continue;
        }
        if (!animationComponent->getAnimationData()) {
            continue;
        }

        animationComponent->setIsEnd(currentAnimationIndex, false);

        // アニメーションの更新
        float currentTime = animationComponent->getCurrentTime(currentAnimationIndex);
        currentTime += deltaTime * animationComponent->getPlaybackSpeed(currentAnimationIndex);
        float duration = animationComponent->getDuration(currentAnimationIndex);
        if (currentTime >= duration) {
            if (animationComponent->isLoop(currentAnimationIndex)) {
                currentTime = std::fmod(currentTime, duration);
            } else {
                currentTime = duration;
                animationComponent->setIsEnd(currentAnimationIndex, true);
            }
        }

        animationComponent->setCurrentTime(currentAnimationIndex, currentTime);

        // アニメーションの状態を更新
        auto* modelRenderer = getComponent<ModelMeshRenderer>(_entity, animationComponent->getBindModeMeshRendererIndex());
        if (!modelRenderer) {
            LOG_ERROR("ModelMeshRenderer not found for entity: {}", _entity->getID());
            return;
        }

        auto& clusterDataMap = ModelManager::getInstance()->getModelMeshData(modelRenderer->getDirectory(), modelRenderer->getFileName())->skinClusterDataMap;
        auto& skeleton       = animationComponent->getSkeletonRef();

        // アニメーションが遷移しているかどうか
        if (animationComponent->isTransitioning()) {
            // 遷移時間の 更新
            int32_t nextAnimationIndex = animationComponent->getNextAnimationIndex();
            if (nextAnimationIndex < 0 || nextAnimationIndex >= static_cast<int32_t>(animationComponent->getAnimationTable().size())) {
                LOG_ERROR("Invalid next animation index: {}", nextAnimationIndex);
                continue;
            }

            float transitionCurrentTime = animationComponent->getBlendCurrentTime();
            transitionCurrentTime += deltaTime;

            if (transitionCurrentTime >= animationComponent->getBlendTime()) {
                transitionCurrentTime = animationComponent->getBlendTime();
                animationComponent->endTransition(); // トランジションを終了
            }
            animationComponent->setBlendCurrentTime(transitionCurrentTime);

            // 次のアニメーションの更新
            float nextAnimationCurrentTime = animationComponent->getCurrentTime(nextAnimationIndex);
            nextAnimationCurrentTime += deltaTime * animationComponent->getPlaybackSpeed(nextAnimationIndex);
            float nextDuration = animationComponent->getDuration(nextAnimationIndex);
            if (nextAnimationCurrentTime >= nextDuration) {
                if (animationComponent->isLoop(currentAnimationIndex)) {
                    nextAnimationCurrentTime = std::fmod(nextAnimationCurrentTime, nextDuration);
                } else {
                    nextAnimationCurrentTime = nextDuration;
                    animationComponent->setIsEnd(nextAnimationIndex, true);
                }
            }
            animationComponent->setCurrentTime(nextAnimationIndex, nextAnimationCurrentTime);

            ApplyBlendedAnimation(
                skeleton,
                animationComponent->getAnimationData(currentAnimationIndex).get(),
                currentTime,
                animationComponent->getAnimationData(nextAnimationIndex).get(),
                nextAnimationCurrentTime,
                transitionCurrentTime / animationComponent->getBlendTime());
        } else {
            ApplyAnimation(
                skeleton,
                animationComponent->getAnimationData(currentAnimationIndex).get(),
                currentTime);
        }
        skeleton.Update();

        auto& commandList = dxCommand_->getCommandList();
        auto& meshGroup   = modelRenderer->getMeshGroup();

        int32_t meshSize = static_cast<int32_t>(meshGroup->size());
        for (int32_t meshIdx = 0; meshIdx < meshSize; ++meshIdx) {
            auto& mesh = meshGroup->at(meshIdx);
            // スキニングされた頂点バッファを更新
            auto& skinnedVertexBuffer = animationComponent->getSkinnedVertexBuffer(meshIdx);

            if (!skinnedVertexBuffer.descriptor || !skinnedVertexBuffer.buffer.isValid()) {
                continue; // スキニングされた頂点バッファが無効な場合はスキップ
            }

            auto clusterItr = clusterDataMap.find(mesh.getName());
            if (clusterItr == clusterDataMap.end()) {
                LOG_ERROR("SkinClusterData not found for mesh at index {}", meshIdx);
                continue;
            }
            auto& clusterData = clusterItr->second;
            clusterData.UpdateMatrixPalette(skeleton);

            commandList->SetComputeRootDescriptorTable(
                outputVertexBufferIndex_,
                skinnedVertexBuffer.descriptor->getGpuHandle());
            commandList->SetComputeRootShaderResourceView(
                inputVertexBufferIndex_,
                mesh.getVBView().BufferLocation);
            commandList->SetComputeRootDescriptorTable(
                matrixPaletteBufferIndex_,
                clusterData.skeletonMatrixPaletteBuffer_.getSrv()->getGpuHandle());
            commandList->SetComputeRootDescriptorTable(
                vertexInfluenceBufferIndex_,
                clusterData.vertexInfluencesBuffer_.getSrv()->getGpuHandle());

            clusterData.skinningInfoBuffer_->vertexSize =
                mesh.getVBView().SizeInBytes / mesh.getVBView().StrideInBytes;
            clusterData.skinningInfoBuffer_.ConvertToBuffer();

            commandList->SetComputeRootConstantBufferView(
                gSkinningInformationBufferIndex_,
                clusterData.skinningInfoBuffer_.getResource().getResource()->GetGPUVirtualAddress());

            dxCommand_->ResourceBarrier(
                skinnedVertexBuffer.buffer.getResource(),
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

            UINT dispatchCount = (clusterData.skinningInfoBuffer_->vertexSize + 1023) / 1024;
            commandList->Dispatch(
                dispatchCount, // 1ワークグループあたり1024頂点を処理
                1,
                1); // X方向に分割、YとZは1

            dxCommand_->ResourceBarrier(
                skinnedVertexBuffer.buffer.getResource(),
                D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

            usingCS_ = true;
        }
    }
}

void SkinningAnimationSystem::CreatePSO() {
    constexpr const char* psoKey = "Skinning.CS";

    if (pso_) {
        return; // PSOが既に作成されている場合は何もしない
    }

    ShaderManager* shaderManager = ShaderManager::getInstance();
    DxDevice* dxDevice           = Engine::getInstance()->getDxDevice();

    if (shaderManager->IsRegistertedPipelineStateObj(psoKey)) {
        pso_ = shaderManager->getPipelineStateObj(psoKey);
        return; // PSOが既に登録されている場合はそれを使用
    }

    // PSOが登録されていない場合は新規に作成

    /// ==========================================
    // Shader 読み込み
    /// ==========================================
    shaderManager->LoadShader(psoKey, shaderDirectory, L"cs_6_0");

    ShaderInfo shaderInfo{};
    shaderInfo.csKey = psoKey;

#pragma region "ROOT_PARAMETER"
    D3D12_ROOT_PARAMETER rootParameters[5]{};
    rootParameters[outputVertexBufferIndex_].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[outputVertexBufferIndex_].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    shaderInfo.pushBackRootParameter(rootParameters[outputVertexBufferIndex_]);

    rootParameters[inputVertexBufferIndex_].ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[inputVertexBufferIndex_].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_SRV;
    rootParameters[inputVertexBufferIndex_].Descriptor.ShaderRegister = 0; // t0

    shaderInfo.pushBackRootParameter(rootParameters[inputVertexBufferIndex_]);

    rootParameters[matrixPaletteBufferIndex_].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[matrixPaletteBufferIndex_].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    shaderInfo.pushBackRootParameter(rootParameters[matrixPaletteBufferIndex_]);

    rootParameters[vertexInfluenceBufferIndex_].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[vertexInfluenceBufferIndex_].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    shaderInfo.pushBackRootParameter(rootParameters[vertexInfluenceBufferIndex_]);

    rootParameters[gSkinningInformationBufferIndex_].ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[gSkinningInformationBufferIndex_].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[gSkinningInformationBufferIndex_].Descriptor.ShaderRegister = 0; // b0
    shaderInfo.pushBackRootParameter(rootParameters[gSkinningInformationBufferIndex_]);

    D3D12_DESCRIPTOR_RANGE outputDescriptorRange[1]            = {};
    outputDescriptorRange[0].BaseShaderRegister                = 0; // u0
    outputDescriptorRange[0].NumDescriptors                    = 1;
    outputDescriptorRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    outputDescriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    shaderInfo.setDescriptorRange2Parameter(
        outputDescriptorRange, 1, outputVertexBufferIndex_);

    D3D12_DESCRIPTOR_RANGE matrixPaletteDescriptorRange[1]            = {};
    matrixPaletteDescriptorRange[0].BaseShaderRegister                = 1; // t1
    matrixPaletteDescriptorRange[0].NumDescriptors                    = 1;
    matrixPaletteDescriptorRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    matrixPaletteDescriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    shaderInfo.setDescriptorRange2Parameter(
        matrixPaletteDescriptorRange, 1, matrixPaletteBufferIndex_);

    D3D12_DESCRIPTOR_RANGE vertexInfluenceDescriptorRange[1]            = {};
    vertexInfluenceDescriptorRange[0].BaseShaderRegister                = 2; // t2
    vertexInfluenceDescriptorRange[0].NumDescriptors                    = 1;
    vertexInfluenceDescriptorRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    vertexInfluenceDescriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    shaderInfo.setDescriptorRange2Parameter(
        vertexInfluenceDescriptorRange, 1, vertexInfluenceBufferIndex_);

#pragma endregion

    /// ==========================================
    // PSOの作成
    /// ==========================================
    pso_ = shaderManager->CreatePso(psoKey, shaderInfo, dxDevice->getDevice());
}

void SkinningAnimationSystem::StartCS() {
    if (!pso_) {
        LOG_ERROR("PSO is not created for SkinningAnimationSystem");
        return;
    }

    ID3D12DescriptorHeap* ppHeaps[] = {
        Engine::getInstance()->getSrvHeap()->getHeap().Get()};
    dxCommand_->getCommandList()->SetDescriptorHeaps(1, ppHeaps);

    dxCommand_->getCommandList()->SetPipelineState(pso_->pipelineState.Get());
    dxCommand_->getCommandList()->SetComputeRootSignature(pso_->rootSignature.Get());
}

void SkinningAnimationSystem::ExecuteCS() {
    HRESULT hr;
    DxFence* fence = Engine::getInstance()->getDxFence();

    // コマンドの受付終了 -----------------------------------
    hr = dxCommand_->Close();
    if (FAILED(hr)) {
        LOG_ERROR("Failed to close command list. HRESULT: {}", std::to_string(hr));
        assert(false);
    }
    //----------------------------------------------------

    ///===============================================================
    /// コマンドリストの実行
    ///===============================================================
    dxCommand_->ExecuteCommand();
    ///===============================================================

    ///===============================================================
    /// コマンドリストの実行を待つ
    ///===============================================================
    fence->Signal(dxCommand_->getCommandQueue());
    fence->WaitForFence();
    ///===============================================================

    ///===============================================================
    /// リセット
    ///===============================================================
    dxCommand_->CommandReset();
    ///===============================================================
}
