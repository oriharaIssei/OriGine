#include "directX12/ShaderManager.h"

/// api
#include <Windows.h>

/// assert (log)
#include "logger/Logger.h"
#include <cassert>

/// util
#include "util/ConvertString.h"

void ShaderManager::Initialize() {
    shaderCompiler_ = std::make_unique<ShaderCompiler>();
    shaderCompiler_->Initialize();
}

void ShaderManager::Finalize() {
    for (auto& pso : psoMap_) {
        pso.second->Finalize();
        pso.second.reset();
    }
    for (auto& blob : shaderBlobMap_) {
        blob.second.Reset();
    }
    shaderCompiler_->Finalize();
}

PipelineStateObj* ShaderManager::CreatePso(const std::string& key,
    const ShaderInformation& shaderInfo,
    Microsoft::WRL::ComPtr<ID3D12Device> device) {
    std::unique_ptr<PipelineStateObj> pso;
    pso = std::make_unique<PipelineStateObj>();
    HRESULT result;

    ///=================================================
    /// RootSignature
    ///=================================================
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {
        .NumParameters     = static_cast<uint32_t>(shaderInfo.rootParameters_.size()),
        .pParameters       = shaderInfo.rootParameters_.data(),
        .NumStaticSamplers = static_cast<uint32_t>(shaderInfo.samplerDescs_.size()),
        .pStaticSamplers   = shaderInfo.samplerDescs_.data(),
        .Flags             = shaderInfo.rootSignatureFlag,
    };

    // シリアライズしてバイナリにする
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob     = nullptr;

    result = D3D12SerializeRootSignature(
        &rootSignatureDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &signatureBlob,
        &errorBlob);

    if (FAILED(result)) {
        std::string errMsg(
            static_cast<const char*>(errorBlob->GetBufferPointer()),
            errorBlob->GetBufferSize());
        LOG_ERROR("D3D12SerializeRootSignature failed: {}", errMsg);
        assert(false);
    }

    device->CreateRootSignature(
        0,
        signatureBlob->GetBufferPointer(),
        signatureBlob->GetBufferSize(),
        IID_PPV_ARGS(&pso->rootSignature));

    ///=================================================
    /// InputLayout
    ///=================================================
    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{
        .pInputElementDescs = shaderInfo.elementDescs_.data(),
        .NumElements        = static_cast<uint32_t>(shaderInfo.elementDescs_.size()),
    };

    ///=================================================
    /// BlendDesc 初期化
    ///=================================================
    D3D12_BLEND_DESC blendDesc{};
    blendDesc.RenderTarget[0] = {}; // RenderTarget[0] を初期化

    switch (shaderInfo.blendMode_) {
    case BlendMode::None:
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        break;
    case BlendMode::Normal:
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        blendDesc.AlphaToCoverageEnable                 = false;
        blendDesc.IndependentBlendEnable                = false;
        blendDesc.RenderTarget[0].BlendEnable           = true;
        blendDesc.RenderTarget[0].LogicOpEnable         = false;
        blendDesc.RenderTarget[0].SrcBlend              = D3D12_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend             = D3D12_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp               = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha         = D3D12_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha        = D3D12_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOpAlpha          = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].LogicOp               = D3D12_LOGIC_OP_NOOP;
        break;
    case BlendMode::Add:
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        blendDesc.AlphaToCoverageEnable                 = false;
        blendDesc.IndependentBlendEnable                = false;
        blendDesc.RenderTarget[0].BlendEnable           = true;
        blendDesc.RenderTarget[0].LogicOpEnable         = false;
        blendDesc.RenderTarget[0].SrcBlend              = D3D12_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp               = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].DestBlend             = D3D12_BLEND_ONE;
        blendDesc.RenderTarget[0].SrcBlendAlpha         = D3D12_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha        = D3D12_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOpAlpha          = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].LogicOp               = D3D12_LOGIC_OP_NOOP;
        break;
    case BlendMode::Sub:
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        blendDesc.AlphaToCoverageEnable                 = false;
        blendDesc.IndependentBlendEnable                = false;
        blendDesc.RenderTarget[0].BlendEnable           = true;
        blendDesc.RenderTarget[0].LogicOpEnable         = false;
        blendDesc.RenderTarget[0].SrcBlend              = D3D12_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp               = D3D12_BLEND_OP_REV_SUBTRACT;
        blendDesc.RenderTarget[0].DestBlend             = D3D12_BLEND_ONE;
        blendDesc.RenderTarget[0].SrcBlendAlpha         = D3D12_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha        = D3D12_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOpAlpha          = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].LogicOp               = D3D12_LOGIC_OP_NOOP;
        break;
    case BlendMode::Multiply:
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        blendDesc.AlphaToCoverageEnable                 = false;
        blendDesc.IndependentBlendEnable                = false;
        blendDesc.RenderTarget[0].BlendEnable           = true;
        blendDesc.RenderTarget[0].LogicOpEnable         = false;
        blendDesc.RenderTarget[0].SrcBlend              = D3D12_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOp               = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].DestBlend             = D3D12_BLEND_SRC_COLOR;
        blendDesc.RenderTarget[0].SrcBlendAlpha         = D3D12_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha        = D3D12_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOpAlpha          = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].LogicOp               = D3D12_LOGIC_OP_NOOP;
        break;
    case BlendMode::Screen:
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        blendDesc.AlphaToCoverageEnable                 = false;
        blendDesc.IndependentBlendEnable                = false;
        blendDesc.RenderTarget[0].BlendEnable           = true;
        blendDesc.RenderTarget[0].LogicOpEnable         = false;
        blendDesc.RenderTarget[0].SrcBlend              = D3D12_BLEND_INV_DEST_COLOR;
        blendDesc.RenderTarget[0].BlendOp               = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].DestBlend             = D3D12_BLEND_ONE;
        blendDesc.RenderTarget[0].SrcBlendAlpha         = D3D12_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha        = D3D12_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOpAlpha          = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].LogicOp               = D3D12_LOGIC_OP_NOOP;
        break;
    default:
        break;
    }

    ///=================================================
    /// GRAPHICS_PIPELINE_STATE_DESC 初期化
    ///=================================================
    // cs か graphics pipeline state かを判定
    if (!shaderInfo.csKey.empty()) {
        D3D12_COMPUTE_PIPELINE_STATE_DESC computeStateDesc{};
        computeStateDesc.CS = {
            shaderBlobMap_[shaderInfo.csKey]->GetBufferPointer(),
            shaderBlobMap_[shaderInfo.csKey]->GetBufferSize()};
        computeStateDesc.pRootSignature = pso->rootSignature.Get();
        result                          = device->CreateComputePipelineState(
            &computeStateDesc,
            IID_PPV_ARGS(&pso->pipelineState));

        if (FAILED(result)) {
            LOG_ERROR("CreateComputePipelineState failed: {}", HrToString(result));
            assert(false);
        }

        psoMap_[key] = std::move(pso);
        LOG_INFO("Compute Pipeline State Object created: {}", key);

        return psoMap_[key].get();
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc{};

    if (!shaderInfo.vsKey.empty()) {
        pipelineStateDesc.VS = {
            shaderBlobMap_[shaderInfo.vsKey]->GetBufferPointer(),
            shaderBlobMap_[shaderInfo.vsKey]->GetBufferSize()};
    }
    if (!shaderInfo.psKey.empty()) {
        pipelineStateDesc.PS = {
            shaderBlobMap_[shaderInfo.psKey]->GetBufferPointer(),
            shaderBlobMap_[shaderInfo.psKey]->GetBufferSize()};
    }
    if (!shaderInfo.dsKey.empty()) {
        pipelineStateDesc.DS = {
            shaderBlobMap_[shaderInfo.dsKey]->GetBufferPointer(),
            shaderBlobMap_[shaderInfo.dsKey]->GetBufferSize()};
    }
    if (!shaderInfo.hsKey.empty()) {
        pipelineStateDesc.HS = {
            shaderBlobMap_[shaderInfo.hsKey]->GetBufferPointer(),
            shaderBlobMap_[shaderInfo.hsKey]->GetBufferSize()};
    }
    if (!shaderInfo.dsKey.empty()) {
        pipelineStateDesc.DS = {
            shaderBlobMap_[shaderInfo.dsKey]->GetBufferPointer(),
            shaderBlobMap_[shaderInfo.dsKey]->GetBufferSize()};
    }

    pipelineStateDesc.pRootSignature = pso->rootSignature.Get();
    pipelineStateDesc.InputLayout    = inputLayoutDesc;

    pipelineStateDesc.BlendState      = blendDesc;
    pipelineStateDesc.RasterizerState = shaderInfo.rasterizerDesc;

    pipelineStateDesc.DepthStencilState = shaderInfo.depthStencilDesc_;
    pipelineStateDesc.DSVFormat         = DXGI_FORMAT_D24_UNORM_S8_UINT;

    pipelineStateDesc.NumRenderTargets = 1;
    pipelineStateDesc.RTVFormats[0]    = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    // 利用するトポロジ(形状)タイプ。
    pipelineStateDesc.PrimitiveTopologyType = shaderInfo.topologyType;
    // どのように画面に色を打ち込むかの設定
    pipelineStateDesc.SampleDesc.Count = 1;
    pipelineStateDesc.SampleMask       = D3D12_DEFAULT_SAMPLE_MASK;

    result = device->CreateGraphicsPipelineState(
        &pipelineStateDesc,
        IID_PPV_ARGS(&pso->pipelineState));

    if (FAILED(result)) {
        LOG_ERROR("CreateGraphicsPipelineState failed: {}", HrToString(result));
        assert(SUCCEEDED(result));
    };

    psoMap_[key] = std::move(pso);

    return psoMap_[key].get();
}

bool ShaderManager::LoadShader(const std::string& fileName, const std::string& directory, const wchar_t* profile) {
    return RegisterShaderBlob(fileName, shaderCompiler_->CompileShader(ConvertString(directory + '/' + fileName + ".hlsl"), profile));
}
