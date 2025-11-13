#include "directX12/ShaderManager.h"

/// api
#include <Windows.h>

/// assert (log)
#include "logger/Logger.h"
#include <cassert>

/// util
#include "util/StringUtil.h"

void ShaderManager::Initialize() {
    shaderCompiler_ = std::make_unique<ShaderCompiler>();
    shaderCompiler_->Initialize();
}

void ShaderManager::Finalize() {
    // PSO解放
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

    // すでに存在するならそれを返す
    auto it = psoMap_.find(key);
    if (it != psoMap_.end()) {
        return it->second.get();
    }

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

    // エラー処理
    if (FAILED(result)) {
        std::string errMsg(
            static_cast<const char*>(errorBlob->GetBufferPointer()),
            errorBlob->GetBufferSize());
        LOG_ERROR("D3D12SerializeRootSignature failed: {}", errMsg);

        OutputDebugStringA(("D3D12SerializeRootSignature failed: " + errMsg + "\n").c_str());

        assert(false);
    }

    // RootSignatureの生成
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

   blendDesc =  CreateBlendDescByBlendMode(shaderInfo.blendMode_);


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

        result = device->CreateComputePipelineState(
            &computeStateDesc,
            IID_PPV_ARGS(&pso->pipelineState));

        if (FAILED(result)) {
            std::string resultString = HrToString(result);
            LOG_ERROR("CreateComputePipelineState failed: {}", resultString);

            OutputDebugStringA(("CreateComputePipelineState failed:\n" + resultString + "\n").c_str());

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
        std::string resultString = HrToString(result);
        LOG_ERROR("CreateGraphicsPipelineState failed: {}", resultString);
        OutputDebugStringA(("CreateGraphicsPipelineState" + resultString + "\n").c_str());
        assert(SUCCEEDED(result));
    };

    psoMap_[key] = std::move(pso);

    return psoMap_[key].get();
}

bool ShaderManager::LoadShader(const std::string& fileName, const std::string& directory, const wchar_t* profile) {
    return RegisterShaderBlob(fileName, shaderCompiler_->CompileShader(ConvertString(directory + '/' + fileName + ".hlsl"), profile));
}

bool ShaderManager::RegisterShaderBlob(const std::string& fileName, Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob) {
    auto it = shaderBlobMap_.find(fileName);
    if (it != shaderBlobMap_.end()) {
        return false;
    }
    shaderBlobMap_.emplace(fileName, std::move(shaderBlob));
    return true;
}
bool ShaderManager::IsRegisteredShaderBlob(const std::string& fileName) const {
    auto it = shaderBlobMap_.find(fileName);
    if (it != shaderBlobMap_.end()) {
        return true;
    }
    return false;
}

PipelineStateObj* ShaderManager::getPipelineStateObj(const std::string& key) {
    auto it = psoMap_.find(key);
    if (it == psoMap_.end()) {
        return nullptr;
    }
    return it->second.get();
}

Microsoft::WRL::ComPtr<IDxcBlob>* ShaderManager::getShaderBlob(const std::string& key) {
    auto it = shaderBlobMap_.find(key);
    if (it == shaderBlobMap_.end()) {
        return nullptr;
    }
    return &it->second;
}
