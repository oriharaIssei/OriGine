#include "directX12/ShaderManager.h"

/// api
#include <Windows.h>

/// assert (log)
#include "logger/Logger.h"
#include <cassert>

/// util
#include "util/StringUtil.h"

using namespace OriGine;

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

PipelineStateObj* ShaderManager::CreatePso(const std::string& _key,
    const ShaderInformation& _shaderInfo,
    Microsoft::WRL::ComPtr<ID3D12Device> _device) {

    // すでに存在するならそれを返す
    auto it = psoMap_.find(_key);
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
        .NumParameters     = static_cast<uint32_t>(_shaderInfo.rootParameters_.size()),
        .pParameters       = _shaderInfo.rootParameters_.data(),
        .NumStaticSamplers = static_cast<uint32_t>(_shaderInfo.samplerDescs_.size()),
        .pStaticSamplers   = _shaderInfo.samplerDescs_.data(),
        .Flags             = _shaderInfo.rootSignatureFlag,
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
    _device->CreateRootSignature(
        0,
        signatureBlob->GetBufferPointer(),
        signatureBlob->GetBufferSize(),
        IID_PPV_ARGS(&pso->rootSignature));

    ///=================================================
    /// InputLayout
    ///=================================================
    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{
        .pInputElementDescs = _shaderInfo.elementDescs_.data(),
        .NumElements        = static_cast<uint32_t>(_shaderInfo.elementDescs_.size()),
    };

    ///=================================================
    /// BlendDesc 初期化
    ///=================================================
    D3D12_BLEND_DESC blendDesc{};
    blendDesc.RenderTarget[0] = {}; // RenderTarget[0] を初期化

    blendDesc = CreateBlendDescByBlendMode(_shaderInfo.blendMode_);

    ///=================================================
    /// GRAPHICS_PIPELINE_STATE_DESC 初期化
    ///=================================================
    // cs か graphics pipeline state かを判定
    if (!_shaderInfo.csKey.empty()) {
        D3D12_COMPUTE_PIPELINE_STATE_DESC computeStateDesc{};
        computeStateDesc.CS = {
            shaderBlobMap_[_shaderInfo.csKey]->GetBufferPointer(),
            shaderBlobMap_[_shaderInfo.csKey]->GetBufferSize()};
        computeStateDesc.pRootSignature = pso->rootSignature.Get();

        result = _device->CreateComputePipelineState(
            &computeStateDesc,
            IID_PPV_ARGS(&pso->pipelineState));

        if (FAILED(result)) {
            std::string resultString = HrToString(result);
            LOG_ERROR("CreateComputePipelineState failed: {}", resultString);

            OutputDebugStringA(("CreateComputePipelineState failed:\n" + resultString + "\n").c_str());

            assert(false);
        }

        psoMap_[_key] = std::move(pso);
        LOG_INFO("Compute Pipeline State Object created: {}", _key);

        return psoMap_[_key].get();
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc{};

    if (!_shaderInfo.vsKey.empty()) {
        pipelineStateDesc.VS = {
            shaderBlobMap_[_shaderInfo.vsKey]->GetBufferPointer(),
            shaderBlobMap_[_shaderInfo.vsKey]->GetBufferSize()};
    }
    if (!_shaderInfo.psKey.empty()) {
        pipelineStateDesc.PS = {
            shaderBlobMap_[_shaderInfo.psKey]->GetBufferPointer(),
            shaderBlobMap_[_shaderInfo.psKey]->GetBufferSize()};
    }
    if (!_shaderInfo.dsKey.empty()) {
        pipelineStateDesc.DS = {
            shaderBlobMap_[_shaderInfo.dsKey]->GetBufferPointer(),
            shaderBlobMap_[_shaderInfo.dsKey]->GetBufferSize()};
    }
    if (!_shaderInfo.hsKey.empty()) {
        pipelineStateDesc.HS = {
            shaderBlobMap_[_shaderInfo.hsKey]->GetBufferPointer(),
            shaderBlobMap_[_shaderInfo.hsKey]->GetBufferSize()};
    }
    if (!_shaderInfo.dsKey.empty()) {
        pipelineStateDesc.DS = {
            shaderBlobMap_[_shaderInfo.dsKey]->GetBufferPointer(),
            shaderBlobMap_[_shaderInfo.dsKey]->GetBufferSize()};
    }

    pipelineStateDesc.pRootSignature = pso->rootSignature.Get();
    pipelineStateDesc.InputLayout    = inputLayoutDesc;

    pipelineStateDesc.BlendState      = blendDesc;
    pipelineStateDesc.RasterizerState = _shaderInfo.rasterizerDesc;

    pipelineStateDesc.DepthStencilState = _shaderInfo.depthStencilDesc_;
    pipelineStateDesc.DSVFormat         = DXGI_FORMAT_D24_UNORM_S8_UINT;

    pipelineStateDesc.NumRenderTargets = 1;
    pipelineStateDesc.RTVFormats[0]    = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    // 利用するトポロジ(形状)タイプ。
    pipelineStateDesc.PrimitiveTopologyType = _shaderInfo.topologyType;
    // どのように画面に色を打ち込むかの設定
    pipelineStateDesc.SampleDesc.Count = 1;
    pipelineStateDesc.SampleMask       = D3D12_DEFAULT_SAMPLE_MASK;

    result = _device->CreateGraphicsPipelineState(
        &pipelineStateDesc,
        IID_PPV_ARGS(&pso->pipelineState));

    if (FAILED(result)) {
        std::string resultString = HrToString(result);
        LOG_ERROR("CreateGraphicsPipelineState failed: {}", resultString);
        OutputDebugStringA(("CreateGraphicsPipelineState" + resultString + "\n").c_str());
        assert(SUCCEEDED(result));
    };

    psoMap_[_key] = std::move(pso);

    return psoMap_[_key].get();
}

bool ShaderManager::LoadShader(const std::string& _fileName, const std::string& _directory, const wchar_t* _profile) {
    return RegisterShaderBlob(_fileName, shaderCompiler_->CompileShader(ConvertString(_directory + '/' + _fileName + ".hlsl"), _profile));
}

bool ShaderManager::RegisterShaderBlob(const std::string& _fileName, Microsoft::WRL::ComPtr<IDxcBlob> _shaderBlob) {
    auto it = shaderBlobMap_.find(_fileName);
    if (it != shaderBlobMap_.end()) {
        return false;
    }
    shaderBlobMap_.emplace(_fileName, std::move(_shaderBlob));
    return true;
}
bool ShaderManager::IsRegisteredShaderBlob(const std::string& _fileName) const {
    auto it = shaderBlobMap_.find(_fileName);
    if (it != shaderBlobMap_.end()) {
        return true;
    }
    return false;
}

PipelineStateObj* ShaderManager::GetPipelineStateObj(const std::string& _key) {
    auto it = psoMap_.find(_key);
    if (it == psoMap_.end()) {
        return nullptr;
    }
    return it->second.get();
}

Microsoft::WRL::ComPtr<IDxcBlob>* ShaderManager::GetShaderBlob(const std::string& _key) {
    auto it = shaderBlobMap_.find(_key);
    if (it == shaderBlobMap_.end()) {
        return nullptr;
    }
    return &it->second;
}
