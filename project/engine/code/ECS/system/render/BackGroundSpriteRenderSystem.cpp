#include "BackGroundSpriteRenderSystem.h"

/// engine
#include "Engine.h"
// directX12Object
#include "directX12/DxDevice.h"
// module
#include "camera/CameraManager.h"
#include "texture/TextureManager.h"
#include "winApp/WinApp.h"
// ECS
#include "component/renderer/Sprite.h"

void BackGroundSpriteRenderSystem::Initialize() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");

    CreatePso();

    // ViewPortMatの計算
    WinApp* window = Engine::getInstance()->getWinApp();
    viewPortMat_   = MakeMatrix::Orthographic(0, 0, (float)window->getWidth(), (float)window->getHeight(), 0.0f, 100.0f);
}

void BackGroundSpriteRenderSystem::Update() {
    eraseDeadEntity();

    if (entityIDs_.empty()) {
        return;
    }

    // 前フレームの描画対象をクリア
    renderersByBlend_.clear();

    bool isRendering = false;
    for (auto& id : entityIDs_) {
        Entity* entity    = getEntity(id);
        auto* entityRenderers = getComponents<SpriteRenderer>(entity);
        for (auto& renderer : *entityRenderers) {
            if (!renderer.isRender()) {
                continue;
            }
            ///==============================
            /// ConstBufferの更新
            ///==============================
            renderer.Update(viewPortMat_);
            renderersByBlend_[renderer.getCurrentBlend()].push_back(&renderer);

            isRendering = true;
        }
    }

    for (auto& [blend, renderers] : renderersByBlend_) {
        std::sort(renderers.begin(), renderers.end(), [](SpriteRenderer* a, SpriteRenderer* b) {
            return a->getRenderPriority() < b->getRenderPriority();
        });
    }

    // skip
    if (!isRendering) {
        return;
    }

    for (size_t i = 0; i < kBlendNum; ++i) {
        BlendMode blend = static_cast<BlendMode>(i);
        RenderingBy(blend);
    }
}

void BackGroundSpriteRenderSystem::RenderingBy(BlendMode blendMode) {
    if (renderersByBlend_[blendMode].empty()) {
        return;
    }

    auto commandList = dxCommand_->getCommandList();
    // PSOの設定
    commandList->SetGraphicsRootSignature(pso_[blendMode]->rootSignature.Get());
    commandList->SetPipelineState(pso_[blendMode]->pipelineState.Get());

    StartRender();

    // 描画
    for (auto& renderer : renderersByBlend_[blendMode]) {
        // ============================= テクスチャの設定 ============================= //
        commandList->SetGraphicsRootDescriptorTable(
            1,
            TextureManager::getDescriptorGpuHandle(renderer->getTextureNumber()));

        SpriteMesh& mesh = renderer->getMeshGroup()->at(0);
        commandList->IASetVertexBuffers(0, 1, &mesh.getVBView());
        commandList->IASetIndexBuffer(&mesh.getIBView());

        renderer->getSpriteBuff().SetForRootParameter(commandList, 0);

        commandList->DrawIndexedInstanced(
            6, 1, 0, 0, 0);

        commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
    }
}

void BackGroundSpriteRenderSystem::Finalize() {
    dxCommand_->Finalize();
}

void BackGroundSpriteRenderSystem::CreatePso() {

    ShaderManager* shaderManager = ShaderManager::getInstance();

    // 登録されているかどうかをチェック
    if (shaderManager->IsRegisteredPipelineStateObj("BackGroundSprite_" + blendModeStr[0])) {
        for (size_t i = 0; i < kBlendNum; ++i) {
            BlendMode blend = static_cast<BlendMode>(i);
            if (pso_[blend]) {
                continue;
            }
            pso_[blend] = shaderManager->getPipelineStateObj("BackGroundSprite_" + blendModeStr[i]);
        }
        return;
    }

    shaderManager->LoadShader("Sprite.VS");
    shaderManager->LoadShader("Sprite.PS", shaderDirectory, L"ps_6_0");

    ShaderInformation shaderInfo{};
    shaderInfo.vsKey = "Sprite.VS";
    shaderInfo.psKey = "Sprite.PS";

    ///=================================================
    /// Depth
    ///=================================================
    shaderInfo.customDepthStencilDesc().DepthEnable    = true;
    shaderInfo.customDepthStencilDesc().DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // 書き込まない z = -1だから
    shaderInfo.customDepthStencilDesc().DepthFunc      = D3D12_COMPARISON_FUNC_LESS_EQUAL;

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
    D3D12_ROOT_PARAMETER rootParameters[2]      = {};
    rootParameters[0].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[0].Descriptor.ShaderRegister = 0;
    shaderInfo.pushBackRootParameter(rootParameters[0]);

    D3D12_DESCRIPTOR_RANGE descriptorRange = {};
    descriptorRange.BaseShaderRegister     = 0;
    descriptorRange.NumDescriptors         = 1;
    // SRV を扱うように設定
    descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    // offset を自動計算するように 設定
    descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    // DescriptorTable を使う
    rootParameters[1].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    size_t rootIndex                   = shaderInfo.pushBackRootParameter(rootParameters[1]);
    shaderInfo.setDescriptorRange2Parameter(&descriptorRange, 1, rootIndex);

    ///================================================
    /// InputElement の設定
    ///================================================
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
    inputElementDescs[0].SemanticName             = "POSITION"; /*Semantics*/
    inputElementDescs[0].SemanticIndex            = 0; /*Semanticsの横に書いてある数字(今回はPOSITION0なので 0 )*/
    inputElementDescs[0].Format                   = DXGI_FORMAT_R32G32B32A32_FLOAT; // float 4
    inputElementDescs[0].AlignedByteOffset        = D3D12_APPEND_ALIGNED_ELEMENT;
    shaderInfo.pushBackInputElementDesc(inputElementDescs[0]);

    inputElementDescs[1].SemanticName      = "TEXCOORD"; /*Semantics*/
    inputElementDescs[1].SemanticIndex     = 0;
    inputElementDescs[1].Format            = DXGI_FORMAT_R32G32_FLOAT;
    inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    shaderInfo.pushBackInputElementDesc(inputElementDescs[1]);

    ///================================================
    /// RasterizerDesc の設定
    ///================================================
    shaderInfo.changeCullMode(D3D12_CULL_MODE_NONE);

    ///================================================
    /// 生成
    ///================================================
    for (size_t i = 0; i < kBlendNum; i++) {
        shaderInfo.blendMode_ = static_cast<BlendMode>(i);

        pso_[shaderInfo.blendMode_] = shaderManager->CreatePso("BackGroundSprite_" + blendModeStr[i], shaderInfo, Engine::getInstance()->getDxDevice()->getDevice());
    }
}

void BackGroundSpriteRenderSystem::StartRender() {
    auto commandList = dxCommand_->getCommandList();

    ID3D12DescriptorHeap* ppHeaps[] = {Engine::getInstance()->getSrvHeap()->getHeap().Get()};
    commandList->SetDescriptorHeaps(1, ppHeaps);
    dxCommand_->getCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void BackGroundSpriteRenderSystem::UpdateEntity(Entity* /*_entity*/) {}
