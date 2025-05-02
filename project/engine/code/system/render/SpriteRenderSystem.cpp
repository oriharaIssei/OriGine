#include "SpriteRenderSystem.h"

/// engine
#include "Engine.h"
// module
#include "camera/CameraManager.h"
#include "texture/TextureManager.h"
#include "winApp/WinApp.h"
// ECS
#include "component/renderer/Sprite.h"
#include "ECS/ECSManager.h"

void SpriteRenderSystem::Initialize() {
    dxCommand_ = std::make_unique<DxCommand>();
    dxCommand_->Initialize("main", "main");

    CreatePso();

    // ViewPortMatの計算
    WinApp* window = Engine::getInstance()->getWinApp();
    viewPortMat_   = MakeMatrix::Orthographic(0, 0, (float)window->getWidth(), (float)window->getHeight(), 0.0f, 100.0f);
}

void SpriteRenderSystem::Update() {
    eraseDeadEntity();

    if (entities_.empty()) {
        return;
    }

    StartRender();
    std::vector<SpriteRenderer*> renderers;
    for (auto& entity : entities_) {
        auto* entityRenderers = getComponents<SpriteRenderer>(entity);
        for (auto& renderer : *entityRenderers) {
            if (!renderer.isRender()) {
                continue;
            }
            ///==============================
            /// ConstBufferの更新
            ///==============================
            renderer.Update(viewPortMat_);
            renderers.push_back(&renderer);
        }
    }
    std::sort(renderers.begin(), renderers.end(), [](SpriteRenderer* a, SpriteRenderer* b) {
        return a->getRenderPriority() < b->getRenderPriority();
    });

    auto commandList = dxCommand_->getCommandList();
    for (auto& renderer : renderers) {
        // ============================= テクスチャの設定 ============================= //
        ID3D12DescriptorHeap* ppHeaps[] = {DxHeap::getInstance()->getSrvHeap()};
        commandList->SetDescriptorHeaps(1, ppHeaps);
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

void SpriteRenderSystem::Finalize() {
    dxCommand_->Finalize();
}

void SpriteRenderSystem::CreatePso() {

    ShaderManager* shaderManager = ShaderManager::getInstance();

    shaderManager->LoadShader("Sprite.VS");
    shaderManager->LoadShader("Sprite.PS", shaderDirectory, L"ps_6_0");

    ShaderInformation shaderInfo{};
    shaderInfo.vsKey = "Sprite.VS";
    shaderInfo.psKey = "Sprite.PS";

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
    std::string psoKeys[kBlendNum] = {
        "Sprite_None",
        "Sprite_Alpha",
        "Sprite_Add",
        "Sprite_Sub",
        "Sprite_Multiply",
        "Sprite_Screen"};

    for (size_t i = 0; i < kBlendNum; i++) {
        shaderInfo.blendMode_ = static_cast<BlendMode>(i);

        pso_[shaderInfo.blendMode_] = shaderManager->CreatePso(psoKeys[i], shaderInfo, Engine::getInstance()->getDxDevice()->getDevice());
    }
}

void SpriteRenderSystem::StartRender() {
    auto commandList     = dxCommand_->getCommandList();
    auto currentBlendPso = pso_[currentBlend_];
    commandList->SetGraphicsRootSignature(currentBlendPso->rootSignature.Get());
    commandList->SetPipelineState(currentBlendPso->pipelineState.Get());
    dxCommand_->getCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void SpriteRenderSystem::UpdateEntity(GameEntity* /*_entity*/) {}
