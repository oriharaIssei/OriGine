#include "SpriteRenderSystem.h"

/// engine
#include "Engine.h"
// directX12Object
// module
#include "camera/CameraManager.h"
#include "texture/TextureManager.h"
#include "winApp/WinApp.h"
/// ECS
// component
#include "component/renderer/Sprite.h"

using namespace OriGine;

SpriteRenderSystem::SpriteRenderSystem() : BaseRenderSystem() {}
SpriteRenderSystem::~SpriteRenderSystem() {}

void SpriteRenderSystem::Initialize() {
    BaseRenderSystem::Initialize();

    // ViewPortMatの計算
    WinApp* window = Engine::GetInstance()->GetWinApp();
    viewPortMat_   = MakeMatrix4x4::Orthographic(0, 0, (float)window->GetWidth(), (float)window->GetHeight(), 0.0f, 100.0f);
}

void SpriteRenderSystem::Rendering() {

    ///=========================================================
    // OrthographicMat の再計算(Resizeされる可能性)
    ///=========================================================
    WinApp* window = Engine::GetInstance()->GetWinApp();
    viewPortMat_   = MakeMatrix4x4::Orthographic(0, 0, (float)window->GetWidth(), (float)window->GetHeight(), 0.0f, 100.0f);

    ///=========================================================
    // Priorityが低い順に
    ///=========================================================
    std::sort(
        renderers_.begin(),
        renderers_.end(),
        [](SpriteRenderer* a, SpriteRenderer* b) {
            return a->GetRenderPriority() < b->GetRenderPriority();
        });

    ///=========================================================
    // 描画
    ///=========================================================
    auto& commandList = dxCommand_->GetCommandList();

    // 描画開始処理
    StartRender();

    // 初期ブレンドモード
    BlendMode currentBlendMode = BlendMode::Normal;
    int32_t blendIndex         = static_cast<int32_t>(currentBlendMode);
    commandList->SetGraphicsRootSignature(psoByBlendMode_[blendIndex]->rootSignature.Get());
    commandList->SetPipelineState(psoByBlendMode_[blendIndex]->pipelineState.Get());

    for (auto& renderer : renderers_) {
        // BlendModeごとにPSOを切り替え
        if (currentBlendMode != renderer->GetCurrentBlend()) {
            currentBlendMode = renderer->GetCurrentBlend();
            blendIndex       = static_cast<int32_t>(currentBlendMode);
            commandList->SetGraphicsRootSignature(psoByBlendMode_[blendIndex]->rootSignature.Get());
            commandList->SetPipelineState(psoByBlendMode_[blendIndex]->pipelineState.Get());
        }

        // バッファ更新
        renderer->UpdateBuffer(viewPortMat_);

        // テクスチャの設定
        commandList->SetGraphicsRootDescriptorTable(
            1,
            TextureManager::GetDescriptorGpuHandle(renderer->GetTextureNumber()));

        // meshの設定・描画
        SpriteMesh& mesh = renderer->GetMeshGroup()->at(0);
        commandList->IASetVertexBuffers(0, 1, &mesh.GetVBView());
        commandList->IASetIndexBuffer(&mesh.GetIBView());

        renderer->GetSpriteBuff().SetForRootParameter(commandList, 0);

        // 描画コマンド
        commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
    }

    renderers_.clear();
}

void SpriteRenderSystem::DispatchRenderer(Entity* _entity) {
    std::vector<SpriteRenderer>* renderers = GetComponents<SpriteRenderer>(_entity);

    if (!renderers) {
        return;
    }
    for (auto& renderer : *renderers) {
        if (!renderer.IsRender()) {
            continue;
        }
        renderers_.push_back(&renderer);
    }
}

bool SpriteRenderSystem::ShouldSkipRender() const {
    return renderers_.empty();
}

void SpriteRenderSystem::Finalize() {
    BaseRenderSystem::Finalize();
}

void SpriteRenderSystem::CreatePSO() {

    ShaderManager* shaderManager = ShaderManager::GetInstance();

    // 登録されているかどうかをチェック
    if (shaderManager->IsRegisteredPipelineStateObj("Sprite_" + kBlendModeStr[0])) {
        for (size_t i = 0; i < kBlendNum; ++i) {
            if (psoByBlendMode_[i]) {
                continue;
            }
            psoByBlendMode_[i] = shaderManager->GetPipelineStateObj("Sprite_" + kBlendModeStr[i]);
        }
        return;
    }

    shaderManager->LoadShader("Sprite.VS");
    shaderManager->LoadShader("Sprite.PS", kShaderDirectory, L"ps_6_0");

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
    shaderInfo.SetDescriptorRange2Parameter(&descriptorRange, 1, rootIndex);

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

        psoByBlendMode_[i] = shaderManager->CreatePso("Sprite_" + kBlendModeStr[i], shaderInfo, Engine::GetInstance()->GetDxDevice()->device_);
    }
}

void SpriteRenderSystem::StartRender() {
    auto& commandList = dxCommand_->GetCommandList();
    dxCommand_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    dxCommand_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D12DescriptorHeap* ppHeaps[] = {Engine::GetInstance()->GetSrvHeap()->GetHeap().Get()};
    commandList->SetDescriptorHeaps(1, ppHeaps);
}
