#include "DissolveEffect.h"

/// engine
#include "Engine.h"
#include "texture/TextureManager.h"

// component
#include "component/effect/post/DissolveEffectParam.h"

// directX12
#include "directX12/DxDevice.h"
#include "directX12/RenderTexture.h"

void DissolveEffect::Initialize() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");
    CreatePSO();
}

void DissolveEffect::Update() {
    auto* sceneView = this->getScene()->getSceneView();

    eraseDeadEntity();

    if (entityIDs_.empty()) {
        return;
    }

    // 実行する必要があるかチェック
    bool allIsUnactive = true;

    for (auto& entityId : entityIDs_) {
        Entity* entity = getEntity(entityId);
        auto* compVec      = getComponents<DissolveEffectParam>(entity);
        if (!compVec || compVec->empty()) {
            continue;
        }
        for (auto& comp : *compVec) {
            if (comp.isActive()) {
                allIsUnactive = false;
                break; // 1つでもアクティブなコンポーネントがあればループを抜ける
            }
        }
        if (!allIsUnactive) {
            break; // 全てのコンポーネントが非アクティブなら何もしない
        }
    }

    if (allIsUnactive) {
        return; // 全てのコンポーネントが非アクティブなら何もしない
    }

    RenderStart();

    sceneView->PreDraw();
    for (auto& id : entityIDs_) {
        auto* entity = getEntity(id);
        UpdateEntity(entity);
    }

    sceneView->PostDraw();
}

void DissolveEffect::UpdateEntity(Entity* _entity) {
    auto effectParams = getComponents<DissolveEffectParam>(_entity);

    if (!effectParams) {
        return; // コンポーネントがない場合は何もしない
    }
    auto& commandList     = dxCommand_->getCommandList();
    const auto& sceneView = getScene()->getSceneView();

    for (auto& param : *effectParams) {
        if (!param.isActive()) {
            continue;
        }
        D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = TextureManager::getDescriptorGpuHandle(param.getTextureIndex());

        auto& paramBuff = param.getDissolveBuffer();
        paramBuff.ConvertToBuffer();

        int32_t materialIndex = param.getMaterialIndex();
        auto& materialBuff    = param.getMaterialBuffer();
        if (materialIndex >= 0) {
            Material* material = getComponent<Material>(_entity, materialIndex);
            material->UpdateUvMatrix();

            materialBuff.ConvertToBuffer(ColorAndUvTransform(material->color_, material->uvTransform_));
            if (material->hasCustomTexture()) {
                srvHandle = material->getCustomTexture()->srv_->getGpuHandle();
            }
        }

        ID3D12DescriptorHeap* ppHeaps[] = {Engine::getInstance()->getSrvHeap()->getHeap().Get()};
        commandList->SetDescriptorHeaps(1, ppHeaps);

        commandList->SetGraphicsRootDescriptorTable(1,
            srvHandle);

        paramBuff.SetForRootParameter(dxCommand_->getCommandList(), 2);

        materialBuff.SetForRootParameter(commandList, 3);

        Render(sceneView->getBackBufferSrvHandle());
    }
}

void DissolveEffect::Finalize() {
    if (dxCommand_) {
        dxCommand_.reset();
    }
    pso_ = nullptr;
}

void DissolveEffect::EffectEntity(RenderTexture* _output, Entity* _entity) {
    if (!_output) {
        return;
    }
    RenderStart();
    _output->PreDraw();

    auto effectParams = getComponents<DissolveEffectParam>(_entity);

    if (!effectParams) {
        return; // コンポーネントがない場合は何もしない
    }
    auto& commandList = dxCommand_->getCommandList();

    for (auto& param : *effectParams) {
        if (!param.isActive()) {
            continue;
        }
        D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = TextureManager::getDescriptorGpuHandle(param.getTextureIndex());

        int32_t materialIndex = param.getMaterialIndex();
        auto& materialBuff    = param.getMaterialBuffer();
        if (materialIndex >= 0) {
            Material* material = getComponent<Material>(_entity, materialIndex);
            material->UpdateUvMatrix();

            materialBuff.ConvertToBuffer(ColorAndUvTransform(material->color_, material->uvTransform_));

            if (material->hasCustomTexture()) {
                srvHandle = material->getCustomTexture()->srv_->getGpuHandle();
            }
        }

        ID3D12DescriptorHeap* ppHeaps[] = {Engine::getInstance()->getSrvHeap()->getHeap().Get()};
        commandList->SetDescriptorHeaps(1, ppHeaps);

        commandList->SetGraphicsRootDescriptorTable(1,srvHandle);

        param.getDissolveBuffer().ConvertToBuffer();
        param.getDissolveBuffer().SetForRootParameter(dxCommand_->getCommandList(), 2);

        materialBuff.SetForRootParameter(commandList, 3);

        Render(_output->getBackBufferSrvHandle());
    }

    _output->PostDraw();
}

void DissolveEffect::CreatePSO() {
    ShaderManager* shaderManager = ShaderManager::getInstance();
    shaderManager->LoadShader("FullScreen.VS");
    shaderManager->LoadShader("Dissolve.PS", shaderDirectory, L"ps_6_0");
    ShaderInformation shaderInfo{};
    shaderInfo.vsKey = "FullScreen.VS";
    shaderInfo.psKey = "Dissolve.PS";

    ///================================================
    /// Sampler の設定
    ///================================================
    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter                    = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイナリニアフィルタ
    // 0 ~ 1 の間をリピート
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

    sampler.ComparisonFunc   = D3D12_COMPARISON_FUNC_NEVER;
    sampler.MinLOD           = 0;
    sampler.MaxLOD           = D3D12_FLOAT32_MAX;
    sampler.ShaderRegister   = 0;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    shaderInfo.pushBackSamplerDesc(sampler);

    ///================================================
    /// RootParameter の設定
    ///================================================
    // Texture だけ
    D3D12_ROOT_PARAMETER rootParameter[4]    = {};
    D3D12_DESCRIPTOR_RANGE sceneViewRange[1] = {};
    sceneViewRange[0].BaseShaderRegister     = 0;
    sceneViewRange[0].NumDescriptors         = 1;
    // SRV を扱うように設定
    sceneViewRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    // offset を自動計算するように 設定
    sceneViewRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // DescriptorTable を使う
    rootParameter[0].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    size_t sceneViewParamIdx          = shaderInfo.pushBackRootParameter(rootParameter[0]);
    shaderInfo.setDescriptorRange2Parameter(sceneViewRange, 1, sceneViewParamIdx);

    D3D12_DESCRIPTOR_RANGE effectTexRange[1] = {};
    effectTexRange[0].BaseShaderRegister     = 1;
    effectTexRange[0].NumDescriptors         = 1;
    // SRV を扱うように設定
    effectTexRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    // offset を自動計算するように 設定
    effectTexRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // DescriptorTable を使う
    rootParameter[1].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    size_t effectTexParamIdx          = shaderInfo.pushBackRootParameter(rootParameter[1]);
    shaderInfo.setDescriptorRange2Parameter(effectTexRange, 1, effectTexParamIdx);

    rootParameter[2].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    shaderInfo.pushBackRootParameter(rootParameter[2]);

    rootParameter[3].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameter[3].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameter[3].Descriptor.ShaderRegister = 1;
    shaderInfo.pushBackRootParameter(rootParameter[3]);

    ///================================================
    /// InputElement の設定
    ///================================================

    // 特に使わない

    ///================================================
    /// depthStencil の設定
    ///================================================
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = false;
    shaderInfo.setDepthStencilDesc(depthStencilDesc);

    pso_ = shaderManager->CreatePso("DissolveEffect", shaderInfo, Engine::getInstance()->getDxDevice()->getDevice());
}

void DissolveEffect::RenderStart() {
    auto& commandList = dxCommand_->getCommandList();

    /// ================================================
    /// pso set
    /// ================================================
    commandList->SetPipelineState(pso_->pipelineState.Get());
    commandList->SetGraphicsRootSignature(pso_->rootSignature.Get());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void DissolveEffect::Render(D3D12_GPU_DESCRIPTOR_HANDLE _viewHandle) {
    auto& commandList = dxCommand_->getCommandList();

    /// ================================================
    /// Viewport の設定
    /// ================================================

    commandList->SetGraphicsRootDescriptorTable(0, _viewHandle);

    commandList->DrawInstanced(6, 1, 0, 0);
}
