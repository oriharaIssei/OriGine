#include "Sprite.h"

/// algorithm
#include <algorithm>

/// engine
#include "Engine.h"
// directX12Object
#include "directX12/DxFunctionHelper.h"
#include <directX12/ShaderCompiler.h>
// assets
#include "texture/TextureManager.h"
// lib
#include "component/renderer/RenderManager.h"
#include "logger/Logger.h"

#ifdef _DEBUG
#include "imgui/imgui.h"
#include "Sprite.h"
#endif // _DEBUG

void SpriteRenderer::Init() {
    // buffer作成
    spriteBuff_.CreateBuffer(Engine::getInstance()->getDxDevice()->getDevice());

    // メッシュの初期化
    meshGroup_ = std::make_shared<std::vector<SpriteMesh>>();
    meshGroup_->push_back(SpriteMesh());

    SpriteMesh& mesh = meshGroup_->at(0);
    mesh.Init(4, 6);
    // indexData
    mesh.indexData[0] = 0;
    mesh.indexData[1] = 1;
    mesh.indexData[2] = 2;
    mesh.indexData[3] = 1;
    mesh.indexData[4] = 3;
    mesh.indexData[5] = 2;

    // テクスチャの読み込みとサイズの適応
    if (!texturePath_.empty()) {
        textureNumber_ = TextureManager::LoadTexture(texturePath_, [this](uint32_t index) {
            const DirectX::TexMetadata& texData = TextureManager::getTexMetadata(index);
            if (textureSize_.lengthSq() == 0.0f) {
                textureSize_ = {static_cast<float>(texData.width), static_cast<float>(texData.height)};
            }
            if (spriteBuff_->scale_.lengthSq() == 0.0f) {
                spriteBuff_->scale_ = textureSize_;
            }
            SpriteRendererController* controller = RenderManager::getInstance()->getRendererController<SpriteRendererController>();
            spriteBuff_->Update(controller->getViewPortMat());
        });
    }
}

void SpriteRenderer::Update() {
    //-------------------------------- ConstBufferの更新 --------------------------------//
    {
        SpriteRendererController* controller = RenderManager::getInstance()->getRendererController<SpriteRendererController>();
        spriteBuff_->Update(controller->getViewPortMat());

        spriteBuff_.ConvertToBuffer();
    }
    //-------------------------------- メッシュの更新 --------------------------------//
    float left   = -anchorPoint_[X];
    float right  = 1.0f - anchorPoint_[X];
    float top    = -anchorPoint_[Y];
    float bottom = 1.0f - anchorPoint_[Y];

    if (isFlipX_) {
        left  = -left;
        right = -right;
    }
    if (isFlipY_) {
        top    = -top;
        bottom = -bottom;
    }

    SpriteMesh& mesh     = meshGroup_->at(0);
    mesh.vertData[0].pos = {left, bottom, 0.0f, 1.0f};
    mesh.vertData[1].pos = {left, top, 0.0f, 1.0f};
    mesh.vertData[2].pos = {right, bottom, 0.0f, 1.0f};
    mesh.vertData[3].pos = {right, top, 0.0f, 1.0f};

    float texLeft   = textureLeftTop_[X] / textureSize_[X];
    float texRight  = (textureLeftTop_[X] + textureSize_[X]) / textureSize_[X];
    float texTop    = textureLeftTop_[Y] / textureSize_[Y];
    float texBottom = (textureLeftTop_[Y] + textureSize_[Y]) / textureSize_[Y];

    mesh.vertData[0].texcoord = {texLeft, texBottom};
    mesh.vertData[1].texcoord = {texLeft, texTop};
    mesh.vertData[2].texcoord = {texRight, texBottom};
    mesh.vertData[3].texcoord = {texRight, texTop};
}

void SpriteRenderer::Render() {
    // commandListの取得
    IRendererComponentController* controller = RenderManager::getInstance()->getRendererController<SpriteRendererController>();
    auto* commandList                        = controller->getDxCommand()->getCommandList();

    SpriteMesh& mesh = meshGroup_->at(0);
    commandList->IASetVertexBuffers(0, 1, &mesh.vbView);
    commandList->IASetIndexBuffer(&mesh.ibView);

    ID3D12DescriptorHeap* ppHeaps[] = {DxHeap::getInstance()->getSrvHeap()};
    commandList->SetDescriptorHeaps(1, ppHeaps);
    commandList->SetGraphicsRootDescriptorTable(
        1,
        TextureManager::getDescriptorGpuHandle(textureNumber_));

    spriteBuff_.SetForRootParameter(commandList, 0);

    commandList->DrawIndexedInstanced(
        6, 1, 0, 0, 0);
}

void SpriteRendererController::Init() {
    RendererComponentController<SpriteRenderer>::Init();

    WinApp* window = Engine::getInstance()->getWinApp();
    viewPortMat_   = MakeMatrix::Orthographic(0, 0, (float)window->getWidth(), (float)window->getHeight(), 0.0f, 100.0f);
}

void SpriteRendererController::Update() {
    // renderSchedule_ から nullptr または外部参照が無い(renderer.use_count() == 1)レンダラーを削除
    std::erase_if(renderSchedule_, [](std::shared_ptr<SpriteRenderer>& renderer) {
        bool isExists = (!renderer) || (renderer.use_count() >= 1);
        if (!isExists) {
            // nullptr ではないなら 開放してから削除
            if (renderer) {
                renderer.get()->Finalize();
                renderer.reset();
            }
        }
        return !isExists;
    });

    // 並び替え
    std::sort(renderSchedule_.begin(), renderSchedule_.end(),
        [](const std::shared_ptr<SpriteRenderer> a, const std::shared_ptr<SpriteRenderer> b) {
            return a->getRenderingNum() < b->getRenderingNum();
        });

    for (std::shared_ptr<SpriteRenderer>& renderer : renderSchedule_) {
        renderer->Update();
    }
}

void SpriteRendererController::StartRender() {
    auto commandList     = dxCommand_->getCommandList();
    auto currentBlendPso = pso_[currentBlend_];
    commandList->SetGraphicsRootSignature(currentBlendPso->rootSignature.Get());
    commandList->SetPipelineState(currentBlendPso->pipelineState.Get());
    dxCommand_->getCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void SpriteRendererController::CreatePso() {

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
    shaderInfo.ChangeCullMode(D3D12_CULL_MODE_NONE);

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
